import { useEffect, useRef, useState } from "react";
import Phaser from "phaser";
import { boardSize, changeDirection, newSnake, stepSnake } from "./snake";
import type { Direction, SnakeState } from "./snake";
import { updateStats } from "./stats";

type Props = { active: boolean; reportError: (error: unknown) => void; setPlaying: (playing: boolean) => void };
const size = 512;

export default function SnakeGame({ active, reportError, setPlaying }: Props) {
  const container = useRef<HTMLDivElement>(null);
  const game = useRef<Phaser.Game | null>(null);
  const state = useRef<SnakeState>(newSnake());
  const direction = useRef<Direction>("right");
  const [score, setScore] = useState(0);
  const [over, setOver] = useState(false);
  const [won, setWon] = useState(false);
  const reportRef = useRef(reportError);
  const playingRef = useRef(setPlaying);
  reportRef.current = reportError;
  playingRef.current = setPlaying;

  useEffect(() => {
    if (!container.current) return;
    class SnakeScene extends Phaser.Scene {
      graphics!: Phaser.GameObjects.Graphics;
      elapsed = 0;
      create() { this.graphics = this.add.graphics(); this.draw(); }
      update(_time: number, delta: number) {
        if (state.current.over) return;
        this.elapsed += delta;
        if (this.elapsed < 140) return;
        this.elapsed = 0;
        state.current = stepSnake(state.current, direction.current);
        direction.current = state.current.direction;
        setScore(state.current.score);
        this.draw();
        if (state.current.over) {
          setOver(true);
          setWon(state.current.won);
          playingRef.current(false);
          void updateStats("snake", 0, state.current.score).catch(reportRef.current);
        }
      }
      draw() {
        const cell = size / boardSize;
        this.graphics.clear();
        this.graphics.fillStyle(0x101626).fillRect(0, 0, size, size);
        this.graphics.fillStyle(0xfb7185).fillRect(state.current.food.x * cell + 3, state.current.food.y * cell + 3, cell - 6, cell - 6);
        this.graphics.fillStyle(0x38bdf8);
        for (const segment of state.current.snake) this.graphics.fillRect(segment.x * cell + 2, segment.y * cell + 2, cell - 4, cell - 4);
      }
    }
    game.current = new Phaser.Game({
      type: Phaser.CANVAS, width: size, height: size, parent: container.current,
      backgroundColor: "#101626", scene: SnakeScene,
      scale: { mode: Phaser.Scale.FIT, autoCenter: Phaser.Scale.CENTER_BOTH }
    });
    playingRef.current(true);
    const keydown = (event: KeyboardEvent) => {
      const keys: Record<string, Direction> = { ArrowUp: "up", ArrowDown: "down", ArrowLeft: "left", ArrowRight: "right" };
      const next = keys[event.key];
      if (next) { event.preventDefault(); direction.current = changeDirection(state.current, next); }
    };
    window.addEventListener("keydown", keydown);
    return () => { window.removeEventListener("keydown", keydown); game.current?.destroy(true); game.current = null; };
  }, []);

  useEffect(() => {
    if (active) game.current?.loop.wake(); else game.current?.loop.sleep();
  }, [active]);

  const restart = () => {
    state.current = newSnake();
    direction.current = "right";
    setScore(0);
    setOver(false);
    setWon(false);
    playingRef.current(true);
    game.current?.scene.scenes[0].scene.restart();
  };
  const steer = (next: Direction) => { direction.current = changeDirection(state.current, next); };
  return (
    <section className="game-panel">
      <div className="game-heading"><strong>Score: {score}</strong>{over && <strong role="status">{won ? "You won!" : "Game over"}</strong>}</div>
      <div className="snake-canvas" ref={container} aria-label="Snake game board" />
      <div className="controls" aria-label="Snake direction controls">
        <button onClick={() => steer("up")} aria-label="Move up">↑</button>
        <div><button onClick={() => steer("left")} aria-label="Move left">←</button>
          <button onClick={() => steer("down")} aria-label="Move down">↓</button>
          <button onClick={() => steer("right")} aria-label="Move right">→</button></div>
      </div>
      {over && <button onClick={restart}>Play again</button>}
      <p>Use arrow keys or the direction buttons. Eat the pink food and avoid walls and yourself.</p>
    </section>
  );
}
