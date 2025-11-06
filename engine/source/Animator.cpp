#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/matrix_decompose.hpp"

#include "Animator.hpp"
#include "Animation.hpp"
#include "Object.hpp"

#include <iostream> // 디버깅용

// 디버그용 (지워야함)
#include "Engine.hpp" 
#include "ObjectManager.hpp" 


Animator::Animator()
	: Component(ComponentTypes::ANIMATOR),
	currentTime(0.0f)
{
	finalBoneMatrices.reserve(256);
	for (int i = 0; i < 256; i++)
		finalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::Init() {}
void Animator::End() {}

void Animator::Update(float dt)
{
	if (!currentAnimation) return; 


	// 블렌딩 팩터 업데이트 (0에서 1로)
	if (blendFactor < 1.0f)
	{
		// 0으로 나누기 방지
		if (currentBlendDuration <= 0.0f) currentBlendDuration = 0.01f;

		blendFactor += dt / currentBlendDuration;
		if (blendFactor >= 1.0f)
		{
			blendFactor = 1.0f;
			previousAnimation = nullptr; // 블렌딩 종료, 이전 애니메이션 정리
		}
	}

	// 이전 애니메이션 시간 업데이트 (사라지는 중일 때)
	if (previousAnimation)
	{
		float prevDuration = previousAnimation->GetDuration();

		// 이전 애니메이션의 시간도 계속 업데이트합니다.
		previousTime += previousAnimation->GetTicksPerSecond() * dt;

		// 1b-1. 시간이 0보다 작아지는 것을 방지합니다.
		if (previousTime < 0.0f) {
			previousTime = 0.0f;
		}

		// 시간이 애니메이션의 총 길이를 초과하지 못하도록 '고정(Clamp)'합니다. 
		// duration(경계 값) 대신, duration - 0.01f (안전한 마지막 값)으로 고정하여
		// NaN 값 발생(뼈 꼬임 및 워프)을 원천적으로 차단합니다.
		if (previousTime > prevDuration)
		{
			previousTime = prevDuration - 0.01f;
		}
	}

	if (playbackState == PlaybackState::Playing && !isScrubbing)
	{
		currentTime += currentAnimation->GetTicksPerSecond() * dt * animationSpeed;
		float duration = currentAnimation->GetDuration();

		if (currentTime >= duration)
		{
			justLooped = true;
			if (isLooping) {
				currentTime = fmod(currentTime, duration);
			}
			else {
				currentTime = duration - 0.01f;
				playbackState = PlaybackState::Stopped;
			}
		}
	}

	if (enableRootMotion && playbackState == PlaybackState::Playing && !isScrubbing)
	{
		// '현재' 애니메이션의 목표 Transform 계산
		glm::mat4 currentTarget = CalculateAbsoluteRootMotion(currentAnimation, currentTime, rootMotionStartTransform);
		glm::mat4 finalTarget = currentTarget;

		// 블렌딩 중이라면 '이전' 애니메이션의 목표 Transform도 계산
		if (blendFactor < 1.0f && previousAnimation)
		{
			// 이제 previousTime은 항상 유효한(안전한) 값이므로 NaN이 발생하지 않습니다.
			glm::mat4 previousTarget = CalculateAbsoluteRootMotion(previousAnimation, previousTime, previousRootMotionStartTransform);

			// 두 Transform을 분해하여 위치(mix)와 회전(slerp)을 보간
			glm::vec3 prevPos, currPos, prevScale, currScale, prevSkew, currSkew;
			glm::quat prevRot, currRot;
			glm::vec4 prevPers, currPers;

			glm::decompose(previousTarget, prevScale, prevRot, prevPos, prevSkew, prevPers);
			glm::decompose(currentTarget, currScale, currRot, currPos, currSkew, currPers);

			glm::vec3 finalPos = glm::mix(prevPos, currPos, blendFactor);
			glm::quat finalRot = glm::slerp(prevRot, currRot, blendFactor);
			glm::vec3 finalScale = glm::mix(prevScale, currScale, blendFactor);

			// 보간된 값으로 최종 Transform 재조립
			finalTarget = glm::translate(glm::mat4(1.0f), finalPos)
				* glm::mat4_cast(finalRot)
				* glm::scale(glm::mat4(1.0f), finalScale);
		}

		// 최종 (또는 보간된) Transform을 오브젝트에 적용
		Object* owner = GetOwner();
		if (owner)
		{
			glm::vec3 pos, scale, skew;
			glm::quat rot;
			glm::vec4 pers;
			glm::decompose(finalTarget, scale, rot, pos, skew, pers);
			owner->transform.SetPosition(pos);
			owner->transform.SetRotation(glm::degrees(glm::eulerAngles(rot)));
		}
	}

	CalculateBoneTransform(&currentAnimation->GetRootNode(), GetOwner()->transform.GetModelMatrix());
}

void Animator::PlayAnimation(Animation* newAnimation, bool isLoop, float speed, float blendDuration)
{
	// 이미 재생 중인 애니메이션이면 무시
	if (currentAnimation == newAnimation) return;

	Object* owner = GetOwner(); // 소유주 오브젝트 미리 찾아두기

	// 블렌딩 시작 설정
	if (blendDuration > 0.0f && currentAnimation != nullptr)
	{
		// 현재 애니메이션을 '이전' 애니메이션으로 이동
		previousAnimation = currentAnimation;
		previousTime = currentTime;
		// 현재 루트 모션 시작점도 '이전' 시작점으로 백업
		if (enableRootMotion)
		{
			previousRootMotionStartTransform = rootMotionStartTransform;
		}

		// 블렌딩 팩터 초기화
		blendFactor = 0.0f;
		currentBlendDuration = blendDuration;
	}
	else
	{
		// 블렌딩 안 함 (즉시 교체)
		previousAnimation = nullptr;
		blendFactor = 1.0f; // 1.0 = 100% 새 애니메이션만 재생
	}

	// '현재' 애니메이션을 새 것으로 설정
	currentAnimation = newAnimation;
	currentTime = 0.0f;
	animationSpeed = speed;
	isLooping = isLoop;
	justLooped = false;
	playbackState = PlaybackState::Playing;

	// [중요] 새 애니메이션의 루트 모션 시작점을 '현재' 오브젝트 위치로 갱신
	if (enableRootMotion && owner)
	{
		rootMotionStartTransform = owner->transform.GetModelMatrix();
	}

	// 루트 뼈 자동 감지 로직 (이전 코드에서 복원)
	if (enableRootMotion && rootBoneName.empty() && newAnimation)
	{
		const std::vector<std::string> commonRootNames = { "Hips", "hips", "Root", "root" };
		const auto& allBones = newAnimation->GetBones();
		bool found = false;
		for (const auto& bone : allBones) {
			for (const auto& commonName : commonRootNames) {
				if (bone.GetBoneName() == commonName || bone.GetBoneName().find(commonName) != std::string::npos) {
					std::string baseName = bone.GetBoneName();
					size_t pos = baseName.find("_$");
					if (pos != std::string::npos) {
						baseName = baseName.substr(0, pos);
					}
					rootBoneName = baseName;
					std::cout << "[Info] Root bone auto-detected: " << rootBoneName << std::endl;
					found = true;
					break;
				}
			}
			if (found) break;
		}
		if (!found) {
			std::cerr << "[Warning] Could not auto-detect a root bone. Root motion will be disabled." << std::endl;
			enableRootMotion = false;
		}
	}
}

void Animator::Play()
{
	if (playbackState == PlaybackState::Stopped)
	{
		currentTime = 0.0f;
	}
	playbackState = PlaybackState::Playing;
}

void Animator::Pause()
{
	playbackState = PlaybackState::Paused;
}

void Animator::Stop()
{
	playbackState = PlaybackState::Stopped;
	currentTime = 0.0f;

	// ImGui의 'Stop' 버튼 클릭 시, 위치를 원점으로 리셋하는 기능은 유지
	if (enableRootMotion)
	{
		UpdateRootMotionTransformToTime(0.0f);
	}
}

const std::vector<glm::mat4>& Animator::GetFinalBoneMatrices() const
{
	return finalBoneMatrices;
}

float Animator::GetCurrentTime() const
{
	return currentTime;
}

float Animator::GetDuration() const
{
	if (currentAnimation)
	{
		return currentAnimation->GetDuration();
	}
	return 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* currBone = currentAnimation ? currentAnimation->FindBone(nodeName) : nullptr;
	Bone* prevBone = previousAnimation ? previousAnimation->FindBone(nodeName) : nullptr;

	glm::mat4 currLocalT = node->transformation;
	bool currFound = false;

	if (currBone) {
		currBone->Update(currentTime);
		currLocalT = currBone->GetLocalTransform();
		currFound = true;
	}

	if (prevBone && blendFactor < 1.0f)
	{
		prevBone->Update(previousTime);
		glm::mat4 prevLocalT = prevBone->GetLocalTransform();

		glm::vec3 prevPos, currPos, prevScale, currScale, prevSkew, currSkew;
		glm::quat prevRot, currRot;
		glm::vec4 prevPers, currPers;

		glm::decompose(prevLocalT, prevScale, prevRot, prevPos, prevSkew, prevPers);
		glm::decompose(currLocalT, currScale, currRot, currPos, currSkew, currPers);

		glm::vec3 finalPos = glm::mix(prevPos, currPos, blendFactor);
		glm::quat finalRot = glm::slerp(prevRot, currRot, blendFactor);
		glm::vec3 finalScale = glm::mix(prevScale, currScale, blendFactor);

		nodeTransform = glm::translate(glm::mat4(1.0f), finalPos)
			* glm::mat4_cast(finalRot)
			* glm::scale(glm::mat4(1.0f), finalScale);
	}
	else if (currFound)
	{
		nodeTransform = currLocalT;
	}


	if (enableRootMotion && !rootBoneName.empty())
	{
		// 두 가지 다른 접미사를 모두 확인합니다.
		std::string translationChannelName = rootBoneName + "_$AssimpFbx$_Translation";
		std::string rotationChannelName_Fbx = rootBoneName + "_$AssimpFbx$_Rotation";
		std::string rotationChannelName_Mocap = rootBoneName + "_$AssMocapFix$_Rotation";

		if (nodeName == translationChannelName) {
			nodeTransform[3][0] = 0.0f; nodeTransform[3][1] = 0.0f; nodeTransform[3][2] = 0.0f;
		}

		// Fbx 이름 또는 Mocap 이름 둘 중 하나라도 맞으면 회전 고정
		if (nodeName == rotationChannelName_Fbx || nodeName == rotationChannelName_Mocap) {
			nodeTransform = glm::mat4(1.0f);
		}
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;
	globalBoneTransforms[nodeName] = globalTransformation;

	const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap.at(nodeName).id;
		glm::mat4 offset = boneInfoMap.at(nodeName).offsetMatrix;
		finalBoneMatrices[index] = globalTransformation * offset;
	}

	for (const auto& child : node->children)
	{
		CalculateBoneTransform(&child, globalTransformation);
	}
}

void Animator::SetLooping(bool loop)
{
	isLooping = loop;
}

void Animator::SetCurrentTime(float time)
{
	currentTime = time;
	justLooped = false;
}

void Animator::UpdateRootMotionTransformToTime(float time)
{
	// 루트 모션이 꺼져있거나, 애니메이션 또는 루트 뼈 이름이 없으면 실행 안함
	if (!enableRootMotion || !currentAnimation || rootBoneName.empty()) return;

	Object* owner = GetOwner();
	if (owner)
	{
		// 헬퍼 함수를 호출하여 목표 Transform 계산
		glm::mat4 finalTransform = CalculateAbsoluteRootMotion(currentAnimation, time, rootMotionStartTransform);

		// 행렬에서 위치와 회전을 분해
		glm::vec3 scale, position, skew;
		glm::quat rotation;
		glm::vec4 perspective;
		glm::decompose(finalTransform, scale, rotation, position, skew, perspective);

		// 오브젝트의 Transform을 즉시 설정
		owner->transform.SetPosition(position);
		owner->transform.SetRotation(glm::degrees(glm::eulerAngles(rotation)));
	}
}

glm::mat4 Animator::CalculateAbsoluteRootMotion(Animation* anim, float time, glm::mat4 startTransform)
{
	if (!enableRootMotion || !anim || rootBoneName.empty()) return startTransform;

	// 두 가지 다른 접미사(Fbx, MocapFix)를 모두 확인합니다.
	std::string posBoneName_Fbx = rootBoneName + "_$AssimpFbx$_Translation";
	std::string rotBoneName_Fbx = rootBoneName + "_$AssimpFbx$_Rotation";
	std::string rotBoneName_Mocap = rootBoneName + "_$AssMocapFix$_Rotation";

	Bone* posBone = anim->FindBone(posBoneName_Fbx);
	Bone* rotBone = anim->FindBone(rotBoneName_Fbx);

	// Fbx 이름으로 회전 뼈를 못 찾았다면, Mocap 이름으로 다시 시도
	if (!rotBone)
	{
		rotBone = anim->FindBone(rotBoneName_Mocap);
	}
	// --- [수정 끝] ---

	if (posBone && rotBone)
	{
		glm::mat4 startBoneLocalT = glm::translate(glm::mat4(1.0f), posBone->GetInterpolatedPosition(0.0f)) * glm::mat4_cast(rotBone->GetInterpolatedRotation(0.0f));
		glm::mat4 targetBoneLocalT = glm::translate(glm::mat4(1.0f), posBone->GetInterpolatedPosition(time)) * glm::mat4_cast(rotBone->GetInterpolatedRotation(time));
		glm::mat4 totalDelta = glm::inverse(startBoneLocalT) * targetBoneLocalT;

		if (bakeOptions.bakePositionX) totalDelta[3][0] = 0.0f;
		if (bakeOptions.bakePositionY) totalDelta[3][1] = 0.0f;
		if (bakeOptions.bakePositionZ) totalDelta[3][2] = 0.0f;
		if (bakeOptions.bakeRotation)
		{
			totalDelta[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			totalDelta[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
			totalDelta[2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		}

		return startTransform * totalDelta;
	}

	// 뼈를 못찾으면(버그 방지) 시작 Transform을 그대로 반환
	return startTransform;
}
