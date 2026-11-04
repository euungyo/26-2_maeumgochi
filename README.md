# 마음고치

마음고치는 사용자의 작은 일상 활동을 캐릭터의 성장으로 연결하는 ESP32-S3 기반 휴대형 다마고치 프로젝트입니다.

현재는 하드웨어가 없는 상태에서도 게임 규칙과 화면 흐름을 개발하고 시험할 수 있도록 펌웨어의 핵심 로직과 실제 장치 코드를 분리합니다. 개발보드가 도착하면 시뮬레이터의 가짜 장치를 실제 센서 드라이버로 교체하는 방식으로 통합합니다.

## 저장소 구조

```text
26-2_maeumgochi/
├── README.md
├── .gitignore
├── firmware/
│   ├── README.md
│   ├── include/
│   │   ├── core/          # 게임 핵심 자료형과 공개 인터페이스
│   │   └── hal/           # 센서와 출력장치 추상 인터페이스
│   ├── src/
│   │   ├── app/           # 초기화와 전체 실행 흐름
│   │   ├── core/          # 캐릭터, 미션, 성장 규칙
│   │   ├── drivers/       # 실제 ESP32 및 센서 드라이버
│   │   └── ui/            # LCD 화면과 버튼 메뉴
│   ├── test/              # 하드웨어 없이 실행하는 단위 테스트
│   └── simulator/         # 가짜 센서 기반 PC 시뮬레이터
└── hardware/
    ├── README.md
    ├── schematics/        # 회로도
    ├── wiring/            # 핀맵과 배선 자료
    └── datasheets/        # 부품 데이터시트 또는 링크 문서
```

## 펌웨어 설계 원칙

`firmware/src/core`에는 Arduino, ESP32, GPIO 또는 특정 센서 라이브러리에 의존하는 코드를 넣지 않습니다. 캐릭터 상태, 미션 완료 조건, 경험치와 성장, 시간에 따른 상태 변화처럼 하드웨어와 관계없는 규칙만 구현합니다.

`firmware/include/hal`에는 걸음 수, 자외선, 시간, 위치, 버튼, 화면, 진동, 저장장치에 대한 인터페이스를 정의합니다. PC 시뮬레이터와 실제 하드웨어 드라이버가 같은 인터페이스를 구현하도록 구성합니다.

`firmware/src/drivers`에는 BMI270, LTR390, PCF8523, L76K, ST7789와 같은 실제 장치 코드를 작성합니다. 하드웨어가 도착하기 전에는 비어 있어도 됩니다.

## 작업 영역

| 담당 영역 | 주요 경로 | 작업 내용 |
| --- | --- | --- |
| 게임 로직 | `firmware/src/core`, `firmware/test` | 캐릭터 상태, 미션, 성장, 회복 규칙 |
| 펌웨어 통합 | `firmware/src/app`, `firmware/include/hal` | 실행 흐름, 장치 인터페이스, 저장 구조 |
| UI | `firmware/src/ui` | 240×240 LCD 화면, 메뉴, 버튼 입력 흐름 |
| 센서와 회로 | `firmware/src/drivers`, `hardware` | 센서 드라이버, 핀맵, 회로와 조립 시험 |
| 시뮬레이터 | `firmware/simulator` | 가짜 걸음 수, UV, 시간과 버튼 입력 |

## 초기 개발 목표

첫 번째 목표는 PC에서 다음 흐름을 실행하는 것입니다.

1. 사용자가 오늘의 기분과 에너지를 입력합니다.
2. 걷기 또는 햇빛 미션이 생성됩니다.
3. 가짜 센서값을 입력합니다.
4. 미션 완료 후 캐릭터 경험치와 상태가 변경됩니다.
5. 프로그램을 다시 실행해도 상태가 복원됩니다.

## 브랜치 이름

작업 하나마다 짧은 브랜치를 생성합니다.

```text
feat/pet-state
feat/mission-engine
feat/home-screen
feat/save-load
hw/bmi270-driver
hw/st7789-display
docs/pin-map
```

`main` 브랜치는 항상 빌드 가능한 상태로 유지하고, 기능은 Pull Request를 통해 병합합니다.

## 개발 환경

펌웨어는 VS Code + PlatformIO + Arduino 프레임워크로 개발합니다. 대상 보드는 Seeed Studio
XIAO ESP32-S3 Plus이며, 공통 보드·빌드·시리얼 설정은 `firmware/platformio.ini`와
`firmware/boards`에 저장합니다. 설치 방법과 빌드·업로드·센서 진단 명령은
[`firmware/README.md`](firmware/README.md)를 참고합니다.

팀원이 각자 Arduino IDE에 라이브러리를 설치하는 방식은 사용하지 않습니다. 프로젝트에 필요한
라이브러리는 실제 부품과 드라이버가 확정될 때 `platformio.ini`의 `lib_deps`에 버전과 함께
등록합니다. 핀 번호는 `src/drivers` 또는 하드웨어 설정에 모으고 게임 핵심 로직에는 넣지 않습니다.

