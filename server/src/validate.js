// 기기에서 온 데이터 형식 검사 (docs/api.md 기준)
export const DEVICE_ID_RE = /^[0-9A-F]{12}$/;
export const MAX_ITEMS = 500;

const MAX_FUTURE_MS = 10 * 60 * 1000; // RTC가 조금 빨라도 허용
const MISSION_RE = /^[a-z][a-z0-9_]{0,31}$/;
const MISSION_STATUSES = new Set(["started", "completed"]);

export class ValidationError extends Error {}

function parseTs(value, where, now) {
  if (typeof value !== "string" || !/(Z|[+-]\d{2}:\d{2})$/.test(value)) {
    throw new ValidationError(`${where}.ts: 타임존이 붙은 ISO 8601 문자열이어야 함`);
  }
  const t = Date.parse(value);
  if (Number.isNaN(t)) throw new ValidationError(`${where}.ts: 날짜 형식 오류`);
  if (t > now + MAX_FUTURE_MS) throw new ValidationError(`${where}.ts: 미래 시각 (기기 시간 확인)`);
  return new Date(t).toISOString(); // 항상 같은 형식(UTC)으로 저장 → 중복 판단이 정확해짐
}

function optionalNumber(value, where, integer) {
  if (value === undefined || value === null) return null;
  if (typeof value !== "number" || !Number.isFinite(value) || value < 0) {
    throw new ValidationError(`${where}: 0 이상의 숫자여야 함`);
  }
  if (integer && !Number.isInteger(value)) throw new ValidationError(`${where}: 정수여야 함`);
  return value;
}

function asObject(raw, where) {
  if (typeof raw !== "object" || raw === null || Array.isArray(raw)) {
    throw new ValidationError(`${where}: 객체여야 함`);
  }
  return raw;
}

export function parseReadings(items, now = Date.now()) {
  return items.map((raw, i) => {
    const where = `readings[${i}]`;
    const r = asObject(raw, where);
    return {
      measured_at: parseTs(r.ts, where, now),
      steps: optionalNumber(r.steps, `${where}.steps`, true),
      sun_seconds: optionalNumber(r.sun_seconds, `${where}.sun_seconds`, true),
      uv_index: optionalNumber(r.uv_index, `${where}.uv_index`, false),
      lux: optionalNumber(r.lux, `${where}.lux`, false),
    };
  });
}

export function parseMissions(items, now = Date.now()) {
  return items.map((raw, i) => {
    const where = `missions[${i}]`;
    const m = asObject(raw, where);
    if (typeof m.mission !== "string" || !MISSION_RE.test(m.mission)) {
      throw new ValidationError(`${where}.mission: 소문자/숫자/_ 최대 32자`);
    }
    if (typeof m.status !== "string" || !MISSION_STATUSES.has(m.status)) {
      throw new ValidationError(`${where}.status: started 또는 completed`);
    }
    return { occurred_at: parseTs(m.ts, where, now), mission: m.mission, status: m.status };
  });
}

export function parseCredentials(body) {
  const username = typeof body?.username === "string" ? body.username.trim() : "";
  const password = typeof body?.password === "string" ? body.password : "";
  if (!/^[A-Za-z0-9_]{3,20}$/.test(username)) {
    throw new ValidationError("아이디는 영문/숫자/_ 3~20자");
  }
  if (password.length < 6 || password.length > 100) {
    throw new ValidationError("비밀번호는 6자 이상");
  }
  return { username, password };
}
