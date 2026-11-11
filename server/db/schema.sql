-- 마음고치 DB 스키마 (PostgreSQL)
-- Node 서버가 켜질 때마다 자동으로 실행됨. `if not exists`라서 여러 번 실행해도 안전.

-- ─────────────────────────────────────────────
-- 웹 사용자 / 로그인 세션
-- ─────────────────────────────────────────────
create table if not exists users (
  id             bigint generated always as identity primary key,
  username       text not null,
  password_hash  text not null,                      -- scrypt 해시 (원문 저장 안 함)
  created_at     timestamptz not null default now()
);
create unique index if not exists users_username_key on users (lower(username));

create table if not exists sessions (
  token_hash  text primary key,                      -- 세션 토큰의 SHA-256
  user_id     bigint not null references users (id) on delete cascade,
  expires_at  timestamptz not null
);

-- ─────────────────────────────────────────────
-- 기기
-- ─────────────────────────────────────────────
create table if not exists devices (
  id                  text primary key check (id ~ '^[0-9A-F]{12}$'),  -- MAC 주소
  token_hash          text not null,                 -- 기기 토큰의 SHA-256 (원문 저장 안 함)
  pairing_code        text unique,
  pairing_expires_at  timestamptz,
  owner_id            bigint references users (id) on delete set null,
  created_at          timestamptz not null default now(),
  last_seen_at        timestamptz
);
create index if not exists devices_owner_idx on devices (owner_id);

-- ─────────────────────────────────────────────
-- 센서 측정 기록
-- ─────────────────────────────────────────────
create table if not exists readings (
  id           bigint generated always as identity primary key,
  device_id    text not null references devices (id) on delete cascade,
  measured_at  timestamptz not null,                 -- 기기에서 측정한 시각
  steps        integer check (steps >= 0),           -- 직전 기록 이후 걸음 수 (구간값)
  sun_seconds  integer check (sun_seconds >= 0),     -- 직전 기록 이후 햇빛 기준을 넘은 시간(초, 구간값). 기기가 계산
  uv_index     real    check (uv_index >= 0),
  lux          real    check (lux >= 0),
  received_at  timestamptz not null default now(),   -- 서버가 받은 시각
  unique (device_id, measured_at)                    -- 재전송해도 중복 저장 안 됨
);
-- 예전 버전으로 만든 DB에도 컬럼이 생기게 함
alter table readings add column if not exists sun_seconds integer check (sun_seconds >= 0);
create index if not exists readings_device_time_idx on readings (device_id, measured_at desc);

-- ─────────────────────────────────────────────
-- 미션 이벤트
-- ─────────────────────────────────────────────
create table if not exists mission_events (
  id           bigint generated always as identity primary key,
  device_id    text not null references devices (id) on delete cascade,
  occurred_at  timestamptz not null,
  mission      text not null check (mission ~ '^[a-z][a-z0-9_]{0,31}$'),
  status       text not null check (status in ('started', 'completed')),
  received_at  timestamptz not null default now(),
  unique (device_id, occurred_at, mission)
);
create index if not exists mission_events_device_time_idx on mission_events (device_id, occurred_at desc);
