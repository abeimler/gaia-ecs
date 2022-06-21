#pragma once
#include <cinttypes>

#include "../config/config.h"
#include "../containers/map.h"
#include "../containers/sarray_ext.h"
#include "../utils/mem.h"
#include "archetype.h"
#include "common.h"
#include "component.h"
#include "component_cache.h"
#include "entity.h"
#include "fwd.h"
#include "world.h"

namespace gaia {
	namespace ecs {

		struct TempEntity final {
			uint32_t id;
		};

		/*!
		Buffer for deferred execution of some operations on entities.

		Adding and removing components and entities inside World::ForEach or can result
		in changes of archetypes or chunk structure. This would lead to undefined behavior.
		Therefore, such operations have to be executed after the loop is done.
		*/
		class CommandBuffer final {
			enum CommandBufferCmd : uint8_t {
				CREATE_ENTITY,
				CREATE_ENTITY_FROM_ARCHETYPE,
				CREATE_ENTITY_FROM_ENTITY,
				DELETE_ENTITY,
				ADD_COMPONENT,
				ADD_COMPONENT_DATA,
				ADD_COMPONENT_TO_TEMPENTITY,
				ADD_COMPONENT_TO_TEMPENTITY_DATA,
				SET_COMPONENT,
				SET_COMPONENT_FOR_TEMPENTITY,
				REMOVE_COMPONENT
			};

			friend class World;

			World& m_world;
			containers::darray<uint8_t> m_data;
			uint32_t m_entities;

			template <typename TEntity, typename... TComponent>
			void AddComponent_Internal(TEntity entity) {
				// Entity
				{
					const auto lastIndex = m_data.size();
					m_data.resize(m_data.size() + sizeof(TEntity));

					utils::unaligned_ref<TEntity> to(&m_data[lastIndex]);
					to = entity;
				}
				// Components
				{
					const ComponentInfo* infosToAdd[] = {GetComponentCache(m_world).GetOrCreateComponentInfo<TComponent>()...};

					// Component count
					constexpr auto componentCount = (uint8_t)sizeof...(TComponent);
					m_data.push_back(componentCount);

					// Component info
					auto lastIndex = m_data.size();
					m_data.resize(m_data.size() + sizeof(ComponentInfo*) * componentCount);

					for (uint8_t i = 0U; i < componentCount; i++) {
						utils::unaligned_ref<const ComponentInfo*> to(&m_data[lastIndex]);
						lastIndex += sizeof(const ComponentInfo*);

						to = infosToAdd[i];
					}
				}
			}

			template <typename T>
			void SetComponentFinal_Internal(uint32_t& index, T&& data) {
				using U = std::decay_t<T>;

				// Component info
				{
					utils::unaligned_ref<uint32_t> mem((void*)&m_data[index]);
					mem = utils::type_info::index<U>();
				}

				// Component data
				{
					utils::unaligned_ref<U> mem((void*)&m_data[index + sizeof(uint32_t)]);
					mem = std::forward<U>(data);
				}

				index += sizeof(uint32_t) + sizeof(U);
			}

			template <typename... TComponent>
			void SetComponentNoEntityNoSize_Internal(TComponent&&... data) {
				// Data size
				auto lastIndex = (uint32_t)m_data.size();

				constexpr auto ComponentsSize = (sizeof(TComponent) + ...);
				constexpr auto ComponentsCount = sizeof...(TComponent);
				constexpr auto ComponentTypeIdxSize = sizeof(uint32_t);
				constexpr auto AddSize = ComponentsSize + ComponentsCount * ComponentTypeIdxSize;
				m_data.resize(m_data.size() + AddSize);

				// Component data
				(this->SetComponentFinal_Internal<TComponent>(lastIndex, std::forward<TComponent>(data)), ...);
			}

			template <typename... TComponent>
			void SetComponentNoEntity_Internal(TComponent&&... data) {
				// Register components
				((void)GetComponentCache(m_world).GetOrCreateComponentInfo<TComponent>(), ...);

				// Component count
				constexpr auto NComponents = (uint8_t)sizeof...(TComponent);
				m_data.push_back(NComponents);

				// Data
				SetComponentNoEntityNoSize_Internal(std::forward<TComponent>(data)...);
			}

			template <typename TEntity, typename... TComponent>
			void SetComponent_Internal(TEntity entity, TComponent&&... data) {
				// Entity
				{
					const auto lastIndex = m_data.size();
					m_data.resize(m_data.size() + sizeof(TEntity));

					utils::unaligned_ref<TEntity> to(&m_data[lastIndex]);
					to = entity;
				}

				// Components
				SetComponentNoEntity_Internal(std::forward<TComponent>(data)...);
			}

			template <typename... TComponent>
			void RemoveComponent_Internal(Entity entity) {
				// Entity
				{
					const auto lastIndex = m_data.size();
					m_data.resize(m_data.size() + sizeof(Entity));

					utils::unaligned_ref<Entity> to(&m_data[lastIndex]);
					to = entity;
				}
				// Components
				{
					const ComponentInfo* typesToRemove[] = {GetComponentCache(m_world).GetComponentInfo<TComponent>()...};

					// Component count
					constexpr auto NComponents = (uint8_t)sizeof...(TComponent);
					m_data.push_back(NComponents);

					// Component info
					auto lastIndex = m_data.size();
					m_data.resize(m_data.size() + sizeof(ComponentInfo*) * NComponents);
					for (uint8_t i = 0U; i < NComponents; i++) {
						utils::unaligned_ref<const ComponentInfo*> to(&m_data[lastIndex]);
						lastIndex += sizeof(const ComponentInfo*);

						GAIA_ASSERT(typesToRemove[i]);
						to = typesToRemove[i];
					}
				}
			}

			/*!
			Requests a new entity to be created from archetype
			\return Entity that will be created. The id is not usable right away. It
			will be filled with proper data after Commit()
			*/
			[[nodiscard]] TempEntity CreateEntity(Archetype& archetype) {
				m_data.push_back(CREATE_ENTITY_FROM_ARCHETYPE);
				const auto archetypeSize = sizeof(void*); // we'll serialize just the pointer
				const auto lastIndex = m_data.size();
				m_data.resize(m_data.size() + archetypeSize);

				utils::unaligned_ref<uintptr_t> to(&m_data[lastIndex]);
				to = (uintptr_t)&archetype;

				return {m_entities++};
			}

		public:
			CommandBuffer(World& world): m_world(world), m_entities(0) {
				m_data.reserve(256);
			}

			CommandBuffer(CommandBuffer&&) = delete;
			CommandBuffer(const CommandBuffer&) = delete;
			CommandBuffer& operator=(CommandBuffer&&) = delete;
			CommandBuffer& operator=(const CommandBuffer&) = delete;

			/*!
			Requests a new entity to be created
			\return Entity that will be created. The id is not usable right away. It
			will be filled with proper data after Commit()
			*/
			[[nodiscard]] TempEntity CreateEntity() {
				m_data.push_back(CREATE_ENTITY);
				return {m_entities++};
			}

			/*!
			Requests a new entity to be created by cloning an already existing
			entity \return Entity that will be created. The id is not usable right
			away. It will be filled with proper data after Commit()
			*/
			[[nodiscard]] TempEntity CreateEntity(Entity entityFrom) {
				m_data.push_back(CREATE_ENTITY_FROM_ENTITY);
				const auto entitySize = sizeof(entityFrom);
				const auto lastIndex = m_data.size();
				m_data.resize(m_data.size() + entitySize);

				utils::unaligned_ref<Entity> to(&m_data[lastIndex]);
				to = entityFrom;

				return {m_entities++};
			}

			/*!
			Requests an existing \param entity to be removed.
			*/
			void DeleteEntity(Entity entity) {
				m_data.push_back(DELETE_ENTITY);
				const auto entitySize = sizeof(entity);
				const auto lastIndex = m_data.size();
				m_data.resize(m_data.size() + entitySize);

				utils::unaligned_ref<Entity> to(&m_data[lastIndex]);
				to = entity;
			}

			/*!
			Requests a component to be added to entity.

			\return True if component could be added (e.g. maximum component count
			on the archetype not exceeded). False otherwise.
			*/
			template <typename TComponent>
			bool AddComponent(Entity entity) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(ADD_COMPONENT);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				AddComponent_Internal<Entity, U>(entity);
				return true;
			}

			/*!
			Requests a component to be added to temporary entity.

			\return True if component could be added (e.g. maximum component count
			on the archetype not exceeded). False otherwise.
			*/
			template <typename TComponent>
			bool AddComponent(TempEntity entity) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(ADD_COMPONENT_TO_TEMPENTITY);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				AddComponent_Internal<TempEntity, U>(entity);
				return true;
			}

			/*!
			Requests a component to be added to entity.

			\return True if component could be added (e.g. maximum component count
			on the archetype not exceeded). False otherwise.
			*/
			template <typename TComponent>
			bool AddComponent(Entity entity, TComponent&& data) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(ADD_COMPONENT_DATA);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				AddComponent_Internal<Entity, U>(entity);
				SetComponentNoEntityNoSize_Internal(std::forward<U>(data));
				return true;
			}

			/*!
			Requests a component to be added to temporary entity.

			\return True if component could be added (e.g. maximum component count
			on the archetype not exceeded). False otherwise.
			*/
			template <typename TComponent>
			bool AddComponent(TempEntity entity, TComponent&& data) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(ADD_COMPONENT_TO_TEMPENTITY_DATA);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				AddComponent_Internal<TempEntity, U>(entity);
				SetComponentNoEntityNoSize_Internal(std::forward<U>(data));
				return true;
			}

			/*!
			Requests component data to be set to given values for a given entity.

			\warning Just like World::SetComponent, this method expects the
			given component infos to exist. Undefined behavior otherwise.
			*/
			template <typename TComponent>
			void SetComponent(Entity entity, TComponent&& data) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(SET_COMPONENT);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				SetComponent_Internal(entity, std::forward<U>(data));
			}

			/*!
			Requests component data to be set to given values for a given temp
			entity.

			\warning Just like World::SetComponent, this method expects the
			given component infos to exist. Undefined behavior otherwise.
			*/
			template <typename TComponent>
			void SetComponent(TempEntity entity, TComponent&& data) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(SET_COMPONENT_FOR_TEMPENTITY);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				SetComponent_Internal(entity, std::forward<U>(data));
			}

			/*!
			Requests removal of a component from entity
			*/
			template <typename TComponent>
			void RemoveComponent(Entity entity) {
				using U = typename DeduceComponent<TComponent>::Type;
				VerifyComponents<U>();

				m_data.push_back(REMOVE_COMPONENT);
				if constexpr (IsGenericComponent<TComponent>::value)
					m_data.push_back(ComponentType::CT_Generic);
				else
					m_data.push_back(ComponentType::CT_Chunk);
				RemoveComponent_Internal<U>(entity);
			}

			/*!
			Commits all queued changes.
			*/
			void Commit() {
				containers::map<uint32_t, Entity> entityMap;
				uint32_t entities = 0;

				// Extract data from the buffer
				for (auto i = 0U; i < m_data.size();) {
					const auto cmd = m_data[i++];
					switch (cmd) {
						case CREATE_ENTITY: {
							[[maybe_unused]] const auto res = entityMap.emplace(entities++, m_world.CreateEntity());
							GAIA_ASSERT(res.second);
						} break;
						case CREATE_ENTITY_FROM_ARCHETYPE: {
							uintptr_t ptr = utils::unaligned_ref<uintptr_t>((void*)&m_data[i]);
							Archetype* archetype = (Archetype*)ptr;
							i += sizeof(void*);
							[[maybe_unused]] const auto res = entityMap.emplace(entities++, m_world.CreateEntity(*archetype));
							GAIA_ASSERT(res.second);
						} break;
						case CREATE_ENTITY_FROM_ENTITY: {
							Entity entityFrom = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);
							[[maybe_unused]] const auto res = entityMap.emplace(entities++, m_world.CreateEntity(entityFrom));
							GAIA_ASSERT(res.second);
						} break;
						case DELETE_ENTITY: {
							Entity entity = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);
							m_world.DeleteEntity(entity);
						} break;
						case ADD_COMPONENT:
						case ADD_COMPONENT_DATA: {
							// Type
							ComponentType type = (ComponentType)m_data[i];
							i += sizeof(ComponentType);
							// Entity
							Entity entity = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);

							// Component count
							uint8_t componentCount = m_data[i++];

							// Components
							auto newInfos = (const ComponentInfo**)alloca(sizeof(ComponentInfo) * componentCount);
							for (uint8_t j = 0; j < componentCount; ++j) {
								const auto info = *(const ComponentInfo**)&m_data[i];
								newInfos[j] = info;
								i += sizeof(const ComponentInfo*);
							}
							m_world.AddComponent_Internal(type, entity, {newInfos, (uintptr_t)componentCount});

							uint32_t indexInChunk;
							auto* pChunk = m_world.GetEntityChunk(entity, indexInChunk);
							GAIA_ASSERT(pChunk != nullptr);

							if (type == ComponentType::CT_Chunk)
								indexInChunk = 0;

							if (cmd == ADD_COMPONENT_DATA) {
								for (uint8_t j = 0; j < componentCount; ++j) {
									// Skip the component index
									// TODO: Don't include the component index here
									uint32_t infoIndex = utils::unaligned_ref<uint32_t>((void*)&m_data[i]);
									(void)infoIndex;
									i += sizeof(uint32_t);

									const auto* info = newInfos[j];
									auto* pComponentDataStart = pChunk->ViewRW_Internal(info, type);
									auto* pComponentData = (void*)&pComponentDataStart[indexInChunk * info->properties.size];
									memcpy(pComponentData, (const void*)&m_data[i], info->properties.size);
									i += info->properties.size;
								}
							}
						} break;
						case ADD_COMPONENT_TO_TEMPENTITY:
						case ADD_COMPONENT_TO_TEMPENTITY_DATA: {
							// Type
							ComponentType type = (ComponentType)m_data[i];
							i += sizeof(ComponentType);
							// Entity
							Entity e = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);

							// For delayed entities we have to do a look in our map
							// of temporaries and find a link there
							const auto it = entityMap.find(e.id());
							// Link has to exist!
							GAIA_ASSERT(it != entityMap.end());

							Entity entity = it->second;

							// Component count
							uint8_t componentCount = m_data[i++];

							// Components
							auto newInfos = (const ComponentInfo**)alloca(sizeof(ComponentInfo) * componentCount);
							for (uint8_t j = 0; j < componentCount; ++j) {
								newInfos[j] = utils::unaligned_ref<const ComponentInfo*>((void*)&m_data[i]);
								i += sizeof(const ComponentInfo*);
							}
							m_world.AddComponent_Internal(type, entity, {newInfos, (uintptr_t)componentCount});

							uint32_t indexInChunk;
							auto* pChunk = m_world.GetEntityChunk(entity, indexInChunk);
							GAIA_ASSERT(pChunk != nullptr);

							if (type == ComponentType::CT_Chunk)
								indexInChunk = 0;

							if (cmd == ADD_COMPONENT_TO_TEMPENTITY_DATA) {
								for (uint8_t j = 0; j < componentCount; ++j) {
									// Skip the type index
									// TODO: Don't include the type index here
									uint32_t infoIndex = utils::unaligned_ref<uint32_t>((void*)&m_data[i]);
									(void)infoIndex;
									i += sizeof(uint32_t);

									const auto* info = newInfos[j];
									auto* pComponentDataStart = pChunk->ViewRW_Internal(info, type);
									auto* pComponentData = (void*)&pComponentDataStart[indexInChunk * info->properties.size];
									memcpy(pComponentData, (const void*)&m_data[i], info->properties.size);
									i += info->properties.size;
								}
							}
						} break;
						case SET_COMPONENT: {
							// Type
							ComponentType type = (ComponentType)m_data[i];
							i += sizeof(ComponentType);
							// Entity
							Entity entity = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);

							const auto& entityContainer = m_world.m_entities[entity.id()];
							auto* pChunk = entityContainer.pChunk;
							const auto indexInChunk = type == ComponentType::CT_Chunk ? 0 : entityContainer.idx;

							// Component count
							const uint8_t componentCount = m_data[i++];

							// Components
							for (uint8_t j = 0; j < componentCount; ++j) {
								const auto infoIndex = utils::unaligned_ref<uint32_t>((void*)&m_data[i]);
								const auto* info = GetComponentCache(m_world).GetComponentInfoFromIdx(infoIndex);
								i += sizeof(uint32_t);

								auto* pComponentDataStart = pChunk->ViewRW_Internal(info, type);
								auto* pComponentData = (void*)&pComponentDataStart[indexInChunk * info->properties.size];
								memcpy(pComponentData, (const void*)&m_data[i], info->properties.size);
								i += info->properties.size;
							}
						} break;
						case SET_COMPONENT_FOR_TEMPENTITY: {
							// Type
							ComponentType type = (ComponentType)m_data[i];
							i += sizeof(ComponentType);
							// Entity
							Entity e = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);

							// For delayed entities we have to do a look in our map
							// of temporaries and find a link there
							const auto it = entityMap.find(e.id());
							// Link has to exist!
							GAIA_ASSERT(it != entityMap.end());

							Entity entity = it->second;

							const auto& entityContainer = m_world.m_entities[entity.id()];
							auto* pChunk = entityContainer.pChunk;
							const auto indexInChunk = type == ComponentType::CT_Chunk ? 0 : entityContainer.idx;

							// Component count
							const uint8_t componentCount = m_data[i++];

							// Components
							for (uint8_t j = 0; j < componentCount; ++j) {
								const auto infoIndex = utils::unaligned_ref<uint32_t>((void*)&m_data[i]);
								const auto* info = GetComponentCache(m_world).GetComponentInfoFromIdx(infoIndex);
								i += sizeof(uint32_t);

								auto* pComponentDataStart = pChunk->ViewRW_Internal(info, type);
								auto* pComponentData = (void*)&pComponentDataStart[indexInChunk * info->properties.size];
								memcpy(pComponentData, (const void*)&m_data[i], info->properties.size);
								i += info->properties.size;
							}
						} break;
						case REMOVE_COMPONENT: {
							// Type
							ComponentType type = utils::unaligned_ref<ComponentType>((void*)&m_data[i]);
							i += sizeof(ComponentType);
							// Entity
							Entity e = utils::unaligned_ref<Entity>((void*)&m_data[i]);
							i += sizeof(Entity);

							// Component count
							uint8_t componentCount = m_data[i++];

							// Components
							containers::sarray_ext<const ComponentInfo*, MAX_COMPONENTS_PER_ARCHETYPE> newInfos;
							newInfos.resize(componentCount);

							for (uint8_t j = 0; j < componentCount; ++j) {
								newInfos[j] = utils::unaligned_ref<const ComponentInfo*>((void*)&m_data[i]);
								i += sizeof(const ComponentInfo*);
							}
							m_world.RemoveComponent_Internal(type, e, {newInfos.data(), (uintptr_t)componentCount});
						} break;
					}
				}

				m_entities = 0;
				m_data.clear();
			}
		};
	} // namespace ecs
} // namespace gaia
