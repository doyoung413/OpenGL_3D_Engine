#pragma once

#include <vector>
#include <memory>
#include <string>
#include "ObjectType.hpp"
#include "Transform.hpp"
#include "Component.hpp"

class Object
{
public:
    Object() = default;
    ~Object();
 
    virtual void Update(float dt);

    void SetType(ObjectType newType){ type = newType; }
    ObjectType GetType() const { return type; }

    void SetName(const std::string& newName) { name = newName; }
    const std::string& GetName() const { return name; }

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);
        newComponent->SetOwner(this);
        newComponent->Init();

        T* componentPtr = newComponent.get();
        components.push_back(std::move(newComponent));
        return componentPtr;
    }

    template <typename T>
    T* GetComponent()
    {
        for (const auto& component : components)
        {
            T* target = dynamic_cast<T*>(component.get());
            if (target)
            {
                return target;
            }
        }
        return nullptr;
    }

public:
    Transform transform;

private:
    std::vector<std::unique_ptr<Component>> components;
    ObjectType type = ObjectType::NONE;
    std::string name = "";
};
