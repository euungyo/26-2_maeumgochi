# 펌웨어

이 디렉터리에는 ESP32-S3 펌웨어, 하드웨어 독립 게임 로직, 단위 테스트와 PC 시뮬레이터를 저장합니다.

## 의존 방향

```text
app -> core
app -> hal
drivers -> hal
ui -> core
simulator -> core + hal
```

`core`는 다른 계층에 의존하지 않습니다. `core`에서 Arduino 또는 센서 라이브러리를 직접 포함하지 않습니다.

## 디렉터리

- `include/core`: 핵심 자료형과 공개 헤더
- `include/hal`: 장치 추상 인터페이스
- `src/app`: 초기화 및 애플리케이션 실행 흐름
- `src/core`: 게임 규칙 구현
- `src/drivers`: ESP32 및 실제 장치 구현
- `src/ui`: 화면 렌더링과 메뉴 흐름
- `test`: 코어 로직 단위 테스트
- `simulator`: 가짜 장치를 사용하는 PC 실행 환경

## 개발 환경 준비

이 프로젝트는 Arduino IDE 대신 VS Code와 PlatformIO를 사용합니다. Arduino 프레임워크는
그대로 사용하므로 센서 라이브러리, 업로드, 시리얼 모니터 사용 방식은 동일합니다.

1. VS Code로 저장소 루트를 엽니다.
2. 추천 확장 알림에서 `PlatformIO IDE`와 `C/C++`을 설치합니다.
3. VS Code를 다시 시작합니다.
4. 터미널에서 `cd firmware`를 실행합니다.
5. 아래 명령으로 첫 빌드를 실행합니다. 최초 한 번은 도구를 내려받아 시간이 걸립니다.

```bash
pio run -e xiao_esp32s3_plus
```

VS Code 명령 팔레트의 `Tasks: Run Task`에서도 한국어로 등록된 작업을 실행할 수 있습니다.
PlatformIO 확장이 설치된 직후 일반 터미널에서 `pio`를 찾지 못하면 VS Code를 다시 시작하거나
PlatformIO의 `New Terminal` 명령으로 터미널을 여십시오.

## 자주 사용하는 명령

모든 명령은 `firmware` 디렉터리에서 실행합니다.

```bash
# 제품 펌웨어 빌드
pio run -e xiao_esp32s3_plus

# USB로 제품 펌웨어 업로드
pio run -e xiao_esp32s3_plus -t upload

# 시리얼 출력 확인 (종료: Ctrl+C)
pio device monitor -e xiao_esp32s3_plus

# PC에서 core 단위 테스트
pio test -e native
```

업로드 포트는 기본적으로 자동 감지합니다. 여러 보드가 연결된 경우에만 개인 환경에서
`--upload-port`를 사용하고, 개인별 포트 이름을 `platformio.ini`에 커밋하지 않습니다.

## I2C 센서 연결 확인

BMI270, LTR390, PCF8523처럼 I2C를 사용하는 장치를 연결한 뒤 다음을 실행합니다.

```bash
pio run -e i2c_scanner -t upload
pio device monitor -e i2c_scanner
```

시리얼 모니터에 발견된 장치 주소가 반복해서 출력됩니다. 배선을 바꿀 때는 먼저 보드 전원을
분리하십시오. Plus 보드의 기본 I2C 핀은 보드 정의에 따라 `SDA=GPIO5`, `SCL=GPIO6`입니다.

## 센서 라이브러리 관리 원칙

확정된 부품에 맞춰 다음 라이브러리 버전을 `platformio.ini`에 고정했습니다.

| 장치 | 라이브러리 | 버전 |
| --- | --- | --- |
| BMI270 | SparkFun BMI270 Arduino Library | 1.0.3 |
| LTR390 | Adafruit LTR390 Library | 1.1.2 |
| PCF8523 | Adafruit RTClib | 2.1.4 |
| L76K | TinyGPSPlus | 1.0.3 |
| ST7789 | Adafruit GFX / ST7735 and ST7789 | 1.12.6 / 1.11.0 |

Arduino IDE의 라이브러리 관리자에서 팀원이 각자 설치하거나, 사용자 홈의 라이브러리를 소스에서
직접 참조하지 않습니다. 센서 예제는 먼저 독립 진단 환경에서 확인한 뒤 `src/drivers`로 옮깁니다.

센서별 진단 환경은 다음 이름을 사용합니다.

```bash
pio run -e bmi270_check -t upload
pio run -e ltr390_check -t upload
pio run -e pcf8523_check -t upload
pio run -e l76k_check -t upload
```

각 명령 다음에는 같은 환경 이름으로 `pio device monitor -e 환경이름`을 실행합니다. 배선과 주소는
[`hardware/wiring/pin-map.md`](../hardware/wiring/pin-map.md)를 따릅니다. ST7789는 SPI 및 제어 핀을
확정한 뒤 별도 진단 환경을 추가합니다.

## 보드 설정

대상 보드는 Seeed Studio XIAO ESP32-S3 Plus입니다. PlatformIO 기본 XIAO ESP32-S3 정의는
8MB Flash 보드이므로, 이 저장소의 `boards/seeed_xiao_esp32s3_plus.json`에서 Plus의 16MB Flash,
8MB Octal PSRAM, USB ID와 추가 핀 변형을 명시합니다. 팀원은 별도 보드 패키지 설정을 만들지
않고 저장소의 설정을 그대로 사용합니다.

업로드 포트가 사라진 경우 보드의 BOOT 버튼을 누른 상태로 USB 데이터 케이블을 연결한 다음
다시 업로드하십시오.

