// 요청을 받아서 처리하는 부분 (API + 대시보드 페이지)
import { readFileSync } from "node:fs";
import { transaction } from "./db.js";
import {
  hashPassword,
  pairingCode,
  randomToken,
  safeEqualHex,
  sha256,
  verifyPassword,
} from "./security.js";
import {
  DEVICE_ID_RE,
  MAX_ITEMS,
  parseCredentials,
  parseMissions,
  parseReadings,
  ValidationError,
} from "./validate.js";

const PAIRING_TTL_MS = 365 * 24 * 60 * 60 * 1000; // 페어링 코드 1년 유효
const SESSION_TTL_MS = 7 * 24 * 3600 * 1000; // 로그인 7일 유지
const MAX_BODY_BYTES = 1024 * 1024; // 요청 최대 1MB
const SESSION_COOKIE = "mg_session";

class HttpError extends Error {
  constructor(status, error, detail) {
    super(detail ?? error);
    this.status = status;
    this.error = error;
    this.detail = detail;
  }
}

function sendJson(res, status, body, headers = {}) {
  res.writeHead(status, { "Content-Type": "application/json; charset=utf-8", ...headers });
  res.end(JSON.stringify(body));
}

async function readJson(req) {
  if (!String(req.headers["content-type"] ?? "").includes("application/json")) {
    throw new HttpError(415, "content_type_must_be_json");
  }
  const chunks = [];
  let size = 0;
  for await (const chunk of req) {
    size += chunk.length;
    if (size > MAX_BODY_BYTES) throw new HttpError(413, "body_too_large");
    chunks.push(chunk);
  }
  try {
    const parsed = JSON.parse(Buffer.concat(chunks).toString("utf8"));
    if (typeof parsed !== "object" || parsed === null || Array.isArray(parsed)) throw new Error();
    return parsed;
  } catch {
    throw new HttpError(400, "invalid_json");
  }
}

function getCookie(req, name) {
  for (const part of String(req.headers.cookie ?? "").split(";")) {
    const [k, ...v] = part.trim().split("=");
    if (k === name) return decodeURIComponent(v.join("="));
  }
  return null;
}

const sessionCookie = (token, maxAgeSec) =>
  `${SESSION_COOKIE}=${token}; HttpOnly; SameSite=Lax; Path=/; Max-Age=${maxAgeSec}`;


const sinceFromQuery = (url) => {
  const days = Math.min(Math.max(Number(url.searchParams.get("days")) || 7, 1), 90);
  return new Date(Date.now() - days * 24 * 3600 * 1000);
};

export function createApp({ db, publicDir }) {
  const one = async (sql, params) => (await db.query(sql, params)).rows[0];
  const all = async (sql, params) => (await db.query(sql, params)).rows;

  // ── 로그인 세션 ──
  async function startSession(userId) {
    const token = randomToken();
    await db.query("delete from sessions where expires_at <= now()");
    await db.query("insert into sessions (token_hash, user_id, expires_at) values ($1, $2, $3)", [
      sha256(token),
      userId,
      new Date(Date.now() + SESSION_TTL_MS),
    ]);
    return sessionCookie(token, SESSION_TTL_MS / 1000);
  }

  async function requireUser(req) {
    const token = getCookie(req, SESSION_COOKIE);
    const user = token
      ? await one(
        `select u.id, u.username from sessions s join users u on u.id = s.user_id
           where s.token_hash = $1 and s.expires_at > now()`,
        [sha256(token)],
      )
      : undefined;
    if (!user) throw new HttpError(401, "login_required");
    return user;
  }

  const userByName = (username) =>
    one("select id, username, password_hash from users where lower(username) = lower($1)", [username]);

  // ── 라우트 ──
  const routes = {
    // 기기: 등록 → 토큰 + 페어링 코드 발급
    "POST /api/register-device": async (req, res) => {
      const body = await readJson(req);
      const deviceId = body.device_id;
      if (typeof deviceId !== "string" || !DEVICE_ID_RE.test(deviceId)) {
        throw new HttpError(400, "invalid_device_id");
      }
      const existing = await one("select owner_id from devices where id = $1", [deviceId]);
      if (existing?.owner_id) throw new HttpError(409, "device_already_claimed");

      const token = randomToken();
      const expiresAt = new Date(Date.now() + PAIRING_TTL_MS);
      for (let attempt = 0; attempt < 5; attempt++) {
        const code = pairingCode();
        try {
          // 주인이 없는 기기만 토큰/코드를 새로 발급 (주인 있는 기기는 where 조건에서 걸러짐)
          const { rowCount } = await db.query(
            `insert into devices (id, token_hash, pairing_code, pairing_expires_at) values ($1, $2, $3, $4)
             on conflict (id) do update set
               token_hash = excluded.token_hash,
               pairing_code = excluded.pairing_code,
               pairing_expires_at = excluded.pairing_expires_at
             where devices.owner_id is null`,
            [deviceId, sha256(token), code, expiresAt],
          );
          if (rowCount === 0) throw new HttpError(409, "device_already_claimed");
          return sendJson(res, 200, {
            device_token: token,
            pairing_code: code,
            expires_at: expiresAt.toISOString(),
          });
        } catch (e) {
          if (e instanceof HttpError) throw e;
          if (e.code !== "23505") throw e; // 페어링 코드가 우연히 겹친 경우만 다시 뽑기
        }
      }
      throw new HttpError(503, "pairing_code_collision");
    },

    // 기기: 측정 기록 + 미션 이벤트 받기
    "POST /api/readings": async (req, res) => {
      const body = await readJson(req);
      const deviceId = body.device_id;
      if (typeof deviceId !== "string" || !DEVICE_ID_RE.test(deviceId)) {
        throw new HttpError(400, "invalid_device_id");
      }
      const token = req.headers["x-device-token"];
      const device = await one("select token_hash from devices where id = $1", [deviceId]);
      if (typeof token !== "string" || !device || !safeEqualHex(sha256(token), device.token_hash)) {
        throw new HttpError(401, "unauthorized");
      }

      const rawReadings = body.readings ?? [];
      const rawMissions = body.missions ?? [];
      if (!Array.isArray(rawReadings)) throw new HttpError(400, "invalid_readings", "readings: 배열이어야 함");
      if (!Array.isArray(rawMissions)) throw new HttpError(400, "invalid_missions", "missions: 배열이어야 함");
      if (rawReadings.length > MAX_ITEMS || rawMissions.length > MAX_ITEMS) {
        throw new HttpError(413, "too_many_items", `각 배열 최대 ${MAX_ITEMS}개`);
      }

      let readings, missions;
      try {
        readings = parseReadings(rawReadings);
      } catch (e) {
        if (e instanceof ValidationError) throw new HttpError(400, "invalid_readings", e.message);
        throw e;
      }
      try {
        missions = parseMissions(rawMissions);
      } catch (e) {
        if (e instanceof ValidationError) throw new HttpError(400, "invalid_missions", e.message);
        throw e;
      }

      const result = await transaction(db, async (client) => {
        let acceptedReadings = 0;
        for (const r of readings) {
          const { rowCount } = await client.query(
            `insert into readings (device_id, measured_at, steps, sun_seconds, uv_index, lux)
             values ($1, $2, $3, $4, $5, $6)
             on conflict (device_id, measured_at) do nothing`,
            [deviceId, r.measured_at, r.steps, r.sun_seconds, r.uv_index, r.lux],
          );
          acceptedReadings += rowCount;
        }
        let acceptedMissions = 0;
        for (const m of missions) {
          const { rowCount } = await client.query(
            `insert into mission_events (device_id, occurred_at, mission, status) values ($1, $2, $3, $4)
             on conflict (device_id, occurred_at, mission) do nothing`,
            [deviceId, m.occurred_at, m.mission, m.status],
          );
          acceptedMissions += rowCount;
        }
        await client.query("update devices set last_seen_at = now() where id = $1", [deviceId]);
        return { accepted_readings: acceptedReadings, accepted_missions: acceptedMissions };
      });
      sendJson(res, 200, result);
    },

    // 웹: 회원가입
    "POST /api/signup": async (req, res) => {
      let creds;
      try {
        creds = parseCredentials(await readJson(req));
      } catch (e) {
        if (e instanceof ValidationError) throw new HttpError(400, "invalid_credentials", e.message);
        throw e;
      }
      let user;
      try {
        user = await one("insert into users (username, password_hash) values ($1, $2) returning id, username", [
          creds.username,
          hashPassword(creds.password),
        ]);
      } catch (e) {
        if (e.code === "23505") throw new HttpError(409, "username_taken", "이미 있는 아이디");
        throw e;
      }
      sendJson(res, 200, { username: user.username }, { "Set-Cookie": await startSession(user.id) });
    },

    // 웹: 로그인
    "POST /api/login": async (req, res) => {
      const body = await readJson(req);
      const user = typeof body.username === "string" ? await userByName(body.username.trim()) : undefined;
      const ok = user && typeof body.password === "string" && verifyPassword(body.password, user.password_hash);
      if (!ok) throw new HttpError(401, "invalid_login", "아이디 또는 비밀번호가 틀림");
      sendJson(res, 200, { username: user.username }, { "Set-Cookie": await startSession(user.id) });
    },

    // 웹: 로그아웃
    "POST /api/logout": async (req, res) => {
      const token = getCookie(req, SESSION_COOKIE);
      if (token) await db.query("delete from sessions where token_hash = $1", [sha256(token)]);
      sendJson(res, 200, { ok: true }, { "Set-Cookie": sessionCookie("", 0) });
    },

    // 웹: 내 정보 + 내 기기 목록
    "GET /api/me": async (req, res) => {
      const user = await requireUser(req);
      const devices = await all(
        "select id, created_at, last_seen_at from devices where owner_id = $1 order by created_at",
        [user.id],
      );
      sendJson(res, 200, { username: user.username, devices });
    },

    // 웹: 페어링 코드로 기기 연결
    "POST /api/claim": async (req, res) => {
      const user = await requireUser(req);
      const body = await readJson(req);
      const code = typeof body.code === "string" ? body.code.trim().toUpperCase() : "";
      const row = await one(
        `update devices set owner_id = $1, pairing_code = null, pairing_expires_at = null
         where pairing_code = $2 and pairing_expires_at > now() and owner_id is null
         returning id`,
        [user.id, code],
      );
      if (!row) throw new HttpError(400, "invalid_or_expired_code", "코드가 틀렸거나 만료됨");
      sendJson(res, 200, { device_id: row.id });
    },

    // 웹: 내 기기 측정 기록 (?days=7)
    "GET /api/readings": async (req, res, url) => {
      const user = await requireUser(req);
      const readings = await all(
        `select r.device_id, r.measured_at, r.steps, r.sun_seconds, r.uv_index, r.lux, r.received_at
         from readings r join devices d on d.id = r.device_id
         where d.owner_id = $1 and r.measured_at >= $2
         order by r.measured_at`,
        [user.id, sinceFromQuery(url)],
      );
      sendJson(res, 200, { readings });
    },

    // 웹: 내 기기 미션 기록 (?days=7)
    "GET /api/missions": async (req, res, url) => {
      const user = await requireUser(req);
      const missions = await all(
        `select m.device_id, m.occurred_at, m.mission, m.status
         from mission_events m join devices d on d.id = m.device_id
         where d.owner_id = $1 and m.occurred_at >= $2
         order by m.occurred_at desc`,
        [user.id, sinceFromQuery(url)],
      );
      sendJson(res, 200, { missions });
    },

    "GET /favicon.ico": async (req, res) => {
      res.writeHead(204);
      res.end();
    },

    // 대시보드 페이지
    "GET /": async (req, res) => {
      res.writeHead(200, { "Content-Type": "text/html; charset=utf-8", "Cache-Control": "no-store" });
      res.end(readFileSync(new URL("index.html", publicDir)));
    },
  };

  return async function handle(req, res) {
    const url = new URL(req.url, "http://localhost");
    const route = routes[`${req.method} ${url.pathname}`];
    try {
      if (!route) throw new HttpError(404, "not_found");
      await route(req, res, url);
    } catch (e) {
      if (e instanceof HttpError) {
        return sendJson(res, e.status, e.detail ? { error: e.error, detail: e.detail } : { error: e.error });
      }
      console.error(e);
      sendJson(res, 500, { error: "server_error" });
    }
  };
}
