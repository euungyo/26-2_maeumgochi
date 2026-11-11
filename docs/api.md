# 마음고치 기기 ↔ 서버 API

기기(XIAO ESP32S3)와 서버(로컬 Node.js + PostgreSQL)가 주고받는 데이터 형식이야.
**이 문서가 펌웨어 담당과 서버/웹 담당 사이의 약속**이니까, 형식을 바꿀 때는 이 문서부터 고치고 펌웨어와 서버를 같은 PR에서 같이 수정해.

- 기본 주소: `http://<서버 노트북 IP>:3000/api` (서버를 켜면 터미널에 주소가 출력돼. 예: `http://192.168.0.12:3000/api`)
- **기기와 서버 노트북은 같은 WiFi**에 있어야 해. 기기에서 `localhost`는 기기 자신이라 쓰면 안 돼
- 모든 요청/응답은 `Content-Type: application/json`
- 시간은 전부 **타임존이 붙은 ISO 8601** 문자열 (`2026-11-09T19:00:00+09:00` 또는 `...Z`)

---

## 1. 기기 등록 — `POST /register-device`

기기를 처음 켰을 때, 또는 아직 주인이 없는 기기가 페어링 코드를 새로 받을 때 호출해.

### 요청

```json
{ "device_id": "A1B2C3D4E5F6" }
```

| 필드 | 타입 | 설명 |
|---|---|---|
| `device_id` | string | MAC 주소 12자리, **대문자 16진수, 콜론 없이** |

### 응답 `200`

```json
{
  "device_token": "3f9a...(64자리 hex)",
  "pairing_code": "K7M2QX",
  "expires_at": "2026-11-09T10:10:00.000Z"
}
```

- `device_token`: 기기 플래시(Preferences)에 저장해. **서버는 다시 알려주지 않아** (해시만 보관).
- `pairing_code`: 기기 화면에 표시. 유저가 웹에서 입력하면 그 유저 계정에 기기가 연결돼. **10분 유효.**
- 주인이 없는 상태에서 다시 호출하면 토큰과 코드가 새로 발급되고, 이전 토큰은 무효가 돼.

### 에러

| 코드 | `error` | 의미 |
|---|---|---|
| 400 | `invalid_json`, `invalid_device_id` | 형식 오류 |
| 409 | `device_already_claimed` | 이미 유저에게 연결된 기기. 기기에 저장된 토큰을 그대로 쓰면 됨 |

---

## 2. 측정 데이터 전송 — `POST /readings`

기기에 모아둔 기록을 몇 분마다 묶어서 보내.

### 헤더

```
x-device-token: <register-device에서 받은 device_token>
```

### 요청

```json
{
  "device_id": "A1B2C3D4E5F6",
  "readings": [
    { "ts": "2026-11-09T19:00:00+09:00", "steps": 120, "sun_seconds": 240, "uv_index": 3.2, "lux": 850.0 },
    { "ts": "2026-11-09T19:05:00+09:00", "steps": 340, "sun_seconds": 300, "uv_index": 2.8, "lux": 790.5 }
  ],
  "missions": [
    { "ts": "2026-11-09T19:05:00+09:00", "mission": "walk", "status": "completed" }
  ]
}
```

`readings[]`

| 필드 | 타입 | 필수 | 설명 |
|---|---|---|---|
| `ts` | string | O | **측정한 시각** (RTC 기준). 서버 시각보다 10분 넘게 미래면 거부 |
| `steps` | int ≥ 0 | X | **직전 기록 이후 걸은 걸음 수 (구간값, 누적 아님)** |
| `sun_seconds` | int ≥ 0 | X | **직전 기록 이후 햇빛 기준을 넘은 시간(초, 구간값)**. 기기가 센서를 자주 읽으면서 직접 누적 |
| `uv_index` | number ≥ 0 | X | LTR390 UV 지수 (기록 시점 값, 참고용) |
| `lux` | number ≥ 0 | X | LTR390 조도 (기록 시점 값, 참고용) |

`missions[]` (생략 가능)

| 필드 | 타입 | 설명 |
|---|---|---|
| `ts` | string | 이벤트 시각 |
| `mission` | string | 미션 이름. 소문자/숫자/`_`, 최대 32자 (예: `walk`, `sunlight`) — **목록은 팀에서 확정 필요** |
| `status` | string | `started` 또는 `completed` |

- `readings`, `missions` 각각 **한 요청에 최대 500개**
- 값이 없는 센서 필드는 빼거나 `null`로 보내면 돼

**역할 분담: 판단은 기기가, 서버는 저장하고 합치기만**

- `steps`, `sun_seconds`는 기기가 계산한 **구간값**이야. 서버와 대시보드는 날짜별로 더하기만 해
- "햇빛으로 인정하는 기준"(예: lux 몇 이상, 또는 UV 지수 몇 이상)은 **기기 펌웨어 설정**이야. 서버는 기준을 모르고 결과(초)만 받아. **기준값은 팀에서 실측으로 확정 필요**
- `sun_seconds`는 그 구간 길이를 넘을 수 없어 (5분 간격 기록이면 최대 300)

### 응답 `200`

```json
{ "accepted_readings": 2, "accepted_missions": 1 }
```

**같은 기기 + 같은 `ts` 기록은 중복 저장되지 않아.** 그래서 응답을 못 받아서 같은 데이터를 다시 보내도 안전해 (재전송 가능).

### 에러

| 코드 | `error` | 의미 |
|---|---|---|
| 400 | `invalid_json`, `invalid_device_id`, `invalid_readings`, `invalid_missions` | 형식 오류. `detail`에 몇 번째 항목이 틀렸는지 나옴 |
| 401 | `unauthorized` | 토큰 없음 / 틀림 / 등록 안 된 기기 → `register-device`부터 다시 |
| 413 | `too_many_items` | 500개 초과. 나눠서 보내기 |

### 기기 쪽 동작 규칙

1. 측정값은 기기 메모리/플래시에 쌓아둔다
2. N분마다 WiFi 연결 → `POST /readings`
3. **200을 받았을 때만** 보낸 기록을 지운다
4. 실패하면 지우지 말고 다음 전송 때 같이 보낸다
5. 401이면 저장된 토큰을 지우고 `register-device`부터 다시

---

## 3. 웹 API (대시보드용)

웹 대시보드(`http://<서버>:3000`)가 쓰는 API야. 로그인하면 `mg_session` 쿠키가 발급되고, 이후 요청은 그 쿠키로 인증해.
**로그인한 유저는 자기 기기 데이터만** 받아 와 (서버가 `owner_id`로 걸러줌).

| 메서드 · 경로 | 요청 | 응답 |
|---|---|---|
| `POST /api/signup` | `{ "username", "password" }` | `{ "username" }` + 쿠키 |
| `POST /api/login` | `{ "username", "password" }` | `{ "username" }` + 쿠키 |
| `POST /api/logout` | `{}` | `{ "ok": true }` |
| `GET /api/me` | | `{ "username", "devices": [{ "id", "created_at", "last_seen_at" }] }` |
| `POST /api/claim` | `{ "code": "K7M2QX" }` | `{ "device_id" }` — 페어링 코드로 기기를 내 계정에 연결 |
| `GET /api/readings?days=7` | | `{ "readings": [{ "device_id", "measured_at", "steps", "sun_seconds", "uv_index", "lux", "received_at" }] }` |
| `GET /api/missions?days=7` | | `{ "missions": [{ "device_id", "occurred_at", "mission", "status" }] }` |

- 아이디: 영문/숫자/`_` 3~20자, 비밀번호: 6자 이상
- `days`는 1~90 (기본 7)
- 로그인 안 하면 `401 login_required`
- 페어링 코드가 틀렸거나 만료/사용됨 → `400 invalid_or_expired_code`
