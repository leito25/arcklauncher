import "fake-indexeddb/auto";
import { beforeEach, describe, expect, it } from "vitest";
import { readProfile, updateStats } from "./stats";

function deleteDatabase(): Promise<void> {
  return new Promise((resolve, reject) => {
    const request = indexedDB.deleteDatabase("arcade-portfolio");
    request.onsuccess = () => resolve();
    request.onerror = () => reject(request.error);
  });
}

async function writeRaw(value: unknown): Promise<void> {
  const db = await new Promise<IDBDatabase>((resolve, reject) => {
    const request = indexedDB.open("arcade-portfolio", 1);
    request.onupgradeneeded = () => request.result.createObjectStore("profiles");
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
  await new Promise<void>((resolve, reject) => {
    const transaction = db.transaction("profiles", "readwrite");
    transaction.objectStore("profiles").put(value, "profile");
    transaction.oncomplete = () => resolve();
    transaction.onerror = () => reject(transaction.error);
  });
  db.close();
}

describe("local profile", () => {
  beforeEach(deleteDatabase);
  it("sums concurrent updates and retains the highest score", async () => {
    await Promise.all([
      updateStats("snake", 10, 4), updateStats("snake", 20, 2),
      updateStats("webgl-cube", 15)
    ]);
    const profile = await readProfile();
    expect(profile.games.snake).toEqual({ playtimeSeconds: 30, highScore: 4, achievements: [] });
    expect(profile.games["webgl-cube"].playtimeSeconds).toBe(15);
  });
  it("rejects unsupported profiles without overwriting them", async () => {
    await writeRaw({ schemaVersion: 2, games: {} });
    await expect(readProfile()).rejects.toThrow("unsupported");
    await expect(updateStats("snake", 1)).rejects.toThrow("unsupported");
    await expect(readProfile()).rejects.toThrow("unsupported");
  });
  it("rejects malformed or unsafe data", async () => {
    await writeRaw({ schemaVersion: 1, games: { snake: { playtimeSeconds: -1, highScore: null, achievements: [] } } });
    await expect(readProfile()).rejects.toThrow("Invalid game statistics");
    await expect(updateStats("snake", -1)).rejects.toThrow("Invalid playtime");
  });
});
