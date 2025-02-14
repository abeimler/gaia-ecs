#pragma once
#include "../config/config.h"

#include <cstdint>
#include <cstring>
#include <stdlib.h>
#include <type_traits>
#include <utility>

#include "../config/profiler.h"

#if GAIA_PLATFORM_WINDOWS && GAIA_COMPILER_MSVC
	#define GAIA_MEM_ALLC(size) malloc(size)
	#define GAIA_MEM_FREE(ptr) free(ptr)

	// Clang with MSVC codegen needs some remapping
	#if !defined(aligned_alloc)
		#define GAIA_MEM_ALLC_A(size, alig) _aligned_malloc(size, alig)
		#define GAIA_MEM_FREE_A(ptr) _aligned_free(ptr)
	#else
		#define GAIA_MEM_ALLC_A(size, alig) aligned_alloc(alig, size)
		#define GAIA_MEM_FREE_A(ptr) aligned_free(ptr)
	#endif
#else
	#define GAIA_MEM_ALLC(size) malloc(size)
	#define GAIA_MEM_ALLC_A(size, alig) aligned_alloc(alig, size)
	#define GAIA_MEM_FREE(ptr) free(ptr)
	#define GAIA_MEM_FREE_A(ptr) free(ptr)
#endif

namespace gaia {
	namespace mem {
		inline void* mem_alloc(size_t size) {
			GAIA_ASSERT(size > 0);

			void* ptr = GAIA_MEM_ALLC(size);
			GAIA_ASSERT(ptr != nullptr);
			GAIA_PROF_ALLOC(ptr, size);
			return ptr;
		}

		inline void* mem_alloc_alig(size_t size, size_t alig) {
			GAIA_ASSERT(size > 0);
			GAIA_ASSERT(alig > 0);

			// Make sure size is a multiple of the alignment
			size = (size + alig - 1) & ~(alig - 1);
			void* ptr = GAIA_MEM_ALLC_A(size, alig);
			GAIA_ASSERT(ptr != nullptr);
			GAIA_PROF_ALLOC(ptr, size);
			return ptr;
		}

		inline void mem_free(void* ptr) {
			GAIA_ASSERT(ptr != nullptr);

			GAIA_MEM_FREE(ptr);
			GAIA_PROF_FREE(ptr);
		}

		inline void mem_free_alig(void* ptr) {
			GAIA_ASSERT(ptr != nullptr);
			
			GAIA_MEM_FREE_A(ptr);
			GAIA_PROF_FREE(ptr);
		}

		//! Align a number to the requested byte alignment
		//! \param num Number to align
		//! \param alignment Requested alignment
		//! \return Aligned number
		template <typename T, typename V>
		constexpr T align(T num, V alignment) {
			return alignment == 0 ? num : ((num + (alignment - 1)) / alignment) * alignment;
		}

		//! Align a number to the requested byte alignment
		//! \tparam alignment Requested alignment in bytes
		//! \param num Number to align
		//! return Aligned number
		template <size_t alignment, typename T>
		constexpr T align(T num) {
			return ((num + (alignment - 1)) & ~(alignment - 1));
		}

		//! Returns the padding
		//! \param num Number to align
		//! \param alignment Requested alignment
		//! \return Padding in bytes
		template <typename T, typename V>
		constexpr uint32_t padding(T num, V alignment) {
			return (uint32_t)(align(num, alignment) - num);
		}

		//! Returns the padding
		//! \tparam alignment Requested alignment in bytes
		//! \param num Number to align
		//! return Aligned number
		template <size_t alignment, typename T>
		constexpr uint32_t padding(T num) {
			return (uint32_t)(align<alignment>(num) - num);
		}

		//! Convert form type \tparam Src to type \tparam Dst without causing an undefined behavior
		template <typename Dst, typename Src>
		Dst bit_cast(const Src& src) {
			static_assert(sizeof(Dst) == sizeof(Src));
			static_assert(std::is_trivially_copyable_v<Src>);
			static_assert(std::is_trivially_copyable_v<Dst>);

			// int i = {};
			// float f = *(*float)&i; // undefined behavior
			// memcpy(&f, &i, sizeof(float)); // okay
			Dst dst;
			memmove((void*)&dst, (const void*)&src, sizeof(Dst));
			return dst;
		}

		//! Pointer wrapper for writing memory in defined way (not causing undefined behavior)
		template <typename T>
		class unaligned_ref {
			void* m_p;

		public:
			unaligned_ref(void* p): m_p(p) {}

			unaligned_ref& operator=(const T& value) {
				memmove(m_p, (const void*)&value, sizeof(T));
				return *this;
			}

			operator T() const {
				T tmp;
				memmove((void*)&tmp, (const void*)m_p, sizeof(T));
				return tmp;
			}
		};
	} // namespace mem
} // namespace gaia
