export const games = [
  { id: "snake", title: "Snake", description: "Eat, grow and beat your best score.", path: "#/snake" },
  { id: "webgl-cube", title: "WebGL Cube", description: "Explore a rotating 3D cube.", path: "#/cube" }
] as const;

export type GameId = (typeof games)[number]["id"];
