import { describe, expect, it } from "vitest";
import { changeDirection, newSnake, stepSnake } from "./snake";
import type { SnakeState } from "./snake";

describe("Snake rules", () => {
  it("prevents reversal and grows on food", () => {
    const state = { ...newSnake(() => 0), food: { x: 8, y: 8 } };
    expect(changeDirection(state, "left")).toBe("right");
    const next = stepSnake(state, "left", () => 0);
    expect(next.snake).toHaveLength(4);
    expect(next.score).toBe(1);
    expect(next.direction).toBe("right");
    expect(next.snake).not.toContainEqual(next.food);
  });

  it("detects wall and self collisions but permits moving into the departing tail", () => {
    const base: SnakeState = { snake: [{ x: 0, y: 0 }, { x: 1, y: 0 }], direction: "up", food: { x: 5, y: 5 }, score: 0, over: false, won: false };
    expect(stepSnake(base, "up").over).toBe(true);
    const loop = { ...base, snake: [{ x: 1, y: 1 }, { x: 1, y: 2 }, { x: 0, y: 2 }, { x: 0, y: 1 }], direction: "up" as const };
    expect(stepSnake(loop, "left").over).toBe(false);
    expect(stepSnake({ ...loop, snake: [...loop.snake, { x: 0, y: 0 }] }, "left").over).toBe(true);
  });
});
