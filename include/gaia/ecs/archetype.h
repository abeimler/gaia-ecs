#pragma once
#include "../containers/darray.h"
#include "../containers/sarray.h"
#include "../containers/sarray_ext.h"
#include "../utils/hashing_policy.h"
#include "../utils/mem.h"
#include "archetype_common.h"
#include "chunk.h"
#include "chunk_allocator.h"
#include "chunk_header.h"
#include "component.h"
#include "component_cache.h"
#include "component_utils.h"

namespace gaia {
	namespace ecs {
		namespace archetype {
			class Archetype final {
			public:
				using LookupHash = utils::direct_hash_key<uint64_t>;
				using GenericComponentHash = utils::direct_hash_key<uint64_t>;
				using ChunkComponentHash = utils::direct_hash_key<uint64_t>;

			private:
#if GAIA_ARCHETYPE_GRAPH
				struct ArchetypeGraphEdge {
					ArchetypeId archetypeId;
				};
#endif

				//! List of active chunks allocated by this archetype
				containers::darray<Chunk*> m_chunks;
				//! List of disabled chunks allocated by this archetype
				containers::darray<Chunk*> m_chunksDisabled;

#if GAIA_ARCHETYPE_GRAPH
				//! Map of edges in the archetype graph when adding components
				containers::map<component::ComponentId, ArchetypeGraphEdge> m_edgesAdd[component::ComponentType::CT_Count];
				//! Map of edges in the archetype graph when removing components
				containers::map<component::ComponentId, ArchetypeGraphEdge> m_edgesDel[component::ComponentType::CT_Count];
#endif

				//! Description of components within this archetype
				containers::sarray<ComponentIdArray, component::ComponentType::CT_Count> m_componentIds;
				//! Lookup hashes of components within this archetype
				containers::sarray<ComponentOffsetArray, component::ComponentType::CT_Count> m_componentOffsets;

				GenericComponentHash m_genericHash = {0};
				ChunkComponentHash m_chunkHash = {0};

				//! Hash of components within this archetype - used for lookups
				component::ComponentLookupHash m_lookupHash = {0};
				//! Hash of components within this archetype - used for matching
				component::ComponentMatcherHash m_matcherHash[component::ComponentType::CT_Count] = {0};
				//! Archetype ID - used to address the archetype directly in the world's list or archetypes
				ArchetypeId m_archetypeId = ArchetypeIdBad;
				//! Stable reference to parent world's world version
				uint32_t& m_worldVersion;
				struct {
					//! The number of entities this archetype can take (e.g 5 = 5 entities with all their components)
					uint32_t capacity : 16;
					//! True if there's a component that requires custom destruction
					uint32_t hasGenericComponentWithCustomDestruction : 1;
					//! True if there's a component that requires custom destruction
					uint32_t hasChunkComponentWithCustomDestruction : 1;
				} m_properties{};

				// Constructor is hidden. Create archetypes via Create
				Archetype(uint32_t& worldVersion): m_worldVersion(worldVersion){};

				/*!
				Releases all memory allocated by \param pChunk.
				\param pChunk Chunk which we want to destroy
				*/
				void ReleaseChunk(Chunk* pChunk) const {
					const auto& cc = ComponentCache::Get();

					auto callDestructors = [&](component::ComponentType componentType) {
						const auto& componentIds = m_componentIds[componentType];
						const auto& offsets = m_componentOffsets[componentType];
						const auto itemCount =
								componentType == component::ComponentType::CT_Generic ? pChunk->GetEntityCount() : 1U;
						for (size_t i = 0; i < componentIds.size(); ++i) {
							const auto componentId = componentIds[i];
							const auto& infoCreate = cc.GetComponentDesc(componentId);
							if (infoCreate.destructor == nullptr)
								continue;
							auto* pSrc = (void*)((uint8_t*)pChunk + offsets[i]);
							infoCreate.destructor(pSrc, itemCount);
						}
					};

					// Call destructors for components that need it
					if (m_properties.hasGenericComponentWithCustomDestruction == 1)
						callDestructors(component::ComponentType::CT_Generic);
					if (m_properties.hasChunkComponentWithCustomDestruction == 1)
						callDestructors(component::ComponentType::CT_Chunk);

					Chunk::Release(pChunk);
				}

				GAIA_NODISCARD Chunk* FindOrCreateFreeChunk_Internal(containers::darray<Chunk*>& chunkArray) const {
					const auto chunkCnt = chunkArray.size();

					if (chunkCnt > 0) {
						// Look for chunks with free space back-to-front.
						// We do it this way because we always try to keep fully utilized and
						// thus only the one in the back should be free.
						auto i = chunkCnt - 1;
						do {
							auto* pChunk = chunkArray[i];
							GAIA_ASSERT(pChunk != nullptr);
							if (!pChunk->IsFull())
								return pChunk;
						} while (i-- > 0);
					}

					GAIA_ASSERT(chunkCnt < UINT16_MAX);

					// No free space found anywhere. Let's create a new chunk.
					auto* pChunk = Chunk::Create(
							m_archetypeId, (uint16_t)chunkCnt, m_properties.capacity, m_worldVersion, m_componentIds,
							m_componentOffsets);

					chunkArray.push_back(pChunk);
					return pChunk;
				}

				/*!
				Checks if a component with \param componentId and type \param componentType is present in the archetype.
				\param componentId Component id
				\param componentType Component type
				\return True if found. False otherwise.
				*/
				GAIA_NODISCARD bool
				HasComponent_Internal(component::ComponentType componentType, component::ComponentId componentId) const {
					const auto& componentIds = GetComponentIdArray(componentType);
					return utils::has(componentIds, componentId);
				}

				/*!
				Checks if a component of type \tparam T is present in the archetype.
				\return True if found. False otherwise.
				*/
				template <typename T>
				GAIA_NODISCARD bool HasComponent_Internal() const {
					const auto componentId = component::GetComponentId<T>();

					if constexpr (component::IsGenericComponent<T>) {
						return HasComponent_Internal(component::ComponentType::CT_Generic, componentId);
					} else {
						return HasComponent_Internal(component::ComponentType::CT_Chunk, componentId);
					}
				}

			public:
				/*!
				Checks if the archetype id is valid.
				\return True if the id is valid, false otherwise.
				*/
				static bool IsIdValid(ArchetypeId id) {
					return id != ArchetypeIdBad;
				}

				Archetype(Archetype&& world) = delete;
				Archetype(const Archetype& world) = delete;
				Archetype& operator=(Archetype&&) = delete;
				Archetype& operator=(const Archetype&) = delete;

				~Archetype() {
					// Delete all archetype chunks
					for (auto* pChunk: m_chunks)
						ReleaseChunk(pChunk);
					for (auto* pChunk: m_chunksDisabled)
						ReleaseChunk(pChunk);
				}

				GAIA_NODISCARD static LookupHash
				CalculateLookupHash(GenericComponentHash genericHash, ChunkComponentHash chunkHash) noexcept {
					return {utils::hash_combine(genericHash.hash, chunkHash.hash)};
				}

				GAIA_NODISCARD static Archetype* Create(
						ArchetypeId archetypeId, uint32_t& worldVersion, component::ComponentIdSpan componentIdsGeneric,
						component::ComponentIdSpan componentIdsChunk) {
					auto* newArch = new Archetype(worldVersion);
					newArch->m_archetypeId = archetypeId;

					const auto& cc = ComponentCache::Get();

					// Size of the entity + all of its generic components
					size_t genericComponentListSize = sizeof(Entity);
					for (const auto componentId: componentIdsGeneric) {
						const auto& desc = cc.GetComponentDesc(componentId);
						genericComponentListSize += desc.properties.size;
						newArch->m_properties.hasGenericComponentWithCustomDestruction |= (desc.properties.destructible != 0);
					}

					// Size of chunk components
					size_t chunkComponentListSize = 0;
					for (const auto componentId: componentIdsChunk) {
						const auto& desc = cc.GetComponentDesc(componentId);
						chunkComponentListSize += desc.properties.size;
						newArch->m_properties.hasChunkComponentWithCustomDestruction |= (desc.properties.destructible != 0);
					}

					// TODO: Calculate the number of entities per chunks precisely so we can
					// fit more of them into chunk on average. Currently, DATA_SIZE_RESERVED
					// is substracted but that's not optimal...

					// Number of components we can fit into one chunk
					auto maxGenericItemsInArchetype = (Chunk::DATA_SIZE - chunkComponentListSize) / genericComponentListSize;

					// Calculate component offsets now. Skip the header and entity IDs
					auto componentOffsets = sizeof(Entity) * maxGenericItemsInArchetype;
					auto alignedOffset = sizeof(ChunkHeader) + componentOffsets;

					// Add generic infos
					for (const auto componentId: componentIdsGeneric) {
						const auto& desc = cc.GetComponentDesc(componentId);
						const auto alignment = desc.properties.alig;
						if (alignment != 0) {
							const size_t padding = utils::align(alignedOffset, alignment) - alignedOffset;
							componentOffsets += padding;
							alignedOffset += padding;

							// Make sure we didn't exceed the chunk size
							GAIA_ASSERT(componentOffsets <= Chunk::DATA_SIZE_NORESERVE);

							// Register the component info
							newArch->m_componentIds[component::ComponentType::CT_Generic].push_back(componentId);
							newArch->m_componentOffsets[component::ComponentType::CT_Generic].push_back((uint32_t)componentOffsets);

							// Make sure the following component list is properly aligned
							componentOffsets += desc.properties.size * maxGenericItemsInArchetype;
							alignedOffset += desc.properties.size * maxGenericItemsInArchetype;

							// Make sure we didn't exceed the chunk size
							GAIA_ASSERT(componentOffsets <= Chunk::DATA_SIZE_NORESERVE);
						} else {
							// Register the component info
							newArch->m_componentIds[component::ComponentType::CT_Generic].push_back(componentId);
							newArch->m_componentOffsets[component::ComponentType::CT_Generic].push_back((uint32_t)componentOffsets);
						}
					}

					// Add chunk infos
					for (const auto componentId: componentIdsChunk) {
						const auto& desc = cc.GetComponentDesc(componentId);
						const auto alignment = desc.properties.alig;
						if (alignment != 0) {
							const size_t padding = utils::align(alignedOffset, alignment) - alignedOffset;
							componentOffsets += padding;
							alignedOffset += padding;

							// Make sure we didn't exceed the chunk size
							GAIA_ASSERT(componentOffsets <= Chunk::DATA_SIZE_NORESERVE);

							// Register the component info
							newArch->m_componentIds[component::ComponentType::CT_Chunk].push_back(componentId);
							newArch->m_componentOffsets[component::ComponentType::CT_Chunk].push_back((uint32_t)componentOffsets);

							// Make sure the following component list is properly aligned
							componentOffsets += desc.properties.size;
							alignedOffset += desc.properties.size;

							// Make sure we didn't exceed the chunk size
							GAIA_ASSERT(componentOffsets <= Chunk::DATA_SIZE_NORESERVE);
						} else {
							// Register the component info
							newArch->m_componentIds[component::ComponentType::CT_Chunk].push_back(componentId);
							newArch->m_componentOffsets[component::ComponentType::CT_Chunk].push_back((uint32_t)componentOffsets);
						}
					}

					newArch->m_properties.capacity = (uint32_t)maxGenericItemsInArchetype;
					newArch->m_matcherHash[component::ComponentType::CT_Generic] =
							component::CalculateMatcherHash(componentIdsGeneric);
					newArch->m_matcherHash[component::ComponentType::CT_Chunk] =
							component::CalculateMatcherHash(componentIdsChunk);

					return newArch;
				}

				/*!
				Initializes the archetype with hash values for each kind of component types.
				\param hashGeneric Generic components hash
				\param hashChunk Chunk components hash
				\param hashLookup Hash used for archetype lookup purposes
				*/
				void Init(
						GenericComponentHash hashGeneric, ChunkComponentHash hashChunk, component::ComponentLookupHash hashLookup) {
					m_genericHash = hashGeneric;
					m_chunkHash = hashChunk;
					m_lookupHash = hashLookup;
				}

				/*!
				Removes a chunk from the list of chunks managed by their achetype.
				\param pChunk Chunk to remove from the list of managed archetypes
				*/
				void RemoveChunk(Chunk* pChunk) {
					const bool isDisabled = pChunk->IsDisabled();
					const auto chunkIndex = pChunk->GetChunkIndex();

					ReleaseChunk(pChunk);

					auto remove = [&](auto& chunkArray) {
						if (chunkArray.size() > 1)
							chunkArray.back()->SetChunkIndex(chunkIndex);
						GAIA_ASSERT(chunkIndex == utils::get_index(chunkArray, pChunk));
						utils::erase_fast(chunkArray, chunkIndex);
					};

					if (isDisabled)
						remove(m_chunksDisabled);
					else
						remove(m_chunks);
				}

				//! Tries to locate a chunk that has some space left for a new entity.
				//! If not found a new chunk is created
				GAIA_NODISCARD Chunk* FindOrCreateFreeChunk() {
					return FindOrCreateFreeChunk_Internal(m_chunks);
				}

				//! Tries to locate a chunk for disabled entities that has some space left for a new one.
				//! If not found a new chunk is created
				GAIA_NODISCARD Chunk* FindOrCreateFreeChunkDisabled() {
					auto* pChunk = FindOrCreateFreeChunk_Internal(m_chunksDisabled);
					pChunk->SetDisabled(true);
					return pChunk;
				}

				GAIA_NODISCARD ArchetypeId GetArchetypeId() const {
					return m_archetypeId;
				}

				GAIA_NODISCARD uint32_t GetCapacity() const {
					return m_properties.capacity;
				}

				GAIA_NODISCARD const containers::darray<Chunk*>& GetChunks() const {
					return m_chunks;
				}

				GAIA_NODISCARD const containers::darray<Chunk*>& GetChunksDisabled() const {
					return m_chunksDisabled;
				}

				GAIA_NODISCARD GenericComponentHash GetGenericHash() const {
					return m_genericHash;
				}

				GAIA_NODISCARD ChunkComponentHash GetChunkHash() const {
					return m_chunkHash;
				}

				GAIA_NODISCARD component::ComponentLookupHash GetLookupHash() const {
					return m_lookupHash;
				}

				GAIA_NODISCARD component::ComponentMatcherHash GetMatcherHash(component::ComponentType componentType) const {
					return m_matcherHash[componentType];
				}

				GAIA_NODISCARD const ComponentIdArray& GetComponentIdArray(component::ComponentType componentType) const {
					return m_componentIds[componentType];
				}

				GAIA_NODISCARD const ComponentOffsetArray&
				GetComponentOffsetArray(component::ComponentType componentType) const {
					return m_componentOffsets[componentType];
				}

				/*!
				Checks if a given component is present on the archetype.
				\return True if the component is present. False otherwise.
				*/
				template <typename T>
				GAIA_NODISCARD bool HasComponent() const {
					return HasComponent_Internal<T>();
				}

				/*!
				Returns the internal index of a component based on the provided \param componentId.
				\param componentType Component type
				\return Component index if the component was found. -1 otherwise.
				*/
				GAIA_NODISCARD uint32_t
				GetComponentIdx(component::ComponentType componentType, component::ComponentId componentId) const {
					const auto idx = utils::get_index_unsafe(m_componentIds[componentType], componentId);
					GAIA_ASSERT(idx != BadIndex);
					return (uint32_t)idx;
				}

#if GAIA_ARCHETYPE_GRAPH
				//! Creates an edge in the graph leading to the target archetype \param archetypeId via component \param
				//! componentId of type \param componentType.
				void AddGraphEdgeRight(
						component::ComponentType componentType, component::ComponentId componentId, ArchetypeId archetypeId) {
					[[maybe_unused]] const auto ret =
							m_edgesAdd[componentType].try_emplace({componentId}, ArchetypeGraphEdge{archetypeId});
					GAIA_ASSERT(ret.second);
				}

				//! Creates an edge in the graph leading to the target archetype \param archetypeId via component \param
				//! componentId of type \param componentType.
				void AddGraphEdgeLeft(
						component::ComponentType componentType, component::ComponentId componentId, ArchetypeId archetypeId) {
					[[maybe_unused]] const auto ret =
							m_edgesDel[componentType].try_emplace({componentId}, ArchetypeGraphEdge{archetypeId});
					GAIA_ASSERT(ret.second);
				}

				//! Checks if the graph edge for component type \param componentType contains the component \param componentId.
				//! \return Archetype id of the target archetype if the edge is found. ArchetypeIdBad otherwise.
				GAIA_NODISCARD ArchetypeId
				FindGraphEdgeRight(component::ComponentType componentType, const component::ComponentId componentId) const {
					const auto& edges = m_edgesAdd[componentType];
					const auto it = edges.find({componentId});
					return it != edges.end() ? it->second.archetypeId : ArchetypeIdBad;
				}

				//! Checks if the graph edge for component type \param componentType contains the component \param componentId.
				//! \return Archetype id of the target archetype if the edge is found. ArchetypeIdBad otherwise.
				GAIA_NODISCARD ArchetypeId
				FindGraphEdgeLeft(component::ComponentType componentType, const component::ComponentId componentId) const {
					const auto& edges = m_edgesDel[componentType];
					const auto it = edges.find({componentId});
					return it != edges.end() ? it->second.archetypeId : ArchetypeIdBad;
				}
#endif

				static void DiagArchetype_PrintBasicInfo(const archetype::Archetype& archetype) {
					const auto& cc = ComponentCache::Get();
					const auto& genericComponents = archetype.m_componentIds[component::ComponentType::CT_Generic];
					const auto& chunkComponents = archetype.m_componentIds[component::ComponentType::CT_Chunk];

					// Caclulate the number of entites in archetype
					uint32_t entityCount = 0;
					uint32_t entityCountDisabled = 0;
					for (const auto* chunk: archetype.m_chunks)
						entityCount += chunk->GetEntityCount();
					for (const auto* chunk: archetype.m_chunksDisabled) {
						entityCountDisabled += chunk->GetEntityCount();
						entityCount += chunk->GetEntityCount();
					}

					// Calculate the number of components
					uint32_t genericComponentsSize = 0;
					uint32_t chunkComponentsSize = 0;
					for (const auto componentId: genericComponents) {
						const auto& desc = cc.GetComponentDesc(componentId);
						genericComponentsSize += desc.properties.size;
					}
					for (const auto componentId: chunkComponents) {
						const auto& desc = cc.GetComponentDesc(componentId);
						chunkComponentsSize += desc.properties.size;
					}

					GAIA_LOG_N(
							"Archetype ID:%u, "
							"lookupHash:%016" PRIx64 ", "
							"mask:%016" PRIx64 "/%016" PRIx64 ", "
							"chunks:%u, data size:%3u B (%u/%u), "
							"entities:%u/%u (disabled:%u)",
							archetype.m_archetypeId, archetype.m_lookupHash.hash,
							archetype.m_matcherHash[component::ComponentType::CT_Generic].hash,
							archetype.m_matcherHash[component::ComponentType::CT_Chunk].hash, (uint32_t)archetype.m_chunks.size(),
							genericComponentsSize + chunkComponentsSize, genericComponentsSize, chunkComponentsSize, entityCount,
							archetype.m_properties.capacity, entityCountDisabled);

					auto logComponentInfo = [](const component::ComponentInfo& info, const component::ComponentDesc& desc) {
						GAIA_LOG_N(
								"    lookupHash:%016" PRIx64 ", mask:%016" PRIx64 ", size:%3u B, align:%3u B, %.*s",
								info.lookupHash.hash, info.matcherHash.hash, desc.properties.size, desc.properties.alig,
								(uint32_t)desc.name.size(), desc.name.data());
					};

					if (!genericComponents.empty()) {
						GAIA_LOG_N("  Generic components - count:%u", (uint32_t)genericComponents.size());
						for (const auto componentId: genericComponents) {
							const auto& info = cc.GetComponentInfo(componentId);
							logComponentInfo(info, cc.GetComponentDesc(componentId));
						}
						if (!chunkComponents.empty()) {
							GAIA_LOG_N("  Chunk components - count:%u", (uint32_t)chunkComponents.size());
							for (const auto componentId: chunkComponents) {
								const auto& info = cc.GetComponentInfo(componentId);
								logComponentInfo(info, cc.GetComponentDesc(componentId));
							}
						}
					}
				}

#if GAIA_ARCHETYPE_GRAPH
				static void DiagArchetype_PrintGraphInfo(const archetype::Archetype& archetype) {
					const auto& cc = ComponentCache::Get();

					// Add edges (movement towards the leafs)
					{
						const auto& edgesG = archetype.m_edgesAdd[component::ComponentType::CT_Generic];
						const auto& edgesC = archetype.m_edgesAdd[component::ComponentType::CT_Chunk];
						const auto edgeCount = (uint32_t)(edgesG.size() + edgesC.size());
						if (edgeCount > 0) {
							GAIA_LOG_N("  Add edges - count:%u", edgeCount);

							if (!edgesG.empty()) {
								GAIA_LOG_N("    Generic - count:%u", (uint32_t)edgesG.size());
								for (const auto& edge: edgesG) {
									const auto& info = cc.GetComponentInfo(edge.first);
									const auto& infoCreate = cc.GetComponentDesc(info.componentId);
									GAIA_LOG_N(
											"      %.*s (--> Archetype ID:%u)", (uint32_t)infoCreate.name.size(), infoCreate.name.data(),
											edge.second.archetypeId);
								}
							}

							if (!edgesC.empty()) {
								GAIA_LOG_N("    Chunk - count:%u", (uint32_t)edgesC.size());
								for (const auto& edge: edgesC) {
									const auto& info = cc.GetComponentInfo(edge.first);
									const auto& infoCreate = cc.GetComponentDesc(info.componentId);
									GAIA_LOG_N(
											"      %.*s (--> Archetype ID:%u)", (uint32_t)infoCreate.name.size(), infoCreate.name.data(),
											edge.second.archetypeId);
								}
							}
						}
					}

					// Delete edges (movement towards the root)
					{
						const auto& edgesG = archetype.m_edgesDel[component::ComponentType::CT_Generic];
						const auto& edgesC = archetype.m_edgesDel[component::ComponentType::CT_Chunk];
						const auto edgeCount = (uint32_t)(edgesG.size() + edgesC.size());
						if (edgeCount > 0) {
							GAIA_LOG_N("  Del edges - count:%u", edgeCount);

							if (!edgesG.empty()) {
								GAIA_LOG_N("    Generic - count:%u", (uint32_t)edgesG.size());
								for (const auto& edge: edgesG) {
									const auto& info = cc.GetComponentInfo(edge.first);
									const auto& infoCreate = cc.GetComponentDesc(info.componentId);
									GAIA_LOG_N(
											"      %.*s (--> Archetype ID:%u)", (uint32_t)infoCreate.name.size(), infoCreate.name.data(),
											edge.second.archetypeId);
								}
							}

							if (!edgesC.empty()) {
								GAIA_LOG_N("    Chunk - count:%u", (uint32_t)edgesC.size());
								for (const auto& edge: edgesC) {
									const auto& info = cc.GetComponentInfo(edge.first);
									const auto& infoCreate = cc.GetComponentDesc(info.componentId);
									GAIA_LOG_N(
											"      %.*s (--> Archetype ID:%u)", (uint32_t)infoCreate.name.size(), infoCreate.name.data(),
											edge.second.archetypeId);
								}
							}
						}
					}
				}
#endif

				static void DiagArchetype_PrintChunkInfo(const archetype::Archetype& archetype) {
					auto logChunks = [](const auto& chunks) {
						for (size_t i = 0; i < chunks.size(); ++i) {
							const auto* pChunk = chunks[i];
							pChunk->Diag((uint32_t)i);
						}
					};

					// Enabled chunks
					{
						const auto& chunks = archetype.m_chunks;
						if (!chunks.empty())
							GAIA_LOG_N("  Enabled chunks");

						logChunks(chunks);
					}

					// Disabled chunks
					{
						const auto& chunks = archetype.m_chunksDisabled;
						if (!chunks.empty())
							GAIA_LOG_N("  Disabled chunks");

						logChunks(chunks);
					}
				}

				/*!
				Performs diagnostics on a specific archetype. Prints basic info about it and the chunks it contains.
				\param archetype Archetype to run diagnostics on
				*/
				static void DiagArchetype(const archetype::Archetype& archetype) {
					DiagArchetype_PrintBasicInfo(archetype);
#if GAIA_ARCHETYPE_GRAPH
					DiagArchetype_PrintGraphInfo(archetype);
#endif
					DiagArchetype_PrintChunkInfo(archetype);
				}
			};
		} // namespace archetype
	} // namespace ecs
} // namespace gaia

REGISTER_HASH_TYPE(gaia::ecs::Archetype::LookupHash)
REGISTER_HASH_TYPE(gaia::ecs::Archetype::GenericComponentHash)
REGISTER_HASH_TYPE(gaia::ecs::Archetype::ChunkComponentHash)