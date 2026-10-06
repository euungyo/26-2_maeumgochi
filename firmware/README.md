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

