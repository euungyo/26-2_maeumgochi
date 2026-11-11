// 마음고치 로컬 서버 시작점
// 실행: server 폴더에서 `npm start` (먼저 `docker compose up -d`로 DB를 켜둬야 함)
import { createServer } from "node:http";
import { networkInterfaces } from "node:os";
import { createApp } from "./app.js";
import { connectDb, DEFAULT_DATABASE_URL } from "./db.js";

// .env 파일이 있으면 읽기 (없으면 기본값 사용)
try {
  process.loadEnvFile(new URL("../.env", import.meta.url));
} catch {
  // .env 없음 → 기본값
}

const PORT = Number(process.env.PORT) || 3000;

let db;
try {
  db = await connectDb(process.env.DATABASE_URL || DEFAULT_DATABASE_URL);
} catch (e) {
  console.error(`\n[DB 연결 실패] ${e.message}\n`);
  process.exit(1);
}

const app = createApp({ db, publicDir: new URL("../public/", import.meta.url) });

// 0.0.0.0 = 이 컴퓨터의 모든 네트워크에서 접속 허용 (같은 WiFi의 ESP32가 들어올 수 있게)
createServer(app).listen(PORT, "0.0.0.0", () => {
  console.log("\n마음고치 서버 실행 중 (DB 연결 OK)");
  console.log(`\n  이 컴퓨터에서 대시보드:  http://localhost:${PORT}`);
  console.log("  ESP32 / 같은 WiFi 기기에서 쓸 API 주소:");
  for (const addrs of Object.values(networkInterfaces())) {
    for (const a of addrs ?? []) {
      if (a.family === "IPv4" && !a.internal) console.log(`    http://${a.address}:${PORT}/api`);
    }
  }
  console.log("\n  끄려면 Ctrl+C\n");
});

const shutdown = async () => {
  await db.end();
  process.exit(0);
};
process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);
