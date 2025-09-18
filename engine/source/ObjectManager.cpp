#include "ObjectManager.hpp"
#include <algorithm>

Object* ObjectManager::FindObject(int id)
{
    Object* obj = objects.at(id).get();
    if (obj == nullptr) 
    {
        return nullptr;
    }
    return obj;

}

Object* ObjectManager::FindObjectByName(const std::string& name)
{
    for (const auto& objectPtr : objects)
    {
        if (objectPtr->GetName() == name)
        {
            return objectPtr.get();
        }
    }
    return nullptr;
}

void ObjectManager::DestroyObject(Object* object)
{
    if (object == nullptr)
    {
        return;
    }
    removalQueue.push_back(object);
}

void ObjectManager::Update(float dt)
{
    for (auto& object : objects)
    {
        object->Update(dt);
    }
}

void ObjectManager::DestroyAllObjects()
{
    lastObjectID = 0;
    for (auto& obj : objects)
    {
        obj.reset();
    }
    removalQueue.clear();
    objects.clear();
}

void ObjectManager::ProcessQueues()
{
    for (const auto& func : commandQueue)
    {
        func();
    }
    commandQueue.clear();

    if (!removalQueue.empty())
    {
        objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                [&](const std::unique_ptr<Object>& object) {
            return std::find(removalQueue.begin(), removalQueue.end(), object.get()) != removalQueue.end();
        }),
            objects.end()
        );
        removalQueue.clear();
    }
}
