# 마음고치 UI 전달 문서 (아인)

> 2026-11-11 프로젝트 반영 메모: 전달받은 원본을 저장소 구조에 맞게 나눠 배치했습니다.
> 화면 코드는 `firmware/include/ui/art/`, PC 미리보기 도구는
> `firmware/tools/ui_preview/`, 글꼴 생성 도구는 `firmware/tools/ui_font/`에 있습니다.
> 이 브랜치는 UI 전달 파일 정리만 포함하며 앱·내비게이션 연결은 포함하지 않습니다.

## PC에서 미리보기 확인

저장소의 `firmware/` 디렉터리에서 실행합니다.

```bash
g++ -std=c++11 -Wall -Wextra -Werror \
  -I tools/ui_preview/host_mock -I include \
  tools/ui_preview/render_all.cpp \
  -o /tmp/maeumgochi-ui-render

mkdir -p /tmp/maeumgochi-ui-preview
cd /tmp/maeumgochi-ui-preview
/tmp/maeumgochi-ui-render
```

총 37개 BMP가 생성됩니다.

회의에 참석하지 못해서 UI 파트를 한 곳에 정리했습니다. **결정이 필요한 것**(2번)만 먼저 봐주셔도 됩니다.

## 1. 지금까지 만든 것
- 화면 19장 (3버튼 표 10장 + 성장·식사·수면·기록·배터리·저장됨)
- 미션 장면 13개 (침대 정리, 물 마시기, 세수·양치, 스트레칭, 환기, 산책, 샤워, 방 청소, 장보기, 요리, 공부, 낚시, 식물 물주기)
- 낚시 연출 6단계 (기다림, 입질, 물었다, 끌어올림, 잡았어요, 놓쳤어요)
- 캐릭터 구름 3단계 × 표정 3종 (졸림, 보통, 신남), 작은 팔·발 포함
- 모두 **한국어**, 240×240, 이미지 파일 없이 **그리기 코드만**으로 구현

`preview/` 폴더의 그림으로 먼저 확인할 수 있습니다.

## 2. 회의에서 정하기
| # | 정할 것 | 아인 의견 | 참고 |
|---|---|---|---|
| 1 | 화면 색: 밝은 하늘색(Soft day) / 어두운 남색 | 밝은 쪽이 덜 딱딱해 보여서 추천. 수면 화면만 어둡게 | `preview/theme_compare.png` |
| 2 | 캐릭터에 작은 팔·발 포함 여부 | 회의에서 정하기 (낚시, 스트레칭 같은 동작을 보여주려면 팔이 필요하다는 점만 참고) | `preview/character_3stages.png` |
| 3 | 한글 표시 방식 | 도트 글꼴 데이터(`ko_font.h`, 약 6KB)로 가능 | 아래 6번 |
| 4 | 상점: 캐릭터 꾸미기 / 방 꾸미기 | 둘 중 하나로 줄이는 게 현실적 (지금 화면은 모자·리본·스탠드 예시) | 아이템 수에 따라 디자인 작업량이 달라짐 |
| 5 | 낚시 성공 구간(타이밍 막대 폭)과 속도 | `ZONE_START 0.40`, `ZONE_END 0.60`은 임시값 | `code/fishing_demo.h` |
| 6 | 식사 시간·수면 시간 범위 | 화면의 시간(07:30, 13:00, 19:00)은 예시값 | 미션 엔진 규칙과 맞춰야 함 |

## 3. 폴더 구성
| 경로 | 내용 |
|---|---|
| `firmware/include/ui/art/cloud_sprite.h` | 구름 캐릭터 그리기 (`maeum::drawCloud`), 색 테마 선택, 모자 |
| `firmware/include/ui/art/scenes_demo.h` | 공통 색, 버튼 안내 줄, 메뉴, 미션 장면 13개 |
| `firmware/include/ui/art/screens_demo.h` | 화면 19장 |
| `firmware/include/ui/art/fishing_demo.h` | 낚시 연출 6단계 |
| `firmware/include/ui/art/ko_text.h`, `ko_font.h` | 한글 글자 그리기, 글자 데이터(자동 생성) |
| `firmware/tools/ui_font/make_korean_font.py` | 글자 데이터 만드는 도구 |
| `docs/ui/preview/` | 미리보기 그림 |
| `firmware/tools/ui_preview/` | PC에서 보드 없이 화면을 그려보는 도구 (가짜 화면 `host_mock`, `render_all.cpp`) |

## 4. 화면 함수 (모두 `Adafruit_GFX` 계열 화면 객체를 받음)
| # | 함수 | 화면 | 버튼 안내 (왼쪽 / 가운데 / 오른쪽) |
|---|---|---|---|
| 1 | `maeum_screen::mainScreen` | 메인 | 생활 / 미션 / 캐릭터 |
| 2 | `lifeMenuScreen` | 생활 메뉴 | 이전 / 선택 / 다음 |
| 3 | `charMenuScreen` | 캐릭터 메뉴 | 이전 / 선택 / 다음 |
| 4 | `missionListScreen` | 미션 목록 | 이전 / 상세 / 다음 |
| 5 | `missionDetailScreen` | 미션 상세 | 이전 / 시작 / 다음 |
| 6 | `checkinScreen` | 체크인 | - / 확정 / + |
| 7 | `confirmScreen` | 완료 확인창 | 취소 / 확정 / 완료 |
| 8 | `shopListScreen` | 상점 목록 | 이전 / 상세 / 다음 |
| 9 | `itemDetailScreen` | 상품 상세 | 이전 / 구매 / 다음 |
| 10 | `bagScreen` | 가방 | 이전 / 해제 / 다음 |
| 11 | `growthScreen` | 성장 | 확인 |
| 12 | `growthUpScreen` | 성장 연출 | 확인 |
| 13~15 | `mealBreakfast`, `mealLunch`, `mealDinner` | 식사 시간 | 나중에 / 먹었어요 |
| 16 | `sleepScreen` | 수면 (항상 어둡게) | 없음 |
| 17 | `recordScreen` | 기록 | 이전 주 / 다음 주 |
| 18 | `lowBatteryScreen` | 배터리 부족 | 확인 |
| 19 | `savedScreen` / `savedToast` | 저장됨 알림 (어느 화면 위에도 덧그릴 수 있음) | - |

- 미션 장면(`maeum_scene::makeBedScene` 등)과 낚시(`maeum_fish::fishScene(tft, 단계, t)`)는 해당 헤더를 보세요.
- 가운데 버튼 길게 누르기(뒤로)는 모든 화면에서 같아서 화면에는 따로 적지 않았습니다.

## 5. 쓰는 법
```cpp
#include <Adafruit_ST7789.h>
#include "ui/art/screens_demo.h"

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);   // 핀은 하드웨어 담당 확정값 사용

void setup() {
  tft.init(240, 240);
  maeum_screen::mainScreen(tft);
}
```
- 캐릭터만 그리기: `maeum::drawCloud(tft, 중심x, 중심y, 폭, 단계 1~3, maeum::MOOD_NORMAL);`
- 팔 자세: 맨 뒤에 `maeum::ARM_UP`, `ARM_OUT`, `ARM_NONE` 등 (왼팔, 오른팔)
- 색 테마: `MAEUM_THEME`을 1(밝음, 기본) 또는 0(어두움)으로 정의
- **위 예시는 실제 보드에서 확인하지 못한 것입니다.** 핀 번호와 화면 초기화 방식은 팀 확정값을 따라주세요.

### PlatformIO(팀 개발 환경)에서 쓰기
- `platformio.ini`(`[env:xiao_esp32s3]`, `framework = arduino`) 기준으로 필요한 라이브러리:
```ini
lib_deps =
    adafruit/Adafruit GFX Library@1.12.6
    adafruit/Adafruit ST7735 and ST7789 Library@1.11.0
```
- UI 아트 헤더는 `firmware/include/ui/art/`에 함께 배치되어 있습니다. `#include "ui/art/screens_demo.h"`로 불러옵니다.
- 글자 데이터(`ko_font.h`, 약 6KB)는 include하는 `.cpp`마다 복사되니, **한 파일에서만** 불러오는 것을 권장합니다.
- 그리기 함수는 `Adafruit_GFX&`만 받습니다. 하드웨어 계층(`display.h`)에서 화면 객체나 `GFXcanvas16`을 넘겨주면 됩니다.
- **화면에 바로 그리면 한 화면을 다시 그리는 데 대략 55~190ms**가 걸릴 것으로 추정됩니다. (컴퓨터에서 그리기 호출 수를 센 뒤 SPI 속도를 가정해 계산한 값이고, 보드에서 잰 것이 아닙니다.) 낚시·산책처럼 움직이는 장면은 메모리에 먼저 그리고 한 번에 전송하는 방식을 권장합니다.
```cpp
GFXcanvas16 canvas(240, 240);                       // 약 115KB 메모리
maeum_fish::fishScene(canvas, phase, t);
tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 240);
```
- PC 시뮬레이터·테스트용: 압축을 푼 폴더에서 아래 명령을 실행하면 모든 화면이 BMP 파일로 저장됩니다.
위의 "PC에서 미리보기 확인" 명령을 사용합니다.

## 6. 한글 글자 데이터
- 기본 글자에는 한글이 없어서, GNU Unifont(16픽셀 도트 글꼴)에서 **화면에 쓰는 글자만** 뽑아 `ko_font.h`를 만듭니다.
- 새 문구를 추가했다면 `firmware/include/ui/art/`에서 다음을 실행합니다:
  `python3 ../../../tools/ui_font/make_korean_font.py scenes_demo.h screens_demo.h fishing_demo.h ko_font.h`
- 글꼴 경로(`FONT = ...`)는 컴퓨터에 맞게 수정해야 합니다. **글꼴 라이선스는 확인이 필요합니다.**

## 7. 확인한 것과 못 한 것
- 컴퓨터에서 `Adafruit_GFX`와 같은 이름의 함수를 가진 가짜 화면으로 **컴파일하고 그려서 확인**했습니다. 두 테마 모두 컴파일됩니다.
- C++ 표준 `gnu++11`, `gnu++14`, `gnu++17`에서 경고 없이 컴파일되고, 여러 `.cpp`에서 include해도 링크됩니다. (ESP32용 `-Os` 옵션도 확인)
- **PlatformIO와 실제 Adafruit 라이브러리로 빌드해본 것은 아닙니다.**
- **실제 보드·ST7789 화면에서는 아직 돌려보지 않았습니다.** 색, 글자 크기, 속도는 보드에서 확인이 필요합니다.
- 센서, 미션 판정, 저장 로직은 포함하지 않았습니다 (화면 그리기만).

## 8. 임의로 정한 것 (확인 부탁드립니다)
- 한국어 문구와 말투("~요"), 기분 이름("좋아요")
- 미션 목록의 "아침/오후" 표시, 미션 상세의 "자동 확인", 보상 "경험치 +10 코인 +5"
- 상점 아이템 3개와 가격, 모자 색상 "주황"
- 성장 규칙을 "미션 2개 = 다음 단계, 총 3단계"로 가정
- 기록 화면의 숫자는 예시값 (실제로는 저장 데이터에서 가져와야 함)
