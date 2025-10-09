#include "ObjectManager.hpp"

#include "Transform.hpp"
#include "MeshRenderer.hpp"
#include "Light.hpp"

#include "imgui.h"
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

void ObjectManager::ObjectControllerForImgui()
{
    ImGui::Begin("Object Controller");
    ImGui::BeginChild("ObjectList", ImVec2(0, 125), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& objectPtr : objects)
    {
        Object* currentObject = objectPtr.get();
        std::string label = currentObject->GetName();
        if (label.empty())
        {
            label = "Unnamed Object";
        }

        std::string uniqueID = label + "##" + std::to_string(reinterpret_cast<uintptr_t>(currentObject));
        if (ImGui::Selectable(uniqueID.c_str(), selectedObject == currentObject))
        {
            selectedObject = currentObject;
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Inspector");
    if (selectedObject)
    {
        ImGui::Text("Name: %s", selectedObject->GetName().c_str());
        ImGui::Separator();

        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 pos = selectedObject->transform.GetPosition();
            float posArr[3] = { pos.x, pos.y, pos.z };
            if (ImGui::DragFloat3("Position", posArr, 0.1f))
            {
                selectedObject->transform.SetPosition({ posArr[0], posArr[1], posArr[2] });
            }

            glm::vec3 rot = selectedObject->transform.GetRotation();
            float rotArr[3] = { rot.x, rot.y, rot.z };
            if (ImGui::DragFloat3("Rotation", rotArr, 1.0f))
            {
                selectedObject->transform.SetRotation({ rotArr[0], rotArr[1], rotArr[2] });
            }

            glm::vec3 scale = selectedObject->transform.GetScale();
            float scaleArr[3] = { scale.x, scale.y, scale.z };
            if (ImGui::DragFloat3("Scale", scaleArr, 0.05f))
            {
                selectedObject->transform.SetScale({ scaleArr[0], scaleArr[1], scaleArr[2] });
            }
        }

        // MeshRenderer
        if (selectedObject->HasComponent<MeshRenderer>())
        {
            if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("This object can be rendered.");
            }
        }

        // Light
        if (selectedObject->HasComponent<Light>())
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Light* light = selectedObject->GetComponent<Light>();

                // 공통 속성
                glm::vec3 color = light->GetColor();
                float colorArr[3] = { color.x, color.y, color.z };
                if (ImGui::ColorEdit3("Color", colorArr))
                {
                    light->SetColor({ colorArr[0], colorArr[1], colorArr[2] });
                }

                float ambient = light->GetAmbientIntensity();
                if (ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f))
                {
                    light->SetAmbientIntensity(ambient);
                }

                float diffuse = light->GetDiffuseIntensity();
                if (ImGui::SliderFloat("Diffuse", &diffuse, 0.0f, 2.0f))
                {
                    light->SetDiffuseIntensity(diffuse);
                }

                // 빛 타입에 따른 개별 속성
                ImGui::Separator();
                if (light->GetType() == LightType::Point)
                {
                    ImGui::Text("Type: Point Light");
                    float specular = light->GetSpecularIntensity();
                    if (ImGui::SliderFloat("Specular", &specular, 0.0f, 2.0f))
                    {
                        light->SetSpecularIntensity(specular);
                    }

                    int shininess = light->GetShininess();
                    if (ImGui::SliderInt("Shininess", &shininess, 2, 256))
                    {
                        light->SetShininess(shininess);
                    }
                }
                else
                {
                    ImGui::Text("Type: Directional Light");
                    glm::vec3 direction = light->GetDirection();
                    float dirArr[3] = { direction.x, direction.y, direction.z };
                    if (ImGui::DragFloat3("Direction", dirArr, 0.01f, -1.0f, 1.0f))
                    {
                        light->SetDirection({ dirArr[0], dirArr[1], dirArr[2] });
                    }
                }
            }
        }
    }
    else
    {
        ImGui::Text("Select an object from the Hierarchy.");
    }
    ImGui::End();
}
