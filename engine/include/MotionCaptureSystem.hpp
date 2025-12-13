#pragma once

#include <vector>
#include <string>

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include "glm.hpp"
#include <glew.h> // OpenGL 텍스처 생성을 위해 필요

enum class InputMode {
    Webcam,
    VideoFile
};

struct PoseLandmark {
    glm::vec3 position; // x, y, z (엔진 월드 좌표)
    glm::vec2 rawUV;    // 2D 화면 좌표 (0.0 ~ 1.0, 시각화용)
    float visibility;   // 0.0 ~ 1.0 (신뢰도)
};

class MotionCaptureSystem {
public:
    MotionCaptureSystem();
    ~MotionCaptureSystem();

    // 초기화: 모델 파일 로드
    bool Init(const std::string& modelPath);

    // 소스 열기: 웹캠 또는 비디오 파일 선택
    bool OpenSource(InputMode mode, const std::string& filePath = "");

    // 업데이트: 매 프레임 실행 (영상 읽기 -> AI 추론 -> 좌표 변환)
    void Update();

    // 데이터 가져오기 (엔진용)
    const std::vector<PoseLandmark>& GetLandmarks() const { return landmarks; }

    const cv::Mat& GetCurrentFrame() const { return currentFrame; }
    bool IsInitialized() const { return isInitialized; }
    GLuint GetTextureID() const { return imageTexture; }
private:
    void PreprocessImage(const cv::Mat& src, std::vector<float>& inputTensor);
    void PostprocessOutput(float* rawOutput);
    std::string OpenFileDialog();

    void UpdateTexture();
    void DrawSkeleton(cv::Mat& img);
private:
    bool isInitialized = false;

    // OpenCV
    cv::VideoCapture capture;
    cv::Mat currentFrame;
    InputMode currentMode = InputMode::Webcam;

    // ONNX Runtime
    Ort::Env env;
    Ort::Session* session = nullptr;

    // 노드 이름 저장소
    std::vector<std::string> inputNodeNameAllocatedStrings;
    std::vector<std::string> outputNodeNameAllocatedStrings;

    // 모델 입출력 노드 이름 (BlazePose 기본값)
    std::vector<const char*> inputNodeNames;
    std::vector<const char*> outputNodeNames;

    // 결과 저장소 (33개 관절)
    std::vector<PoseLandmark> landmarks;

    // 모델 입력 해상도 (BlazePose는 보통 256x256)
    const int MODEL_WIDTH = 256;
    const int MODEL_HEIGHT = 256;
    GLuint imageTexture = 0;

    // 관절 연결 순서 (Bone Connections) 정의
    const std::vector<std::pair<int, int>> POSE_CONNECTIONS = {
        {0, 1}, {1, 2}, {2, 3}, {3, 7}, {0, 4}, {4, 5}, {5, 6}, {6, 8}, // 얼굴
        {9, 10}, {11, 12}, // 입, 어깨 연결
        {11, 13}, {13, 15}, // 왼쪽 팔
        {12, 14}, {14, 16}, // 오른쪽 팔
        {11, 23}, {12, 24}, {23, 24}, // 몸통
        {23, 25}, {25, 27}, {27, 29}, {29, 31}, {31, 27}, // 왼쪽 다리 & 발
        {24, 26}, {26, 28}, {28, 30}, {30, 32}, {32, 28}  // 오른쪽 다리 & 발
    };
};