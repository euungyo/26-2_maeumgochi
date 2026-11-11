# 마음고치 서버 (로컬: Node.js + PostgreSQL)

기기(ESP32)가 보낸 센서/미션 기록을 받아서 PostgreSQL에 저장하고, 웹 대시보드에서 유저별로 보여주는 서버야.
기기 ↔ 서버 데이터 형식은 [`docs/api.md`](../docs/api.md)에 있어.

```
[ESP32] ─HTTP→ [Node 서버 :3000] ─SQL→ [PostgreSQL (Docker) :5432]
[웹 브라우저] ─HTTP→ [Node 서버 :3000]
```

- **PostgreSQL**: Docker 컨테이너로 실행. 이 컴퓨터(127.0.0.1)에서만 접속 가능
- **Node 서버**: 기기 API + 웹 로그인 + 대시보드 페이지. 같은 WiFi의 기기가 접속 가능
- DB 테이블은 Node 서버가 켜질 때 `db/schema.sql`로 자동 생성돼 (여러 번 켜도 안전)

## 구조

```
server/
├─ docker-compose.yml    # PostgreSQL 컨테이너 설정
├─ .env.example          # 설정 예시 (복사해서 .env로 쓰면 값 변경 가능, 없어도 동작)
├─ package.json
├─ db/schema.sql         # 테이블 정의
├─ src/
│  ├─ server.js          # 시작점 (npm start)
│  ├─ app.js             # API 라우트
│  ├─ db.js              # DB 연결
│  ├─ validate.js        # 기기 데이터 형식 검사
│  └─ security.js        # 토큰/비밀번호 해시
├─ public/index.html     # 웹 대시보드
└─ test/smoke.test.js    # 전체 흐름 자동 점검 (npm test)
```

## 처음 실행 (Git Bash, `server` 폴더에서)

준비물: **Docker Desktop**(켜져 있어야 함), **Node.js 22 이상**

```bash
cd server
npm install              # 처음 한 번 (pg 라이브러리 설치)
docker compose up -d     # PostgreSQL 켜기 (처음엔 이미지 다운로드로 조금 걸림)
npm start                # 서버 켜기
```

`npm start` 하면 이런 게 나와:

```
마음고치 서버 실행 중 (DB 연결 OK)

  이 컴퓨터에서 대시보드:  http://localhost:3000
  ESP32 / 같은 WiFi 기기에서 쓸 API 주소:
    http://192.168.0.12:3000/api
```

- 브라우저로 `http://localhost:3000` → 회원가입 → 대시보드
- 펌웨어에는 아래쪽 `http://192.168.x.x:3000/api` 주소를 넣으면 돼
- **Windows 방화벽 창이 뜨면 "개인 네트워크" 허용** (안 하면 ESP32가 못 들어옴)

## 평소 사용

| 하고 싶은 것 | 명령어 |
|---|---|
| DB 켜기 | `docker compose up -d` |
| 서버 켜기 | `npm start` |
| 서버 끄기 | 서버 창에서 `Ctrl+C` |
| DB 끄기 (데이터는 유지) | `docker compose down` |
| **DB 완전 초기화 (데이터 삭제)** | `docker compose down -v` |
| 전체 흐름 자동 점검 | `npm test` (DB 켜진 상태에서) |
| DB 직접 들여다보기 | `docker exec -it maeumgochi-db psql -U maeumgochi` |

`docker compose up -d`로 켠 DB는 `restart: unless-stopped`라서 Docker Desktop이 켜지면 자동으로 같이 켜져.

## 기기 없이 테스트 (Git Bash)

```bash
BASE=http://localhost:3000/api

# 1) 기기 등록 → device_token, pairing_code가 나옴
curl -s -X POST "$BASE/register-device" \
  -H "Content-Type: application/json" \
  -d '{"device_id":"A1B2C3D4E5F6"}'

# 2) 받은 토큰으로 데이터 전송 (ts는 현재 시각 근처로 바꿔서)
TOKEN=<받은 device_token>
curl -s -X POST "$BASE/readings" \
  -H "Content-Type: application/json" \
  -H "x-device-token: $TOKEN" \
  -d '{"device_id":"A1B2C3D4E5F6",
       "readings":[{"ts":"2026-11-10T17:00:00+09:00","steps":120,"sun_seconds":240,"uv_index":3.2,"lux":850}],
       "missions":[{"ts":"2026-11-10T17:00:00+09:00","mission":"walk","status":"completed"}]}'
# → {"accepted_readings":1,"accepted_missions":1}
# 같은 요청을 한 번 더 보내면 0, 0 (중복 저장 안 됨)
```

3) 브라우저 대시보드에서 회원가입 → 1)에서 받은 **페어링 코드** 입력 → 그래프에 데이터가 보이면 성공

## 테이블

| 테이블 | 내용 |
|---|---|
| `users` | 웹 아이디, 비밀번호 해시 |
| `sessions` | 로그인 세션 (7일 유지) |
| `devices` | 기기 ID(MAC), 기기 토큰 해시, 페어링 코드, 주인 |
| `readings` | 측정 시각, 걸음 수·햇빛 쬔 시간(구간값, 기기가 계산), UV 지수, 조도 |
| `mission_events` | 미션 이름, 시작/완료, 시각 |

## 보안 메모

- 비밀번호, 기기 토큰, 세션 토큰은 **해시만 저장**해 (DB가 유출돼도 원문은 모름)
- PostgreSQL 포트는 `127.0.0.1`에만 열려 있어서 다른 기기에서 DB로 직접 못 들어와
- `.env`의 DB 비밀번호는 로컬 개발용 기본값이야. 실제 서비스로 쓸 거면 바꾸기
- HTTP(암호화 없음)라서 **같은 WiFi 안에서만** 쓰는 걸 전제로 해. 공공 WiFi에서 쓰지 말기

## 안 될 때

| 증상 | 확인할 것 |
|---|---|
| `[DB 연결 실패] PostgreSQL에 연결할 수 없음` | Docker Desktop 켜져 있는지, `docker compose up -d` 했는지 |
| `npm: command not found` | Node.js 설치 후 Git Bash 새로 열기 (안 되면 재부팅) |
| ESP32에서 접속이 안 됨 | 같은 WiFi인지, 방화벽 허용했는지, 주소가 `localhost`가 아니라 `192.168...`인지 |
| 학교 WiFi에서 ESP32가 안 붙음 | 학번 로그인 WiFi는 ESP32가 못 씀 → 노트북+기기를 **폰 핫스팟**에 연결 |
