#include "MotionCaptureSystem.hpp"

#include <iostream>
#include <Windows.h> // 파일 탐색기

MotionCaptureSystem::MotionCaptureSystem()
    : env(ORT_LOGGING_LEVEL_WARNING, "BlazePose")
{
    // 33개 관절 데이터 공간 확보
    landmarks.resize(33);
}

MotionCaptureSystem::~MotionCaptureSystem() {
    if (session) {
        delete session;
        session = nullptr;
    }
    if (capture.isOpened()) {
        capture.release();
    }
    if (imageTexture != 0) { 
        glDeleteTextures(1, &imageTexture); 
    }
}

// 모델 로드
bool MotionCaptureSystem::Init(const std::string& modelPath) {
    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        std::wstring widestr = std::wstring(modelPath.begin(), modelPath.end());
        session = new Ort::Session(env, widestr.c_str(), sessionOptions);

        Ort::AllocatorWithDefaultOptions allocator;

        // 입력 노드 처리
        size_t numInputNodes = session->GetInputCount();
        inputNodeNames.clear();
        inputNodeNameAllocatedStrings.clear(); // std::string 저장소

        // 미리 공간을 예약하여 메모리 재할당(주소 변경) 방지
        inputNodeNameAllocatedStrings.reserve(numInputNodes);

        for (size_t i = 0; i < numInputNodes; i++) {
            auto input_name = session->GetInputNameAllocated(i, allocator);
            inputNodeNameAllocatedStrings.push_back(input_name.get());
        }

        // 모든 문자열이 저장된 후 포인터 추출
        for (const auto& name : inputNodeNameAllocatedStrings) {
            inputNodeNames.push_back(name.c_str());
        }

        // 출력 노드 처리
        size_t numOutputNodes = session->GetOutputCount();
        outputNodeNames.clear();
        outputNodeNameAllocatedStrings.clear();

        // 미리 공간 예약
        outputNodeNameAllocatedStrings.reserve(numOutputNodes);

        for (size_t i = 0; i < numOutputNodes; i++) {
            auto output_name = session->GetOutputNameAllocated(i, allocator);
            outputNodeNameAllocatedStrings.push_back(output_name.get()); 
        }

        // 모든 문자열이 저장된 후 포인터 추출
        for (const auto& name : outputNodeNameAllocatedStrings) {
            outputNodeNames.push_back(name.c_str());
        }

        // 디버그 출력
        std::cout << "Input Name: " << inputNodeNames[0] << std::endl;
        std::cout << "Output Name: " << outputNodeNames[0] << std::endl;

        isInitialized = true;
        std::cout << "Model Loaded Successfully." << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Init Error: " << e.what() << std::endl;
        return false;
    }
}

// 윈도우 파일 열기 창
std::string MotionCaptureSystem::OpenFileDialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Video Files\0*.mp4;*.avi;*.mkv\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return ""; // 취소됨
}

void MotionCaptureSystem::UpdateTexture()
{
    if (currentFrame.empty()) return;

    // 텍스처 ID 생성 (기존과 동일)
    if (imageTexture == 0) {
        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // 시각화용 이미지 복사 (원본 훼손 방지)
    cv::Mat displayFrame = currentFrame.clone();

    // 뼈대와 관절 그리기
    DrawSkeleton(displayFrame);

    // 색상 변환 (BGR -> RGB)
    cv::Mat rgbFrame;
    cv::cvtColor(displayFrame, rgbFrame, cv::COLOR_BGR2RGB);

    // GPU 업로드
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbFrame.cols, rgbFrame.rows,
        0, GL_RGB, GL_UNSIGNED_BYTE, rgbFrame.data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MotionCaptureSystem::DrawSkeleton(cv::Mat& img)
{
    int w = img.cols;
    int h = img.rows;

    // 뼈(Line) 그리기
    for (const auto& pair : POSE_CONNECTIONS) {
        int idx1 = pair.first;
        int idx2 = pair.second;

        // 두 관절이 모두 화면 안에 있고 신뢰도가 높을 때만 그림
        if (landmarks[idx1].visibility > 0.5f && landmarks[idx2].visibility > 0.5f) {
            cv::Point p1(landmarks[idx1].rawUV.x * w, landmarks[idx1].rawUV.y * h);
            cv::Point p2(landmarks[idx2].rawUV.x * w, landmarks[idx2].rawUV.y * h);

            // 녹색 선 (두께 2)
            cv::line(img, p1, p2, cv::Scalar(0, 255, 0), 2);
        }
    }

    // 관절(Circle) 그리기
    for (int i = 0; i < 33; i++) {
        if (landmarks[i].visibility > 0.5f) {
            int cx = (int)(landmarks[i].rawUV.x * w);
            int cy = (int)(landmarks[i].rawUV.y * h);

            // 빨간 원 (반지름 4, 채움)
            cv::circle(img, cv::Point(cx, cy), 4, cv::Scalar(0, 0, 255), -1);
        }
    }
}

bool MotionCaptureSystem::OpenSource(InputMode mode, const std::string& filePath) {
    if (capture.isOpened()) capture.release();
    currentMode = mode;

    std::string path = filePath;

    if (mode == InputMode::VideoFile) {
        // 경로가 비어있으면 파일 탐색기 실행
        if (path.empty()) {
            std::cout << "Opening File Dialog..." << std::endl;
            path = OpenFileDialog();
            if (path.empty()) return false; // 사용자가 취소함
        }
        capture.open(path);
    }
    else {
        // 웹캠 (0번 인덱스)
        capture.open(0);
    }

    if (!capture.isOpened()) {
        std::cerr << "Failed to open source!" << std::endl;
        return false;
    }

    std::cout << "[MoCap] Source Opened: " << (mode == InputMode::Webcam ? "Webcam" : path) << std::endl;
    return true;
}

// 프레임 읽기 -> 추론
void MotionCaptureSystem::Update() {
    if (!isInitialized || !capture.isOpened()) return;

    cv::Mat frame;
    capture >> frame;

    if (frame.empty()) {
        // 비디오가 끝나면 반복 재생 (Loop)
        if (currentMode == InputMode::VideoFile) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
        }
        return;
    }

    // 웹캠은 거울 모드로 좌우 반전
    if (currentMode == InputMode::Webcam) {
        cv::flip(frame, currentFrame, 1);
    }
    else {
        currentFrame = frame.clone();
    }

    // AI 추론 시작
    std::vector<float> inputTensorValues;
    PreprocessImage(currentFrame, inputTensorValues);

    // 입력 텐서 생성 (1, 256, 256, 3) - NHWC 포맷 가정
    std::vector<int64_t> inputShape = { 1, MODEL_HEIGHT, MODEL_WIDTH, 3 };

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size());

    // 실행
    auto outputTensors = session->Run(
        Ort::RunOptions{ nullptr },
        inputNodeNames.data(), &inputTensor, 1,
        outputNodeNames.data(), 1
    );

    // 결과 변환
    float* floatArr = outputTensors[0].GetTensorMutableData<float>();
    PostprocessOutput(floatArr);

    UpdateTexture();
}

// 이미지 리사이즈 및 정규화
void MotionCaptureSystem::PreprocessImage(const cv::Mat& src, std::vector<float>& output) {
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(MODEL_WIDTH, MODEL_HEIGHT));
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB); // BGR -> RGB

    output.resize(MODEL_WIDTH * MODEL_HEIGHT * 3);

    // 정규화 (0~255 -> 0.0~1.0) 및 평탄화
    int idx = 0;
    for (int y = 0; y < MODEL_HEIGHT; y++) {
        for (int x = 0; x < MODEL_WIDTH; x++) {
            cv::Vec3b pixel = resized.at<cv::Vec3b>(y, x);
            output[idx++] = pixel[0] / 255.0f;
            output[idx++] = pixel[1] / 255.0f;
            output[idx++] = pixel[2] / 255.0f;
        }
    }
}

// 좌표 변환
void MotionCaptureSystem::PostprocessOutput(float* rawOutput) {
    float scale = 0.02f;

    for (int i = 0; i < 33; i++) {
        float x = rawOutput[i * 5 + 0];
        float y = rawOutput[i * 5 + 1];
        float z = rawOutput[i * 5 + 2];
        float vis = rawOutput[i * 5 + 3];

        // 3D 월드 좌표
        landmarks[i].position = glm::vec3(
            (x - 128.0f) * scale,
            -(y - 128.0f) * scale,
            z * scale
        );
        landmarks[i].visibility = vis;

        // 2D 화면 좌표 (0.0 ~ 1.0 정규화)
        // 모델 출력(x, y)은 0~256 범위이므로 256으로 나눔
        landmarks[i].rawUV = glm::vec2(
            x / (float)MODEL_WIDTH,
            y / (float)MODEL_HEIGHT
        );
    }
}