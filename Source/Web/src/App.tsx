import { lazy, Suspense, useCallback, useEffect, useState } from "react";
import { games } from "./catalog";
import type { GameId } from "./catalog";
import { formatPlaytime, readProfile, subscribeToStats } from "./stats";
import type { Profile } from "./stats";
import { GameSession } from "./session";

const SnakeGame = lazy(() => import("./games/snake/SnakeGame"));
const CubeGame = lazy(() => import("./games/cube/CubeGame"));

const emptyProfile: Profile = { schemaVersion: 1, games: {} };

function message(error: unknown): string {
  return error instanceof Error ? error.message : String(error);
}

export default function App() {
  const [route, setRoute] = useState(window.location.hash);
  const [profile, setProfile] = useState<Profile>(emptyProfile);
  const [error, setError] = useState("");

  const refreshStats = useCallback(() => {
    void readProfile().then((value) => { setProfile(value); setError(""); })
      .catch((cause: unknown) => setError(`Statistics unavailable: ${message(cause)}`));
  }, []);

  useEffect(() => {
    const navigate = () => setRoute(window.location.hash);
    window.addEventListener("hashchange", navigate);
    refreshStats();
    const unsubscribe = subscribeToStats(refreshStats);
    return () => { window.removeEventListener("hashchange", navigate); unsubscribe(); };
  }, [refreshStats]);

  const current = route === "#/snake" ? "snake" : route === "#/cube" ? "webgl-cube" : null;
  return (
    <main className="app">
      <header>
        <p className="eyebrow">ARCADE</p>
        <h1>{current ? games.find((game) => game.id === current)?.title : "Choose your game"}</h1>
        {current ? <a className="back" href="#">← Back to arcade</a> : <p>Discover a game and press play.</p>}
      </header>
      {error && <div role="alert" className="error">{error}</div>}
      {current ? (
        <GameView key={current} id={current} reportError={(cause) => setError(`Statistics unavailable: ${message(cause)}`)} />
      ) : (
        <>
          <p className="total">Total playtime: {formatPlaytime(Object.values(profile.games).reduce((total, game) => total + game.playtimeSeconds, 0))}</p>
          <div className="grid">
            {games.map((game) => {
              const stats = profile.games[game.id];
              return (
                <article className="card" key={game.id}>
                  <div className="accent" />
                  <h2>{game.title}</h2>
                  <p>{game.description}</p>
                  <p>{stats ? `Played ${formatPlaytime(stats.playtimeSeconds)}` : "Not played yet"}</p>
                  {game.id === "snake" && stats?.highScore != null && <p>Best score: {stats.highScore}</p>}
                  <a className="button" href={game.path}>Play {game.title}</a>
                </article>
              );
            })}
          </div>
        </>
      )}
    </main>
  );
}

function GameView({ id, reportError }: { id: GameId; reportError: (error: unknown) => void }) {
  const [session] = useState(() => new GameSession(id, reportError));
  const [active, setActive] = useState(!document.hidden);
  const [playing, setPlaying] = useState(false);
  useEffect(() => {
    const visibility = () => setActive(!document.hidden);
    const pagehide = () => session.pause();
    document.addEventListener("visibilitychange", visibility);
    window.addEventListener("pagehide", pagehide);
    return () => { document.removeEventListener("visibilitychange", visibility); window.removeEventListener("pagehide", pagehide); };
  }, [session]);
  useEffect(() => {
    if (active && playing) session.start(); else session.pause();
    return () => session.pause();
  }, [active, playing, session]);

  return (
    <Suspense fallback={<p>Loading game...</p>}>
      {id === "snake" ? <SnakeGame active={active} reportError={reportError} setPlaying={setPlaying} /> :
        <CubeGame active={active} setPlaying={setPlaying} />}
    </Suspense>
  );
}
