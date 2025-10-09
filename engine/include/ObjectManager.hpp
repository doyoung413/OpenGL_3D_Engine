#pragma once
#include "Object.hpp"
#include <vector>
#include <memory>
#include <functional>

class Engine;
class ObjectManager
{
public:
    ObjectManager() = default;
    ~ObjectManager() = default;
    ObjectManager(const ObjectManager&) = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;

    template <typename T, typename... Args>
    void AddObject(Args... args)
    {
        objects.push_back(std::move(std::unique_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...))));
        ++lastObjectID;
    }

    template<typename ComponentTypes, typename Func>
    void QueueComponentFunction(ComponentTypes* component, Func&& func)
    {
        if (component == nullptr)
        {
            return;
        }

        commandQueue.push_back([component, func]()
        {
            func(component);
        });
    }

    template<typename T, typename Func>
    void QueueObjectFunction(T* object, Func&& func)
    {
        if (object == nullptr)
        {
            return;
        }
        commandQueue.push_back([object, func]()
        {
            func(object);
        });
    }

    Object* FindObject(int id);
    Object* FindObjectByName(const std::string& name);

    void DestroyObject(Object* object);
    void Update(float dt);
    void DestroyAllObjects();
    void ProcessQueues();

    const std::vector<std::unique_ptr<Object>>& GetObjectList() const { return objects; }
    void ObjectControllerForImgui();
private:
    friend class Engine;

    int lastObjectID = 0;
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::function<void()>> commandQueue;
    std::vector<Object*> removalQueue;

    Object* selectedObject = nullptr;
};
