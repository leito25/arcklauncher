import type { GameId } from "./catalog";
import { updateStats } from "./stats";

export class GameSession {
  private startedAt: number | null = null;
  private remainder = 0;
  private pending = 0;
  private saving = false;
  private timer: ReturnType<typeof setInterval> | null = null;

  constructor(private readonly id: GameId, private readonly reportError: (error: unknown) => void) {}

  start(): void {
    if (this.startedAt !== null) return;
    this.startedAt = performance.now();
    this.timer = setInterval(() => this.checkpoint(), 5000);
  }

  pause(): void {
    if (this.startedAt === null) return;
    this.checkpoint();
    this.startedAt = null;
    if (this.timer !== null) clearInterval(this.timer);
    this.timer = null;
  }

  checkpoint(): void {
    if (this.startedAt === null) return;
    const now = performance.now();
    this.remainder += Math.max(0, now - this.startedAt);
    this.startedAt = now;
    const seconds = Math.floor(this.remainder / 1000);
    this.remainder -= seconds * 1000;
    this.pending += seconds;
    void this.flush();
  }

  private async flush(): Promise<void> {
    if (this.saving || this.pending === 0) return;
    this.saving = true;
    const seconds = this.pending;
    this.pending = 0;
    let failed = false;
    try {
      await updateStats(this.id, seconds);
    } catch (error) {
      failed = true;
      this.pending += seconds;
      this.reportError(error);
    } finally {
      this.saving = false;
    }
    if (!failed && this.pending > 0) void this.flush();
  }
}
