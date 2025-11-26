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
#include "AnimationStateMachine.hpp"

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
				}
			}
			selectedBoneName = "";
			bDrawSkeleton = false; 
			isWeightDebugMode = false;
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
					MeshRenderer* renderer = selectedObject->GetComponent<MeshRenderer>();
					if (ImGui::Button("ShowWeight"))
					{
						isWeightDebugMode = !isWeightDebugMode; // 모드 전환
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

					ImGui::Separator();
					// GetColor()는 const glm::vec4&를 반환
					glm::vec4 currentColor = renderer->GetColor();
					// ImGui 위젯은 float 배열을 사용
					float colorArr[4] = { currentColor.r, currentColor.g, currentColor.b, currentColor.a };

					// ImGui::ColorEdit4 위젯을 사용하여 색상(RGBA)을 편집
					if (ImGui::ColorEdit4("Color", colorArr))
					{
						// 위젯 값이 변경되면 SetColor 함수를 즉시 호출
						renderer->SetColor({ colorArr[0], colorArr[1], colorArr[2], colorArr[3] });
					}

					ImGui::Separator();
					ImGui::Text("Material Properties");

					float currentMetallic = renderer->GetMetallic();
					if (ImGui::SliderFloat("Metallic", &currentMetallic, 0.0f, 1.0f))
					{
						renderer->SetMetallic(currentMetallic);
					}

					float currentRoughness = renderer->GetRoughness();
					if (ImGui::SliderFloat("Roughness", &currentRoughness, 0.0f, 1.0f))
					{
						renderer->SetRoughness(currentRoughness);
					}

					float currentExposure = renderer->GetExposure();
					if (ImGui::DragFloat("Exposure", &currentExposure, 0.01f, 0.0f, 10.0f))
					{
						renderer->SetExposure(currentExposure);
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
				else if (light->GetType() == LightType::Directional)
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

		if (selectedObject->HasComponent<AnimationStateMachine>())
		{
			if (ImGui::CollapsingHeader("Animation State Machine", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto fsm = selectedObject->GetComponent<AnimationStateMachine>();
				auto animator = selectedObject->GetComponent<Animator>();
				bool enableRootMotion = animator->IsRootMotionEnabled();

				// 현재 상태 이름 표시
				std::string currentStateName = fsm->GetCurrentStateName();

				// 드롭다운 메뉴로 상태 변경 UI
				bool isLooping = animator->IsLooping();
				if (ImGui::BeginCombo("State", currentStateName.c_str()))
				{
					auto stateNames = fsm->GetStateNames();
					for (const auto& name : stateNames)
					{
						const bool isSelected = (name == currentStateName);
						if (ImGui::Selectable(name.c_str(), isSelected))
						{
							fsm->ChangeState(name, isLooping);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			
				ImGui::Separator();
				// 현재 애니메이션 재생 시간 표시 UI
				if (animator)
				{
					float absoluteTime = animator->GetCurrentTime();
					float duration = animator->GetDuration();
					if (duration == 0.0f) { duration = 1.0f; }

					// 슬라이더의 최대 범위를 duration(경계 값)이 아닌, 뼈가 꼬이지 않는 '안전한 마지막 값'으로 설정합니다.
					float safeDuration = duration - 0.01f;
					if (safeDuration < 0.0f) safeDuration = 0.0f;

					// 슬라이더가 표시하고 제어할 시간을 0.0 ~ safeDuration 사이로 한정
					float sliderTime = fmod(absoluteTime, duration);

					// 만약 sliderTime이 safeDuration을 초과하면(경계 값에 도달하면), 슬라이더의 손잡이가 최대치에 머무르도록 값을 고정(Clamp)합니다.
					if (sliderTime > safeDuration)
					{
						sliderTime = safeDuration;
					}

					// ImGui 슬라이더의 최대값을 'duration'이 아닌 'safeDuration'으로 설정
					if (ImGui::SliderFloat("Timeline", &sliderTime, 0.0f, safeDuration))
					{
						// 슬라이더를 움직이면, 그 값을 Animator의 현재 시간으로 설정
						animator->SetCurrentTime(sliderTime);

						// 루트 모션 위치로 즉시 이동
						if (animator->IsRootMotionEnabled())
						{
							animator->UpdateRootMotionTransformToTime(sliderTime);
						}
					}

					// 슬라이더를 잡고 있는지(스크러빙 중인지) Animator에게 알림
					animator->SetIsScrubbing(ImGui::IsItemActive());
				}
				ImGui::Separator(); // 구분선 추가

				// 재생 버튼
				if (ImGui::Button("Play"))
				{
					animator->Play();
				}
				ImGui::SameLine(); // 버튼들을 같은 줄에 배치

				// 일시정지 버튼
				if (ImGui::Button("Pause"))
				{
					animator->Pause();
				}
				ImGui::SameLine();

				// 정지 버튼
				if (ImGui::Button("Stop"))
				{
					animator->Stop();
				}

				if (ImGui::Checkbox("Loop Animation", &isLooping))
				{
					// 사용자가 체크박스를 클릭하여 isLooping 값이 변경되었다면,
					// 변경된 값을 Animator에 즉시 다시 적용
					animator->SetLooping(isLooping);
				}

				// 루트 모션 제어 UI 추가
				ImGui::Separator();
				RootMotionBakeOptions tempOptions = animator->GetBakeOptions();

				ImGui::Checkbox("Apply Root Motion", &enableRootMotion);
				animator->SetEnableRootMotion(enableRootMotion);

				// Apply Root Motion이 켜져 있을 때만 활성화
				ImGui::BeginDisabled(!animator->IsRootMotionEnabled());
				{
					std::string currentRootBone = animator->GetRootBoneName();
					if (currentRootBone.empty()) {
						currentRootBone = "None (Click to Select)";
					}

					if (ImGui::BeginCombo("Root Bone", currentRootBone.c_str()))
					{
						Animation* animation = animator->GetCurrentAnimation();
						if (animation)
						{
							std::function<void(const AssimpNodeData*)> generateBoneNodes;
							generateBoneNodes =
								[&](const AssimpNodeData* node)
							{
								// 각 뼈를 선택 가능한 항목으로 생성
								if (ImGui::Selectable(node->name.c_str(), selectedBoneName == node->name))
								{
									// 뼈를 클릭하면 Animator의 루트 뼈 이름을 변경
									animator->SetRootBoneName(node->name);
								}

								// 모든 자식에 대해 재귀 호출
								for (const auto& child : node->children)
								{
									generateBoneNodes(&child);
								}
							};

							// 루트 노드부터 시작하여 전체 뼈 목록을 드롭다운 리스트에 그림
							generateBoneNodes(&animation->GetRootNode());
						}

						ImGui::EndCombo();
					}

					ImGui::Text("Bake Into Pose:");
					ImGui::Checkbox("X", &tempOptions.bakePositionX); ImGui::SameLine();
					ImGui::Checkbox("Y", &tempOptions.bakePositionY); ImGui::SameLine();
					ImGui::Checkbox("Z", &tempOptions.bakePositionZ);
					ImGui::Checkbox("Rotation", &tempOptions.bakeRotation);
				}
				animator->SetBakeOptions(tempOptions);
				ImGui::EndDisabled();

				ImGui::Separator();
				float currentSpeed = animator->GetSpeed();
				// DragFloat 위젯을 생성하여 속도를 제어 (0.0 ~ 5.0 범위, 0.05 단위로 조절)
				if (ImGui::DragFloat("Playback Speed", &currentSpeed, 0.05f, 0.0f, 100.0f))
				{
					// 사용자가 값을 변경하면, Animator에 즉시 새로운 속도를 설정
					animator->SetSpeed(currentSpeed);
				}

				// 뼈 목록 UI
				ImGui::Separator(); // 구분선 추가
				Animation* animation = animator->GetCurrentAnimation();

				if (animation)
				{
					ImGui::Text("Bone Hierarchy");
					ImGui::BeginChild("BoneList", ImVec2(0, 150), true);

					const auto& bones = animation->GetBones();
					for (const auto& bone : bones)
					{
						const std::string& boneName = bone.GetBoneName();
						if (ImGui::Selectable(boneName.c_str(), selectedBoneName == boneName))
						{
							selectedBoneName = boneName;
						}
					}
					ImGui::EndChild();
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
	glDepthMask(GL_FALSE); 
	glDisable(GL_CULL_FACE);

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
	glDepthMask(GL_TRUE); 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

			// 기본 색상은 녹색으로 설정
			glm::vec4 boneColor = { 0.0f, 1.0f, 0.0f, 1.0f };
			// 만약 현재 그리는 뼈의 이름이 사용자가 선택한 뼈와 같다면,
			if (node->name == selectedBoneName)
			{
				// 색상을 빨간색으로 변경
				boneColor = { 1.0f, 0.0f, 0.0f, 1.0f };
			}

			debugShader->SetUniformVec4("color", boneColor);

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
