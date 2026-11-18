import { afterEach, describe, expect, it, vi } from "vitest";

vi.mock("./stats", () => ({ updateStats: vi.fn().mockResolvedValue(undefined) }));
import { updateStats } from "./stats";
import { GameSession } from "./session";

describe("game sessions", () => {
  afterEach(() => { vi.useRealTimers(); vi.clearAllMocks(); });
  it("saves active seconds and excludes paused time", async () => {
    vi.useFakeTimers();
    const session = new GameSession("snake", vi.fn());
    session.start();
    await vi.advanceTimersByTimeAsync(2200);
    session.pause();
    await vi.advanceTimersByTimeAsync(5000);
    session.start();
    await vi.advanceTimersByTimeAsync(1200);
    session.pause();
    await vi.runAllTimersAsync();
    expect(vi.mocked(updateStats).mock.calls.map((call) => call[1])).toEqual([2, 1]);
  });
});
