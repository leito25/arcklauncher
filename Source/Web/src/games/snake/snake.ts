export type Point = { x: number; y: number };
export type Direction = "up" | "down" | "left" | "right";
export type SnakeState = { snake: Point[]; direction: Direction; food: Point; score: number; over: boolean; won: boolean };
export const boardSize = 16;
const vectors: Record<Direction, Point> = {
  up: { x: 0, y: -1 }, down: { x: 0, y: 1 },
  left: { x: -1, y: 0 }, right: { x: 1, y: 0 }
};

function chooseFood(snake: Point[], random: () => number): Point | null {
  const free: Point[] = [];
  for (let y = 0; y < boardSize; y++) {
    for (let x = 0; x < boardSize; x++) {
      if (!snake.some((segment) => segment.x === x && segment.y === y)) free.push({ x, y });
    }
  }
  return free.length ? free[Math.floor(random() * free.length)] : null;
}

export function newSnake(random: () => number = Math.random): SnakeState {
  const snake = [{ x: 7, y: 8 }, { x: 6, y: 8 }, { x: 5, y: 8 }];
  const food = chooseFood(snake, random);
  if (!food) throw new Error("Cannot place initial Snake food.");
  return { snake, direction: "right", food, score: 0, over: false, won: false };
}

export function changeDirection(state: SnakeState, direction: Direction): Direction {
  const current = vectors[state.direction];
  const next = vectors[direction];
  return current.x + next.x === 0 && current.y + next.y === 0 ? state.direction : direction;
}

export function stepSnake(state: SnakeState, direction: Direction, random: () => number = Math.random): SnakeState {
  if (state.over) return state;
  const nextDirection = changeDirection(state, direction);
  const vector = vectors[nextDirection];
  const head = { x: state.snake[0].x + vector.x, y: state.snake[0].y + vector.y };
  const eating = head.x === state.food.x && head.y === state.food.y;
  const body = eating ? state.snake : state.snake.slice(0, -1);
  if (head.x < 0 || head.x >= boardSize || head.y < 0 || head.y >= boardSize ||
      body.some((segment) => segment.x === head.x && segment.y === head.y)) {
    return { ...state, direction: nextDirection, over: true };
  }
  const snake = [head, ...body];
  if (!eating) return { ...state, snake, direction: nextDirection };
  const food = chooseFood(snake, random);
  return { ...state, snake, direction: nextDirection, score: state.score + 1,
    food: food ?? state.food, over: food === null, won: food === null };
}
