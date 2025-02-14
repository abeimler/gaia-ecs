#pragma once

#include <type_traits>

#include "../cnt/darray.h"
#include "../cnt/sarray.h"
#include "../cnt/sarray_ext.h"
#include "../core/bit_utils.h"
#include "../core/hashing_policy.h"
#include "component.h"
#include "component_utils.h"
#include "id.h"

namespace gaia {
	namespace ecs {
		class World;
		class Archetype;

		ComponentCache& comp_cache_mut(World& world);

		//! Number of items that can be a part of Query
		static constexpr uint32_t MAX_ITEMS_IN_QUERY = 8U;

		GAIA_GCC_WARNING_PUSH()
		// GCC is unnecessarily too strict about shadowing.
		// We have a globally defined entity All and thinks QueryOp::All shadows it.
		GAIA_GCC_WARNING_DISABLE("-Wshadow")

		//! Operation type
		enum class QueryOp : uint8_t { All, Any, Not, Count };
		//! Access type
		enum class QueryAccess : uint8_t { None, Read, Write };

		GAIA_GCC_WARNING_POP()

		using QueryId = uint32_t;
		using QueryLookupHash = core::direct_hash_key<uint64_t>;
		using QueryEntityArray = cnt::sarray_ext<Entity, MAX_ITEMS_IN_QUERY>;
		using QueryArchetypeCacheIndexMap = cnt::map<EntityLookupKey, uint32_t>;
		using QueryOpArray = cnt::sarray_ext<QueryOp, MAX_ITEMS_IN_QUERY>;

		static constexpr QueryId QueryIdBad = (QueryId)-1;

		struct QueryItem {
			//! Entity/Component/Pair to query
			Entity id;
			//! Operation to perform with the query item
			QueryOp op = QueryOp::All;
			//! Access type
			QueryAccess access = QueryAccess::Read;
			//! Source entity to query the id on.
			//! If id==EntityBad the source is fixed.
			//! If id!=src the source is variable.
			Entity src = EntityBad;
		};

		struct QueryEntityOpPair {
			//! Queried id
			Entity id;
			//! Source of where the queried id if looked up at
			Entity src;
			//! Archetype of the src entity
			Archetype* srcArchetype;
			//! Operation to perform with the query item
			QueryOp op;

			bool operator==(const QueryEntityOpPair& other) const {
				return id == other.id && src == other.src && op == other.op;
			}
			bool operator!=(const QueryEntityOpPair& other) const {
				return !operator==(other);
			}
		};

		using QueryEntityOpPairArray = cnt::sarray_ext<QueryEntityOpPair, MAX_ITEMS_IN_QUERY>;
		using QueryEntityOpPairSpan = std::span<QueryEntityOpPair>;
		using QueryRemappingArray = cnt::sarray_ext<uint8_t, MAX_ITEMS_IN_QUERY>;

		struct QueryCtx {
			// World
			const World* w{};
			//! Component cache
			ComponentCache* cc{};
			//! Lookup hash for this query
			QueryLookupHash hashLookup{};
			//! Query id
			QueryId queryId = QueryIdBad;

			struct Data {
				//! List of querried ids
				QueryEntityArray ids;
				//! List of [op,id] pairs
				QueryEntityOpPairArray pairs;
				//! Index of the last checked archetype in the component-to-archetype map
				QueryArchetypeCacheIndexMap lastMatchedArchetypeIdx_All;
				QueryArchetypeCacheIndexMap lastMatchedArchetypeIdx_Any;
				//! Mapping of the original indices to the new ones after sorting
				QueryRemappingArray remapping;
				//! List of filtered components
				QueryEntityArray withChanged;
				//! Mask for items with Is relationship pair.
				//! If the id is a pair, the first part (id) is written here.
				uint32_t as_mask;
				//! Mask for items with Is relationship pair.
				//! If the id is a pair, the second part (gen) is written here.
				uint32_t as_mask_2;
				//! First NOT record in pairs/ids/ops
				uint8_t firstNot;
				//! First ANY record in pairs/ids/ops
				uint8_t firstAny;
				//! Read-write mask. Bit 0 stands for component 0 in component arrays.
				//! A set bit means write access is requested.
				uint8_t readWriteMask;
			} data{};
			// Make sure that MAX_ITEMS_IN_QUERY can fit into data.readWriteMask
			static_assert(MAX_ITEMS_IN_QUERY == 8);

			void init(World* pWorld) {
				w = pWorld;
				cc = &comp_cache_mut(*pWorld);
			}

			GAIA_NODISCARD bool operator==(const QueryCtx& other) const {
				// Comparison expected to be done only the first time the query is set up
				GAIA_ASSERT(queryId == QueryIdBad);
				// Fast path when cache ids are set
				// if (queryId != QueryIdBad && queryId == other.queryId)
				// 	return true;

				// Lookup hash must match
				if (hashLookup != other.hashLookup)
					return false;

				const auto& left = data;
				const auto& right = other.data;

				// Check array sizes first
				if (left.pairs.size() != right.pairs.size())
					return false;
				if (left.withChanged.size() != right.withChanged.size())
					return false;
				if (left.readWriteMask != right.readWriteMask)
					return false;

				// Components need to be the same
				if (left.pairs != right.pairs)
					return false;

				// Filters need to be the same
				if (left.withChanged != right.withChanged)
					return false;

				return true;
			}

			GAIA_NODISCARD bool operator!=(const QueryCtx& other) const {
				return !operator==(other);
			};
		};

		//! Smaller ops first. Smaller ids second.
		struct query_sort_cond {
			constexpr bool operator()(const QueryEntityOpPair& lhs, const QueryEntityOpPair& rhs) const {
				if (lhs.op != rhs.op)
					return lhs.op < rhs.op;

				return lhs.id < rhs.id;
			}
		};

		//! Sorts internal component arrays
		inline void sort(QueryCtx& ctx) {
			auto& data = ctx.data;

			// Sort data. Necessary for correct hash calculation.
			// Without sorting query.all<XXX, YYY> would be different than query.all<YYY, XXX>.
			// Also makes sure data is in optimal order for query processing.
			core::sort(data.pairs, query_sort_cond{}, [&](uint32_t left, uint32_t right) {
				core::swap(data.ids[left], data.ids[right]);
				core::swap(data.pairs[left], data.pairs[right]);
				core::swap(data.remapping[left], data.remapping[right]);

				// Make sure masks remains correct after sorting
				core::swap_bits(data.readWriteMask, left, right);
				core::swap_bits(data.as_mask, left, right);
				core::swap_bits(data.as_mask_2, left, right);
			});

			auto& pairs = data.pairs;
			if (!pairs.empty()) {
				uint32_t i = 0;
				while (i < pairs.size() && pairs[i].op == QueryOp::All)
					++i;
				data.firstAny = (uint8_t)i;
				while (i < pairs.size() && pairs[i].op == QueryOp::Any)
					++i;
				data.firstNot = (uint8_t)i;
			}
		}

		inline void calc_lookup_hash(QueryCtx& ctx) {
			GAIA_ASSERT(ctx.cc != nullptr);
			// Make sure we don't calculate the hash twice
			GAIA_ASSERT(ctx.hashLookup.hash == 0);

			QueryLookupHash::Type hashLookup = 0;

			auto& data = ctx.data;

			// Ids & ops
			{
				QueryLookupHash::Type hash = 0;

				const auto& pairs = data.pairs;
				for (auto pair: pairs) {
					hash = core::hash_combine(hash, (QueryLookupHash::Type)pair.op);
					hash = core::hash_combine(hash, (QueryLookupHash::Type)pair.id.value());
				}
				hash = core::hash_combine(hash, (QueryLookupHash::Type)pairs.size());
				hash = core::hash_combine(hash, (QueryLookupHash::Type)data.readWriteMask);

				hashLookup = hash;
			}

			// Filters
			{
				QueryLookupHash::Type hash = 0;

				const auto& withChanged = data.withChanged;
				for (auto id: withChanged)
					hash = core::hash_combine(hash, (QueryLookupHash::Type)id.value());
				hash = core::hash_combine(hash, (QueryLookupHash::Type)withChanged.size());

				hashLookup = core::hash_combine(hashLookup, hash);
			}

			ctx.hashLookup = {core::calculate_hash64(hashLookup)};
		}

		//! Located the index at which the provided component id is located in the component array
		//! \param pComps Pointer to the start of the component array
		//! \param entity Entity we search for
		//! \return Index of the component id in the array
		//! \warning The component id must be present in the array
		template <uint32_t MAX_COMPONENTS>
		GAIA_NODISCARD inline uint32_t comp_idx(const QueryEntityOpPair* pComps, Entity entity, Entity src) {
			// We let the compiler know the upper iteration bound at compile-time.
			// This way it can optimize better (e.g. loop unrolling, vectorization).
			GAIA_FOR(MAX_COMPONENTS) {
				if (pComps[i].id == entity && pComps[i].src == src)
					return i;
			}

			GAIA_ASSERT(false);
			return BadIndex;
		}
	} // namespace ecs
} // namespace gaia