# OpenGL 3D Game Engine & AI Motion Capture
> **도전학기 15주 프로젝트: 자체 제작 3D 그래픽스 엔진 및 AI 모션 캡처 시스템**

이 프로젝트는 C++와 OpenGL 4.3+을 기반으로 바닥부터 직접 구현한 **3D 그래픽스 엔진**입니다.
렌더링 파이프라인, PBR(물리 기반 렌더링), 스켈레탈 애니메이션 시스템을 포함하며, **ONNX Runtime과 OpenCV를 활용한 실시간 AI 모션 캡처(BlazePose)** 기능을 탑재하여 웹캠만으로 3D 캐릭터의 뼈대를 추적하고 시각화할 수 있습니다.

---

## 주요 기능 (Key Features)

### 1. Core Engine Architecture
- **Component-Based System**: Unity와 유사한 컴포넌트 기반 객체 관리 (`MeshRenderer`, `Animator`, `Light` 등).
- **Manager System**: 기능별 매니저 모듈화 (`Render`, `Object`, `Input`, `Scene`, `Camera`, `Thread`).
- **Scene Management**: 런타임 중 씬 전환 및 리소스 생명주기 관리.

### 2. Graphics & Rendering
- **PBR (Physically Based Rendering)**:
    - Albedo, Normal, Metallic, Roughness, AO 맵 지원.
    - **IBL (Image Based Lighting)**: HDR 환경맵을 활용한 Irradiance, Prefilter Map, BRDF LUT 적용.
- **Advanced Lighting**: Directional Light, Point Light 지원.
- **Skybox**: HDR 텍스처를 큐브맵으로 변환하여 배경 렌더링.

### 3. Animation System
- **Skeletal Animation**: Assimp를 이용한 뼈대 및 키프레임 데이터 추출.
- **GPU Skinning**: 셰이더 기반의 고성능 스키닝 처리.
- **FSM (Finite State Machine)**: 상태 기반 애니메이션 전환 및 **블렌딩(Blending)** 처리.
- **Root Motion**: 애니메이션의 이동 데이터를 실제 오브젝트 좌표에 반영.

### 4. AI Motion Capture
- **Real-time Skeleton Tracking**: Google **BlazePose** 모델 사용.
- **ONNX Runtime & OpenCV**: 웹캠 및 비디오 파일에서 33개 관절 랜드마크 실시간 추론.
- **Visualization**: 추론된 3D 좌표를 엔진 월드 좌표로 변환하여 실시간 시각화.

---

## 🛠️ 개발 환경 (Tech Stack)

* **Language**: C++ (Visual Studio 2022)
* **Graphics API**: OpenGL 4.3+
* **Libraries**:
    * SDL 3.0 (Window & Input)
    * GLEW, GLM (OpenGL Extensions & Math)
    * Assimp (Model Loading)
    * ImGui (UI/Debugging)
    * **OpenCV** (Video Processing)
    * **ONNX Runtime** (AI Inference)

---

## 설치 및 실행 방법 (Installation & Setup)

이 프로젝트는 **OpenCV**와 **ONNX Runtime** 라이브러리가 필요합니다. 프로젝트 설정(링커 등)은 이미 완료되어 있으므로, 아래 절차에 따라 라이브러리 파일만 준비해주시면 됩니다.

### 1. ONNX Runtime 설치 (NuGet)
Visual Studio에서 프로젝트를 열고 다음 단계를 수행하세요.
1.  **솔루션 탐색기** -> 프로젝트 우클릭 -> **NuGet 패키지 관리(Manage NuGet Packages)...** 선택.
2.  **`Microsoft.ML.OnnxRuntime`** 검색 후 설치.
3.  (선택 사항) GPU 가속이 필요한 경우 `Microsoft.ML.OnnxRuntime.Gpu`를 설치합니다 (기본 CPU 버전으로도 정상 작동함).

### 2. OpenCV 설정 및 DLL 배치
OpenCV는 용량 문제로 저장소에 포함되지 않았으므로 별도 다운로드가 필요합니다.
1.  [OpenCV 공식 릴리즈 페이지](https://opencv.org/releases/)에서 Windows 버전(4.x 이상)을 다운로드하여 설치합니다.
2.  **실행을 위해 필수적인 DLL 파일**들을 프로젝트의 실행 파일이 생성되는 폴더(`x64/Debug` 또는 `x64/Release`) 혹은 프로젝트 루트 폴더에 복사해야 합니다.
    * **필수 파일 1**: `opencv_world4xx.dll` (설치된 버전에 맞는 파일)
    * **필수 파일 2**: `opencv_videoio_ffmpeg4xx_64.dll` (비디오 파일 재생을 위한 코덱)
    * *위치 예시: `C:\opencv\build\x64\vc16\bin` 내부*

### 3. 빌드 및 실행
1.  Visual Studio 2022에서 `.sln` 파일을 엽니다.
2.  솔루션 구성이 **`x64`** 인지 확인합니다.
3.  **빌드(Build)** -> **솔루션 빌드**를 실행합니다.
4.  **F5**를 눌러 실행합니다.

> **Tip**: 실행 시 `asset` 폴더를 찾지 못하는 오류가 발생하면, `asset` 폴더가 실행 파일(`target .exe`)과 같은 위치에 있는지 확인하거나 Visual Studio의 **작업 디렉터리(Working Directory)** 설정을 확인해주세요.

---

##  데모 및 조작법 (Controls & Demos)

### Scene List (상단 ImGui 메뉴에서 선택 가능)
1.  **Meshes Demo**: 기본 도형 및 텍스처 테스트.
2.  **Animation Demo**:
    * Thriller 춤 애니메이션 시퀀스 및 펀치 동작 시연 (FSM & Root Motion).
3.  **PBR Demo**: 재질(Metallic/Roughness)에 따른 구체 렌더링 및 조명 테스트.
4.  **Game Demo (Coin Collection)**:
    * 3인칭 캐릭터 컨트롤 (가속/감속, Slerp 회전).
    * 코인을 모두 모으면 클리어되는 미니 게임.
5.  **MoCap Demo**:
    * 웹캠을 켜거나 비디오 파일을 로드하여 실시간으로 내 몸의 관절을 3D로 시각화.

   
### 조작법 (Keyboard & Mouse)
* **W, A, S, D**: 카메라 이동 / 캐릭터 이동 (Game 씬)
* **마우스 우클릭 (드래그)**: 시점 회전
* **Space / Shift**: 카메라 상승 / 하강 (Free Camera 모드)
* **F11**: 전체 화면 토글
* **ESC**: 종료
* **숫자키 3**: MSAA (안티에일리어싱) 토글

---

## 주차별 개발 요약 (15주차 완료)

* **1~4주차**: SDL3 윈도우 생성, 렌더링 파이프라인 구축, 카메라 시스템.
* **5~8주차**: Assimp 모델 로딩, 퐁 조명, 스켈레탈 애니메이션 데이터 파싱.
* **9~12주차**: GPU 스키닝, PBR(Cook-Torrance) 구현, IBL 환경맵 생성.
* **13~15주차**: AI 모션 캡처(BlazePose) 연동, 애니메이션 FSM 고도화, 최종 데모 씬 통합 및 최적화.

