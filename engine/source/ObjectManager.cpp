#include "ObjectManager.hpp"

#include "Mesh.hpp"   
#include "Shader.hpp" 
#include "Engine.hpp"
#include "RenderManager.hpp"
#include "Camera.hpp"
#include "Transform.hpp"
#include "MeshRenderer.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "Animator.hpp"
#include "Animation.hpp"

#include <assimp/scene.h> 
#include "imgui.h"
#include <glew.h>
#include <algorithm>


ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::Init()
{
	gizmoArrowCone = std::make_unique<Mesh>();
	gizmoArrowCone->CreateCone();
	gizmoArrowCone->UploadToGPU();

	gizmoArrowCylinder = std::make_unique<Mesh>();
	gizmoArrowCylinder->CreateCylinder();
	gizmoArrowCylinder->UploadToGPU();
	
	Engine::GetInstance().GetRenderManager()->LoadShader("unlit", "asset/shaders/unlit_debug.vert", "asset/shaders/unlit_debug.frag");
	gizmoShader = Engine::GetInstance().GetRenderManager()->GetShader("unlit");
	debugShader = Engine::GetInstance().GetRenderManager()->GetShader("unlit");

	// 뼈 위치에 그려줄 작은 구 메쉬 생성 및 GPU 업로드
	boneMesh = std::make_unique<Mesh>();
	boneMesh->CreateDiamond();
	boneMesh->UploadToGPU();
}

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
			if(selectedObject != nullptr)
			{
				MeshRenderer* renderer = selectedObject->GetComponent<MeshRenderer>();
				if (renderer)
				{
					if (prevShader != nullptr)
					{
						renderer->SetShader(prevShader);
					}
					else
					{
						renderer->SetShader("basic");
					}
				}
			}
			bDrawSkeleton = false;
			prevShader = nullptr;
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
				//if (!selectedObject)
				{
					if (ImGui::Button("ShowWeight"))
					{
						isWeightDebugMode = !isWeightDebugMode; // 모드 전환

						MeshRenderer* renderer = selectedObject->GetComponent<MeshRenderer>();
						if(renderer)
						{
							if (isWeightDebugMode)
							{
								prevShader = renderer->GetShader();
								renderer->SetShader("weight_debug");
							}
							else
							{
								if (prevShader != nullptr)
								{
									renderer->SetShader(prevShader);
									prevShader = nullptr;
								}
								else
								{
									renderer->SetShader("basic");
									prevShader = nullptr;
								}
							}
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("ShowSkeletonBone"))
					{
						bDrawSkeleton = !bDrawSkeleton;
					}
				}
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

void ObjectManager::RenderGizmos(Camera* camera)
{
	// 선택된 객체가 없거나, 그리는 데 필요한 메쉬/셰이더가 준비되지 않았다면 함수를 종료
	if (!selectedObject || !gizmoShader || !gizmoArrowCone || !gizmoArrowCylinder)
	{
		return;
	}

	glDisable(GL_DEPTH_TEST);

	gizmoShader->Bind();
	gizmoShader->SetUniformMat4f("view", camera->GetViewMatrix());
	gizmoShader->SetUniformMat4f("projection", camera->GetProjectionMatrix());
	gizmoShader->SetUniform1i("useTexture", 0);

	VertexArray* coneVA = gizmoArrowCone->GetVertexArray();
	VertexArray* cylinderVA = gizmoArrowCylinder->GetVertexArray();

	if (!coneVA || !cylinderVA) return;

	// 선택된 객체의 위치를 가져옵니다. 이 위치가 기즈모의 중심
	glm::vec3 objectPosition = selectedObject->transform.GetPosition();
	glm::quat objectRotationQuat = glm::quat(glm::radians(selectedObject->transform.GetRotation()));
	glm::mat4 objectRotationMatrix = glm::mat4_cast(objectRotationQuat);

	const float gizmoLineLength = 1.0f; // 축의 길이
	const float gizmoLineRadius = 0.02f; // 축의 두께
	const float gizmoArrowHeadRadius = 0.05f; // 화살촉의 반지름
	const float gizmoArrowHeadLength = 0.2f; // 화살촉의 길이

	// X축
	gizmoShader->SetUniformVec4("color", { 1.0f, 0.0f, 0.0f, 1.0f });
	// 원기둥 (몸통)
	glm::mat4 modelMatrix_X_Cyl = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { gizmoLineLength / 2.0f, 0.0f, 0.0f }) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), { 0.0f, 1.0f, 0.0f }) * glm::scale(glm::mat4(1.0f), { gizmoLineRadius, gizmoLineRadius, gizmoLineLength });
	gizmoShader->SetUniformMat4f("model", modelMatrix_X_Cyl);
	cylinderVA->Bind();
	glDrawElements(GL_TRIANGLES, cylinderVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);
	// 원뿔 (화살촉)
	glm::mat4 modelMatrix_X_Cone = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { gizmoLineLength, 0.0f, 0.0f }) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { gizmoArrowHeadRadius, gizmoArrowHeadLength, gizmoArrowHeadRadius });
	gizmoShader->SetUniformMat4f("model", modelMatrix_X_Cone);
	coneVA->Bind();
	glDrawElements(GL_TRIANGLES, coneVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);

	// Y축
	gizmoShader->SetUniformVec4("color", { 0.0f, 1.0f, 0.0f, 1.0f });
	// 원기둥 (몸통)
	glm::mat4 modelMatrix_Y_Cyl = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { 0.0f, gizmoLineLength / 2.0f, 0.0f }) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), { 1.0f, 0.0f, 0.0f }) * glm::scale(glm::mat4(1.0f), { gizmoLineRadius, gizmoLineRadius, gizmoLineLength });
	gizmoShader->SetUniformMat4f("model", modelMatrix_Y_Cyl);
	cylinderVA->Bind();
	glDrawElements(GL_TRIANGLES, cylinderVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);
	// 원뿔 (화살촉)
	glm::mat4 modelMatrix_Y_Cone = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { 0.0f, gizmoLineLength, 0.0f }) * glm::scale(glm::mat4(1.0f), { gizmoArrowHeadRadius, gizmoArrowHeadLength, gizmoArrowHeadRadius });
	gizmoShader->SetUniformMat4f("model", modelMatrix_Y_Cone);
	coneVA->Bind();
	glDrawElements(GL_TRIANGLES, coneVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);

	// Z축
	gizmoShader->SetUniformVec4("color", { 0.0f, 0.0f, 1.0f, 1.0f });
	// 원기둥 (몸통)
	glm::mat4 modelMatrix_Z_Cyl = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, gizmoLineLength / 2.0f }) * glm::scale(glm::mat4(1.0f), { gizmoLineRadius, gizmoLineRadius, gizmoLineLength });
	gizmoShader->SetUniformMat4f("model", modelMatrix_Z_Cyl);
	cylinderVA->Bind();
	glDrawElements(GL_TRIANGLES, cylinderVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);
	// 원뿔 (화살촉)
	glm::mat4 modelMatrix_Z_Cone = glm::translate(glm::mat4(1.0f), objectPosition) * objectRotationMatrix * glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, gizmoLineLength }) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), { 1.0f, 0.0f, 0.0f }) * glm::scale(glm::mat4(1.0f), { gizmoArrowHeadRadius, gizmoArrowHeadLength, gizmoArrowHeadRadius });
	gizmoShader->SetUniformMat4f("model", modelMatrix_Z_Cone);
	coneVA->Bind();
	glDrawElements(GL_TRIANGLES, coneVA->GetIndicesCount(), GL_UNSIGNED_INT, 0);


	// 다음 렌더링에 영향을 주지 않도록 깊이 테스트를 다시 활성화
	glEnable(GL_DEPTH_TEST);
	coneVA->UnBind(); // 마지막에 사용한 VAO 언바인드
}


inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

void ObjectManager::RenderBoneHierarchy(Camera* camera)
{
	if (!bDrawSkeleton || !camera || !selectedObject) return;

	// 선택된 오브젝트에 Animator 컴포넌트가 있는지 확인
	if (selectedObject->HasComponent<Animator>())
	{
		Animator* animator = selectedObject->GetComponent<Animator>();
		Animation* animation = animator->GetCurrentAnimation();
		if (!animation) return;

		glDisable(GL_DEPTH_TEST);

		// Animator로부터 애니메이션이 적용된 뼈 행렬 맵을 가져옴
		const auto& animatedTransforms = animator->GetGlobalBoneTransforms();
		// 애니메이션의 뼈대 계층 구조의 루트 노드를 가져옴
		const AssimpNodeData* rootNode = &animation->GetRootNode();

		// 새 데이터를 사용하여 뼈 그리기 시작
		DrawBoneHierarchy(rootNode, animatedTransforms, camera);

		glEnable(GL_DEPTH_TEST);
	}
}

void ObjectManager::DrawBoneHierarchy(const AssimpNodeData* node, const std::map<std::string, glm::mat4>& animatedTransforms, Camera* camera)
{
	// 현재 노드 이름이 Animator가 제공한 맵에 있는지 확인
	if (animatedTransforms.count(node->name))
	{
		// T-포즈 행렬 대신, 애니메이션이 적용된 월드 변환 행렬을 가져옴
		glm::mat4 globalTransform = animatedTransforms.at(node->name);

		// 기존 뼈 그리기 로직 (이제 애니메이션된 위치에 다이아몬드를 그림)
		if (debugShader && boneMesh)
		{
			debugShader->Bind();
			debugShader->SetUniformMat4f("view", camera->GetViewMatrix());
			debugShader->SetUniformMat4f("projection", camera->GetProjectionMatrix());
			debugShader->SetUniform1i("useTexture", 0);
			debugShader->SetUniformVec4("color", { 0.0f, 1.0f, 0.0f, 1.0f });

			glm::mat4 boneModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(globalTransform[3]));
			boneModelMatrix = glm::scale(boneModelMatrix, glm::vec3(0.03f));
			debugShader->SetUniformMat4f("model", boneModelMatrix);

			VertexArray* va = boneMesh->GetVertexArray();
			if (va)
			{
				va->Bind();
				glDrawElements(static_cast<GLenum>(boneMesh->GetPrimitivePattern()), boneMesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);
				va->UnBind();
			}
		}
	}

	// 자식 노드들에 대해 재귀 호출 (aiNode의 mChildren 대신 AssimpNodeData의 children 사용)
	for (const auto& child : node->children)
	{
		DrawBoneHierarchy(&child, animatedTransforms, camera);
	}
}
