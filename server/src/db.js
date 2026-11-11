// PostgreSQL 연결 + 스키마 적용
import { readFile } from "node:fs/promises";
import pg from "pg";

export const DEFAULT_DATABASE_URL = "postgres://maeumgochi:maeumgochi_local@localhost:5432/maeumgochi";

export async function connectDb(databaseUrl = DEFAULT_DATABASE_URL) {
  const pool = new pg.Pool({ connectionString: databaseUrl, max: 10 });
  try {
    const schema = await readFile(new URL("../db/schema.sql", import.meta.url), "utf8");
    await pool.query(schema);
  } catch (e) {
    await pool.end();
    if (e.code === "ECONNREFUSED") {
      throw new Error(
        "PostgreSQL에 연결할 수 없음. Docker Desktop이 켜져 있는지, `docker compose up -d`를 했는지 확인해 줘.",
      );
    }
    throw e;
  }
  return pool;
}

// 여러 쿼리를 하나로 묶기 (중간에 실패하면 전부 취소)
export async function transaction(pool, fn) {
  const client = await pool.connect();
  try {
    await client.query("begin");
    const result = await fn(client);
    await client.query("commit");
    return result;
  } catch (e) {
    await client.query("rollback");
    throw e;
  } finally {
    client.release();
  }
}
