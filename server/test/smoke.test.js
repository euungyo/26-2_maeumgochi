// 서버 전체 흐름 점검 (기기 등록 → 전송 → 회원가입 → 기기 연결 → 조회)
// 실행: DB를 켠 상태에서 `npm test`
// 주의: 테스트용 기기/유저를 실제 DB에 만들고 끝나면 지움
import assert from "node:assert/strict";
import { createServer } from "node:http";
import { createApp } from "../src/app.js";
import { connectDb, DEFAULT_DATABASE_URL } from "../src/db.js";

try {
  process.loadEnvFile(new URL("../.env", import.meta.url));
} catch {}

const db = await connectDb(process.env.DATABASE_URL || DEFAULT_DATABASE_URL);
const server = createServer(createApp({ db, publicDir: new URL("../public/", import.meta.url) }));
await new Promise((r) => server.listen(0, "127.0.0.1", r));
const BASE = `http://127.0.0.1:${server.address().port}`;

const DEVICE = "0000000000AA";
const DEVICE2 = "0000000000BB";
const suffix = Date.now().toString(36);
const USER_A = `test_a_${suffix}`;
const USER_B = `test_b_${suffix}`;

async function call(method, path, { body, headers = {}, cookie } = {}) {
  const res = await fetch(BASE + path, {
    method,
    headers: {
      ...(body !== undefined ? { "Content-Type": "application/json" } : {}),
      ...(cookie ? { Cookie: cookie } : {}),
      ...headers,
    },
    body: body !== undefined ? JSON.stringify(body) : undefined,
  });
  const setCookie = res.headers.get("set-cookie");
  return { status: res.status, data: await res.json().catch(() => null), cookie: setCookie?.split(";")[0] };
}

let passed = 0;
async function check(name, fn) {
  await fn();
  passed++;
  console.log(`  ok  ${name}`);
}

async function cleanup() {
  await db.query("delete from devices where id = any($1)", [[DEVICE, DEVICE2]]);
  await db.query("delete from users where username = any($1)", [[USER_A, USER_B]]);
}

try {
  await cleanup();
  const ts = (minAgo) => new Date(Date.now() - minAgo * 60000).toISOString().replace("Z", "+00:00");

  let token, code;
  await check("기기 등록 → 토큰/코드 발급", async () => {
    const r = await call("POST", "/api/register-device", { body: { device_id: DEVICE } });
    assert.equal(r.status, 200);
    assert.match(r.data.device_token, /^[0-9a-f]{64}$/);
    assert.match(r.data.pairing_code, /^[A-HJ-NP-Z2-9]{6}$/);
    ({ device_token: token, pairing_code: code } = r.data);
  });

  await check("잘못된 device_id 거부", async () => {
    const r = await call("POST", "/api/register-device", { body: { device_id: "a1:b2" } });
    assert.equal(r.status, 400);
  });

  const payload = {
    device_id: DEVICE,
    readings: [
      { ts: ts(10), steps: 120, sun_seconds: 240, uv_index: 3.2, lux: 850 },
      { ts: ts(5), steps: 340, sun_seconds: 300, uv_index: 2.8, lux: 790 },
    ],
    missions: [{ ts: ts(5), mission: "walk", status: "completed" }],
  };

  await check("측정 데이터 저장", async () => {
    const r = await call("POST", "/api/readings", { body: payload, headers: { "x-device-token": token } });
    assert.equal(r.status, 200);
    assert.deepEqual(r.data, { accepted_readings: 2, accepted_missions: 1 });
  });

  await check("같은 데이터 재전송 → 중복 저장 안 됨", async () => {
    const r = await call("POST", "/api/readings", { body: payload, headers: { "x-device-token": token } });
    assert.deepEqual(r.data, { accepted_readings: 0, accepted_missions: 0 });
  });

  await check("토큰 틀리면 401", async () => {
    const r = await call("POST", "/api/readings", { body: payload, headers: { "x-device-token": "0".repeat(64) } });
    assert.equal(r.status, 401);
  });

  await check("시간 형식 틀리면 400 + 어느 항목인지 알려줌", async () => {
    const r = await call("POST", "/api/readings", {
      body: { device_id: DEVICE, readings: [{ ts: "어제", steps: 1 }] },
      headers: { "x-device-token": token },
    });
    assert.equal(r.status, 400);
    assert.equal(r.data.error, "invalid_readings");
    assert.match(r.data.detail, /readings\[0\]\.ts/);
  });

  await check("sun_seconds가 음수/소수면 거부", async () => {
    for (const bad of [-1, 1.5, "60"]) {
      const r = await call("POST", "/api/readings", {
        body: { device_id: DEVICE, readings: [{ ts: ts(3), sun_seconds: bad }] },
        headers: { "x-device-token": token },
      });
      assert.equal(r.status, 400);
      assert.match(r.data.detail, /sun_seconds/);
    }
  });

  await check("미래 시각 거부", async () => {
    const future = new Date(Date.now() + 3600000).toISOString();
    const r = await call("POST", "/api/readings", {
      body: { device_id: DEVICE, readings: [{ ts: future, steps: 1 }] },
      headers: { "x-device-token": token },
    });
    assert.equal(r.status, 400);
  });

  let cookieA, cookieB;
  await check("회원가입 → 로그인 쿠키 발급", async () => {
    const a = await call("POST", "/api/signup", { body: { username: USER_A, password: "secret123" } });
    assert.equal(a.status, 200);
    assert.ok(a.cookie?.startsWith("mg_session="));
    cookieA = a.cookie;
    const b = await call("POST", "/api/signup", { body: { username: USER_B, password: "secret123" } });
    cookieB = b.cookie;
  });

  await check("같은 아이디 가입 거부", async () => {
    const r = await call("POST", "/api/signup", { body: { username: USER_A.toUpperCase(), password: "secret123" } });
    assert.equal(r.status, 409);
  });

  await check("틀린 비밀번호 로그인 거부, 맞으면 성공", async () => {
    assert.equal((await call("POST", "/api/login", { body: { username: USER_A, password: "nope123" } })).status, 401);
    assert.equal((await call("POST", "/api/login", { body: { username: USER_A, password: "secret123" } })).status, 200);
  });

  await check("로그인 안 하면 조회 불가", async () => {
    assert.equal((await call("GET", "/api/readings")).status, 401);
  });

  await check("연결 전에는 내 데이터 0개", async () => {
    const r = await call("GET", "/api/readings", { cookie: cookieA });
    assert.equal(r.data.readings.length, 0);
  });

  await check("틀린 코드로 연결 실패", async () => {
    const r = await call("POST", "/api/claim", { body: { code: "XXXXXX" }, cookie: cookieA });
    assert.equal(r.status, 400);
  });

  await check("페어링 코드로 기기 연결 (소문자/공백 허용)", async () => {
    const r = await call("POST", "/api/claim", { body: { code: ` ${code.toLowerCase()} ` }, cookie: cookieA });
    assert.equal(r.status, 200);
    assert.equal(r.data.device_id, DEVICE);
  });

  await check("연결 후 내 데이터 조회 + 기기 목록", async () => {
    const r = await call("GET", "/api/readings", { cookie: cookieA });
    assert.equal(r.data.readings.length, 2);
    assert.deepEqual(r.data.readings.map((x) => x.sun_seconds), [240, 300]);
    const m = await call("GET", "/api/missions", { cookie: cookieA });
    assert.equal(m.data.missions.length, 1);
    const me = await call("GET", "/api/me", { cookie: cookieA });
    assert.equal(me.data.devices[0].id, DEVICE);
  });

  await check("이미 쓴 코드 재사용 불가 + 다른 유저는 내 데이터 못 봄", async () => {
    assert.equal((await call("POST", "/api/claim", { body: { code }, cookie: cookieB })).status, 400);
    assert.equal((await call("GET", "/api/readings", { cookie: cookieB })).data.readings.length, 0);
  });

  await check("주인 있는 기기는 재등록(토큰 탈취) 불가", async () => {
    const r = await call("POST", "/api/register-device", { body: { device_id: DEVICE } });
    assert.equal(r.status, 409);
    const still = await call("POST", "/api/readings", {
      body: { device_id: DEVICE, readings: [{ ts: ts(1), steps: 5 }] },
      headers: { "x-device-token": token },
    });
    assert.equal(still.status, 200);
  });

  await check("주인 없는 기기 재등록 → 이전 토큰 무효", async () => {
    const first = await call("POST", "/api/register-device", { body: { device_id: DEVICE2 } });
    const second = await call("POST", "/api/register-device", { body: { device_id: DEVICE2 } });
    const old = await call("POST", "/api/readings", {
      body: { device_id: DEVICE2, readings: [] },
      headers: { "x-device-token": first.data.device_token },
    });
    assert.equal(old.status, 401);
    const fresh = await call("POST", "/api/readings", {
      body: { device_id: DEVICE2, readings: [] },
      headers: { "x-device-token": second.data.device_token },
    });
    assert.equal(fresh.status, 200);
  });

  await check("로그아웃 후 조회 불가", async () => {
    await call("POST", "/api/logout", { body: {}, cookie: cookieA });
    assert.equal((await call("GET", "/api/me", { cookie: cookieA })).status, 401);
  });

  await check("대시보드 페이지 응답", async () => {
    const res = await fetch(BASE + "/");
    assert.equal(res.status, 200);
    assert.match(await res.text(), /마음고치 대시보드/);
  });

  console.log(`\n${passed}개 통과`);
} finally {
  await cleanup();
  server.close();
  await db.end();
}
