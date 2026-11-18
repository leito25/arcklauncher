import type { GameId } from "./catalog";

export interface GameStats {
  playtimeSeconds: number;
  highScore: number | null;
  achievements: string[];
}

export interface Profile {
  schemaVersion: 1;
  games: Record<string, GameStats>;
}

const databaseName = "arcade-portfolio";
const profileKey = "profile";
const channel = typeof BroadcastChannel === "undefined" ? null : new BroadcastChannel("arcade-profile");
const listeners = new Set<() => void>();
channel?.addEventListener("message", () => listeners.forEach((listener) => listener()));

export function subscribeToStats(listener: () => void): () => void {
  listeners.add(listener);
  return () => listeners.delete(listener);
}

function notify(): void {
  listeners.forEach((listener) => listener());
  channel?.postMessage("changed");
}

function validNumber(value: unknown): value is number {
  return typeof value === "number" && Number.isSafeInteger(value) && value >= 0;
}

function validateProfile(value: unknown): Profile {
  if (!value || typeof value !== "object" || Array.isArray(value)) {
    throw new Error("Invalid arcade profile.");
  }
  const profile = value as Record<string, unknown>;
  if (profile.schemaVersion !== 1 || !profile.games || typeof profile.games !== "object" || Array.isArray(profile.games)) {
    throw new Error("Invalid or unsupported arcade profile.");
  }
  for (const [id, entry] of Object.entries(profile.games)) {
    if (!/^[a-z0-9._-]+$/.test(id) || !entry || typeof entry !== "object" || Array.isArray(entry)) {
      throw new Error("Invalid game statistics in arcade profile.");
    }
    const game = entry as Record<string, unknown>;
    if (!validNumber(game.playtimeSeconds) ||
        !(game.highScore === null || validNumber(game.highScore)) ||
        !Array.isArray(game.achievements) ||
        !game.achievements.every((item: unknown) => typeof item === "string" && /^[a-z0-9._-]+$/.test(item))) {
      throw new Error("Invalid game statistics in arcade profile.");
    }
  }
  return value as Profile;
}

function openDatabase(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(databaseName, 1);
    request.onupgradeneeded = () => request.result.createObjectStore("profiles");
    request.onerror = () => reject(request.error ?? new Error("Cannot open arcade storage."));
    request.onblocked = () => reject(new Error("Arcade storage upgrade is blocked by another tab."));
    request.onsuccess = () => {
      request.result.onversionchange = () => request.result.close();
      resolve(request.result);
    };
  });
}

function emptyProfile(): Profile {
  return { schemaVersion: 1, games: {} };
}

export async function readProfile(): Promise<Profile> {
  const db = await openDatabase();
  try {
    return await new Promise((resolve, reject) => {
      const request = db.transaction("profiles", "readonly").objectStore("profiles").get(profileKey);
      request.onerror = () => reject(request.error ?? new Error("Cannot read arcade profile."));
      request.onsuccess = () => {
        try {
          resolve(request.result === undefined ? emptyProfile() : validateProfile(request.result));
        } catch (error) {
          reject(error);
        }
      };
    });
  } finally {
    db.close();
  }
}

export async function updateStats(id: GameId, seconds: number, score?: number): Promise<void> {
  if (!validNumber(seconds) || (score !== undefined && !validNumber(score))) {
    throw new Error("Invalid playtime or score.");
  }
  const db = await openDatabase();
  try {
    await new Promise<void>((resolve, reject) => {
      const transaction = db.transaction("profiles", "readwrite");
      const store = transaction.objectStore("profiles");
      const request = store.get(profileKey);
      let failure: unknown;
      request.onsuccess = () => {
        try {
          const profile = request.result === undefined ? emptyProfile() : validateProfile(request.result);
          const game = profile.games[id] ?? { playtimeSeconds: 0, highScore: null, achievements: [] };
          if (!Number.isSafeInteger(game.playtimeSeconds + seconds)) {
            throw new Error("Playtime exceeds the supported range.");
          }
          game.playtimeSeconds += seconds;
          if (score !== undefined && (game.highScore === null || score > game.highScore)) game.highScore = score;
          profile.games[id] = game;
          store.put(profile, profileKey);
        } catch (error) {
          failure = error;
          transaction.abort();
        }
      };
      transaction.oncomplete = () => resolve();
      transaction.onabort = () => reject(failure ?? transaction.error ?? new Error("Cannot save arcade profile."));
      transaction.onerror = () => { failure = transaction.error; };
    });
    notify();
  } finally {
    db.close();
  }
}

export function formatPlaytime(seconds: number): string {
  const minutes = Math.floor(seconds / 60);
  return minutes >= 60 ? `${Math.floor(minutes / 60)}h ${minutes % 60}m` : `${minutes}m`;
}
