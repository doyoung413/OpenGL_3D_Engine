#pragma once
#include "Object.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <functional>
#include "glm.hpp"

class Engine;
class Mesh;
class Shader;
class Camera;
struct aiNode;
struct AssimpNodeData;
class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();
    ObjectManager(const ObjectManager&) = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;

    void Init();

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
    
    //void DrawBoneHierarchy(const AssimpNodeData* node, const std::map<std::string, glm::mat4>& animatedTransforms, Camera* camera);
    const std::vector<std::unique_ptr<Object>>& GetObjectList() const { return objects; }
    void ObjectControllerForImgui();
    
    // Gizmo
    void RenderGizmos(Camera* camera);
    void RenderBoneHierarchy(Camera* camera);

private:
    friend class Engine;

    int lastObjectID = 0;
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::function<void()>> commandQueue;
    std::vector<Object*> removalQueue;

    Object* selectedObject = nullptr;
    std::shared_ptr<Shader> prevShader = nullptr;

    // Gizmo
    std::unique_ptr<Mesh> gizmoArrowCone;
    std::unique_ptr<Mesh> gizmoArrowCylinder;
    std::shared_ptr<Shader> gizmoShader;

    //Born
    std::unique_ptr<Mesh> boneMesh;
    std::shared_ptr<Shader> debugShader;

    // 나머지 멤버 변수들
    bool bDrawSkeleton = false;
    bool isWeightDebugMode = false;

    // 뼈를 재귀적으로 그리는 헬퍼 함수
    void DrawBoneHierarchy(const AssimpNodeData* node, const std::map<std::string, glm::mat4>& animatedTransforms, Camera* camera);
};
