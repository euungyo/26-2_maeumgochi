// 토큰, 비밀번호, 페어링 코드 관련 유틸
import { createHash, randomBytes, scryptSync, timingSafeEqual } from "node:crypto";

// 헷갈리는 글자(0/O, 1/I) 뺀 32자 → 256이 32로 나눠떨어져서 글자가 고르게 뽑힘
const PAIRING_ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

export const randomToken = (bytes = 32) => randomBytes(bytes).toString("hex");

export const sha256 = (text) => createHash("sha256").update(text).digest("hex");

export function pairingCode(length = 6) {
  return Array.from(randomBytes(length), (b) => PAIRING_ALPHABET[b % 32]).join("");
}

export function safeEqualHex(a, b) {
  if (typeof a !== "string" || typeof b !== "string" || a.length !== b.length) return false;
  return timingSafeEqual(Buffer.from(a, "hex"), Buffer.from(b, "hex"));
}

// 비밀번호는 scrypt로 해시 (같은 비밀번호라도 salt 때문에 매번 다른 값)
export function hashPassword(password) {
  const salt = randomBytes(16);
  const hash = scryptSync(password, salt, 32);
  return `scrypt$${salt.toString("hex")}$${hash.toString("hex")}`;
}

export function verifyPassword(password, stored) {
  const [scheme, saltHex, hashHex] = String(stored).split("$");
  if (scheme !== "scrypt" || !saltHex || !hashHex) return false;
  const hash = scryptSync(password, Buffer.from(saltHex, "hex"), 32);
  return safeEqualHex(hash.toString("hex"), hashHex);
}
