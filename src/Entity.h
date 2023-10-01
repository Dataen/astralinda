#ifndef LIMITEDSPACE_ENTITY_H
#define LIMITEDSPACE_ENTITY_H

#include <entt/entt.hpp>

struct Entity
{
public:
    Entity() = default;
    explicit Entity(entt::entity handle, entt::registry* registry)
            : handle(handle)
            , registry(registry)
    {}
    Entity(const Entity& other) = default;

    operator bool() const { return handle != entt::null; }
    operator entt::entity() const { return handle; }
    operator std::uint32_t () const { return (std::uint32_t) handle; }

    bool operator==(const Entity& other) const
    {
        return handle == other.handle && registry == other.registry;
    }

    bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }

    template<typename T, typename... Args>
    T& add_component(Args&&... args)
    {
        T& component = registry->emplace<T>(handle, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    void remove_component()
    {
        registry->remove<T>(handle);
    }

    template<typename T>
    T& get_component()
    {
        return registry->get<T>(handle);
    }

    template<typename T>
    const T& get_component() const
    {
        return registry->get<T>(handle);
    }

    template<typename T>
    bool has_component() const
    {
        return registry->any_of<T>(handle);
    }


private:
    entt::entity handle{ entt::null };
    entt::registry* registry;
};


#endif //LIMITEDSPACE_ENTITY_H
