#pragma once
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../../utils/data_layout_policy.h"
#include "../../utils/iterator.h"
#include "../../utils/mem_utils.h"
#include "gaia/utils/utility.h"

namespace gaia {
	namespace containers {
		//! Array of elements of type \tparam T with fixed size and capacity \tparam N allocated on stack.
		//! Interface compatiblity with std::array where it matters.
		template <typename T, uint32_t N>
		class sarr {
		public:
			static_assert(N > 0);

			using value_type = T;
			using reference = T&;
			using const_reference = const T&;
			using pointer = T*;
			using const_pointer = T*;
			using difference_type = decltype(N);
			using size_type = decltype(N);
			using view_policy = utils::auto_view_policy<T>;

			static constexpr size_type extent = N;
			static constexpr uint32_t allocated_bytes = view_policy::get_min_byte_size(0, N);

			utils::raw_data_holder<T, allocated_bytes> m_data;

			class iterator {
			public:
				using iterator_category = GAIA_UTIL::random_access_iterator_tag;
				using value_type = T;
				using difference_type = sarr::size_type;
				using pointer = T*;
				using reference = T&;
				using size_type = sarr::size_type;

			private:
				pointer m_ptr;

			public:
				constexpr iterator(pointer ptr): m_ptr(ptr) {}

				constexpr reference operator*() const {
					return *m_ptr;
				}
				constexpr pointer operator->() const {
					return m_ptr;
				}
				constexpr iterator operator[](size_type offset) const {
					return {m_ptr + offset};
				}

				constexpr iterator& operator+=(size_type diff) {
					m_ptr += diff;
					return *this;
				}
				constexpr iterator& operator-=(size_type diff) {
					m_ptr -= diff;
					return *this;
				}
				constexpr iterator& operator++() {
					++m_ptr;
					return *this;
				}
				constexpr iterator operator++(int) {
					iterator temp(*this);
					++*this;
					return temp;
				}
				constexpr iterator& operator--() {
					--m_ptr;
					return *this;
				}
				constexpr iterator operator--(int) {
					iterator temp(*this);
					--*this;
					return temp;
				}

				constexpr iterator operator+(size_type offset) const {
					return {m_ptr + offset};
				}
				constexpr iterator operator-(size_type offset) const {
					return {m_ptr - offset};
				}
				constexpr difference_type operator-(const iterator& other) const {
					return (difference_type)(m_ptr - other.m_ptr);
				}

				GAIA_NODISCARD constexpr bool operator==(const iterator& other) const {
					return m_ptr == other.m_ptr;
				}
				GAIA_NODISCARD constexpr bool operator!=(const iterator& other) const {
					return m_ptr != other.m_ptr;
				}
				GAIA_NODISCARD constexpr bool operator>(const iterator& other) const {
					return m_ptr > other.m_ptr;
				}
				GAIA_NODISCARD constexpr bool operator>=(const iterator& other) const {
					return m_ptr >= other.m_ptr;
				}
				GAIA_NODISCARD constexpr bool operator<(const iterator& other) const {
					return m_ptr < other.m_ptr;
				}
				GAIA_NODISCARD constexpr bool operator<=(const iterator& other) const {
					return m_ptr <= other.m_ptr;
				}
			};

			class iterator_soa {
				friend class sarr;

			public:
				using iterator_category = GAIA_UTIL::random_access_iterator_tag;
				using value_type = T;
				using difference_type = sarr::size_type;
				// using pointer = T*; not supported
				// using reference = T&; not supported
				using size_type = sarr::size_type;

			private:
				uint8_t* m_ptr;
				uint32_t m_cnt;
				uint32_t m_idx;

			public:
				iterator_soa(uint8_t* ptr, uint32_t cnt, uint32_t idx): m_ptr(ptr), m_cnt(cnt), m_idx(idx) {}

				T operator*() const {
					return utils::data_view_policy<T::Layout, T>::get({m_ptr, m_cnt}, m_idx);
				}

				iterator_soa operator[](size_type offset) const {
					return iterator_soa(m_ptr, m_cnt, m_idx + offset);
				}

				iterator_soa& operator+=(size_type diff) {
					m_idx += diff;
					return *this;
				}
				iterator& operator-=(size_type diff) {
					m_idx -= diff;
					return *this;
				}
				iterator_soa& operator++() {
					++m_idx;
					return *this;
				}
				iterator_soa operator++(int) {
					iterator_soa temp(*this);
					++*this;
					return temp;
				}
				iterator_soa& operator--() {
					--m_idx;
					return *this;
				}
				iterator_soa operator--(int) {
					iterator_soa temp(*this);
					--*this;
					return temp;
				}

				iterator_soa operator+(size_type offset) const {
					return iterator_soa(m_ptr, m_cnt, m_idx + offset);
				}
				iterator_soa operator-(size_type offset) const {
					return iterator_soa(m_ptr, m_cnt, m_idx + offset);
				}
				difference_type operator-(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return (difference_type)(m_idx - other.m_idx);
				}

				GAIA_NODISCARD bool operator==(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx == other.m_idx;
				}
				GAIA_NODISCARD bool operator!=(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx != other.m_idx;
				}
				GAIA_NODISCARD bool operator>(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx > other.m_idx;
				}
				GAIA_NODISCARD bool operator>=(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx >= other.m_idx;
				}
				GAIA_NODISCARD bool operator<(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx < other.m_idx;
				}
				GAIA_NODISCARD bool operator<=(const iterator_soa& other) const {
					GAIA_ASSERT(m_ptr == other.m_ptr);
					return m_idx <= other.m_idx;
				}
			};

			constexpr sarr() noexcept {
				utils::construct_elements((pointer)&m_data[0], extent);
			}

			~sarr() {
				utils::destruct_elements((pointer)&m_data[0], extent);
			}

			template <typename InputIt>
			constexpr sarr(InputIt first, InputIt last) noexcept {
				utils::construct_elements((pointer)&m_data[0], extent);

				const auto count = (size_type)GAIA_UTIL::distance(first, last);

				if constexpr (std::is_pointer_v<InputIt>) {
					for (size_type i = 0; i < count; ++i)
						operator[](i) = first[i];
				} else if constexpr (std::is_same_v<
																 typename InputIt::iterator_category, GAIA_UTIL::random_access_iterator_tag>) {
					for (size_type i = 0; i < count; ++i)
						operator[](i) = *(first[i]);
				} else {
					size_type i = 0;
					for (auto it = first; it != last; ++it)
						operator[](i++) = *it;
				}
			}

			constexpr sarr(std::initializer_list<T> il): sarr(il.begin(), il.end()) {}

			constexpr sarr(const sarr& other): sarr(other.begin(), other.end()) {}

			constexpr sarr(sarr&& other) noexcept {
				GAIA_ASSERT(GAIA_UTIL::addressof(other) != this);

				utils::construct_elements((pointer)&m_data[0], extent);
				utils::move_elements<T>((uint8_t*)m_data, (const uint8_t*)other.m_data, 0, other.size(), extent, other.extent);

				other.m_cnt = size_type(0);
			}

			sarr& operator=(std::initializer_list<T> il) {
				*this = sarr(il.begin(), il.end());
				return *this;
			}

			constexpr sarr& operator=(const sarr& other) {
				GAIA_ASSERT(GAIA_UTIL::addressof(other) != this);

				utils::construct_elements((pointer)&m_data[0], extent);
				utils::copy_elements<T>(
						(uint8_t*)&m_data[0], (const uint8_t*)&other.m_data[0], 0, other.size(), extent, other.extent);

				return *this;
			}

			constexpr sarr& operator=(sarr&& other) noexcept {
				GAIA_ASSERT(GAIA_UTIL::addressof(other) != this);

				utils::construct_elements((pointer)&m_data[0], extent);
				utils::move_elements<T>(
						(uint8_t*)&m_data[0], (const uint8_t*)&other.m_data[0], 0, other.size(), extent, other.extent);

				return *this;
			}

			GAIA_NODISCARD constexpr pointer data() noexcept {
				return (pointer)&m_data[0];
			}

			GAIA_NODISCARD constexpr const_pointer data() const noexcept {
				return (const_pointer)&m_data[0];
			}

			GAIA_NODISCARD constexpr auto operator[](size_type pos) noexcept -> decltype(view_policy::set({}, 0)) {
				GAIA_ASSERT(pos < size());
				return view_policy::set({(typename view_policy::TargetCastType) & m_data[0], extent}, pos);
			}

			GAIA_NODISCARD constexpr auto operator[](size_type pos) const noexcept -> decltype(view_policy::get({}, 0)) {
				GAIA_ASSERT(pos < size());
				return view_policy::get({(typename view_policy::TargetCastType) & m_data[0], extent}, pos);
			}

			GAIA_NODISCARD constexpr size_type size() const noexcept {
				return N;
			}

			GAIA_NODISCARD constexpr bool empty() const noexcept {
				return begin() == end();
			}

			GAIA_NODISCARD constexpr size_type max_size() const noexcept {
				return N;
			}

			GAIA_NODISCARD constexpr auto front() noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return *begin();
				else
					return (reference)*begin();
			}

			GAIA_NODISCARD constexpr auto front() const noexcept {

				if constexpr (utils::is_soa_layout_v<T>)
					return *begin();
				else
					return (const_reference)*begin();
			}

			GAIA_NODISCARD constexpr auto back() noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return (operator[])(N - 1);
				else
					return (reference) operator[](N - 1);
			}

			GAIA_NODISCARD constexpr auto back() const noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return operator[](N - 1);
				else
					return (const_reference) operator[](N - 1);
			}

			GAIA_NODISCARD constexpr auto begin() const noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return iterator_soa(m_data, size(), 0);
				else
					return iterator((pointer)&m_data[0]);
			}

			GAIA_NODISCARD constexpr auto rbegin() const noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return iterator_soa(m_data, size(), size() - 1);
				else
					return iterator((pointer)&back());
			}

			GAIA_NODISCARD constexpr auto end() const noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return iterator_soa(m_data, size(), size());
				else
					return iterator((pointer)&m_data[0] + size());
			}

			GAIA_NODISCARD constexpr auto rend() const noexcept {
				if constexpr (utils::is_soa_layout_v<T>)
					return iterator_soa(m_data, size(), -1);
				else
					return iterator((pointer)m_data - 1);
			}

			GAIA_NODISCARD constexpr bool operator==(const sarr& other) const {
				for (size_type i = 0; i < N; ++i)
					if (!(m_data[i] == other.m_data[i]))
						return false;
				return true;
			}

			template <size_t Item>
			auto soa_view_mut() noexcept {
				return utils::data_view_policy<T::Layout, T>::template get<Item>(
						std::span<uint8_t>{(uint8_t*)&m_data[0], extent});
			}

			template <size_t Item>
			auto soa_view() const noexcept {
				return utils::data_view_policy<T::Layout, T>::template get<Item>(
						std::span<const uint8_t>{(const uint8_t*)&m_data[0], extent});
			}
		};

		namespace detail {
			template <typename T, uint32_t N, uint32_t... I>
			constexpr sarr<std::remove_cv_t<T>, N> to_array_impl(T (&a)[N], std::index_sequence<I...> /*no_name*/) {
				return {{a[I]...}};
			}
		} // namespace detail

		template <typename T, uint32_t N>
		constexpr sarr<std::remove_cv_t<T>, N> to_array(T (&a)[N]) {
			return detail::to_array_impl(a, std::make_index_sequence<N>{});
		}

		template <typename T, typename... U>
		sarr(T, U...) -> sarr<T, 1 + (uint32_t)sizeof...(U)>;

	} // namespace containers
} // namespace gaia

namespace std {
	template <typename T, uint32_t N>
	struct tuple_size<gaia::containers::sarr<T, N>>: std::integral_constant<uint32_t, N> {};

	template <size_t I, typename T, uint32_t N>
	struct tuple_element<I, gaia::containers::sarr<T, N>> {
		using type = T;
	};
} // namespace std
