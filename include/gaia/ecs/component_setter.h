#pragma once
#include <cstdint>

#include "chunk.h"
#include "component.h"

namespace gaia {
	namespace ecs {
		struct ComponentSetter {
			archetype::Chunk* m_pChunk;
			uint32_t m_idx;

			//! Sets the value of the component \tparam T on \param entity.
			//! \tparam T Component
			//! \param value Value to set for the component
			//! \return ComponentSetter
			template <typename T, typename U = typename component::component_type_t<T>::Type>
			U& Set() {
				component::VerifyComponent<T>();

				if constexpr (component::component_type_v<T> == component::ComponentType::CT_Generic)
					return m_pChunk->template Set<T>(m_idx);
				else
					return m_pChunk->template Set<T>();
			}

			//! Sets the value of the component \tparam T on \param entity.
			//! \tparam T Component
			//! \param value Value to set for the component
			//! \return ComponentSetter
			template <typename T, typename U = typename component::component_type_t<T>::Type>
			ComponentSetter& Set(U&& data) {
				component::VerifyComponent<T>();

				if constexpr (component::component_type_v<T> == component::ComponentType::CT_Generic)
					m_pChunk->template Set<T>(m_idx, std::forward<U>(data));
				else
					m_pChunk->template Set<T>(std::forward<U>(data));
				return *this;
			}

			//! Sets the value of the component \tparam T on \param entity without trigger a world version update.
			//! \tparam T Component
			//! \param value Value to set for the component
			//! \return ComponentSetter
			template <typename T, typename U = typename component::component_type_t<T>::Type>
			ComponentSetter& SetSilent(U&& data) {
				component::VerifyComponent<T>();

				if constexpr (component::component_type_v<T> == component::ComponentType::CT_Generic)
					m_pChunk->template SetSilent<T>(m_idx, std::forward<U>(data));
				else
					m_pChunk->template SetSilent<T>(std::forward<U>(data));
				return *this;
			}
		};
	} // namespace ecs
} // namespace gaia