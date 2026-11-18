import { useEffect, useRef, useState } from "react";

type Props = { active: boolean; setPlaying: (playing: boolean) => void };

const vertexShader = `
  attribute vec3 position;
  attribute vec3 color;
  uniform float angleX;
  uniform float angleY;
  uniform float aspect;
  varying vec3 vColor;
  void main() {
    float cx = cos(angleX), sx = sin(angleX), cy = cos(angleY), sy = sin(angleY);
    vec3 p = vec3(position.x * cy + position.z * sy, position.y, -position.x * sy + position.z * cy);
    p = vec3(p.x, p.y * cx - p.z * sx, p.y * sx + p.z * cx);
    float depth = 4.0 - p.z;
    gl_Position = vec4(p.x * 2.0 / aspect, p.y * 2.0, depth - 0.1, depth);
    vColor = color;
  }
`;
const fragmentShader = `
  precision mediump float;
  varying vec3 vColor;
  void main() { gl_FragColor = vec4(vColor, 1.0); }
`;
const vertices = new Float32Array([
  -1,-1,1, 1,-1,1, 1,1,1, -1,1,1,
  1,-1,-1, -1,-1,-1, -1,1,-1, 1,1,-1,
  -1,1,1, 1,1,1, 1,1,-1, -1,1,-1,
  -1,-1,-1, 1,-1,-1, 1,-1,1, -1,-1,1,
  1,-1,1, 1,-1,-1, 1,1,-1, 1,1,1,
  -1,-1,-1, -1,-1,1, -1,1,1, -1,1,-1
]);
const colors = new Float32Array([
  ...Array(4).fill([0.22,0.74,0.97]).flat(), ...Array(4).fill([0.15,0.4,0.76]).flat(),
  ...Array(4).fill([0.49,0.83,0.99]).flat(), ...Array(4).fill([0.12,0.3,0.54]).flat(),
  ...Array(4).fill([0.25,0.55,0.85]).flat(), ...Array(4).fill([0.18,0.46,0.68]).flat()
]);
const indices = new Uint16Array(Array.from({ length: 6 }, (_, face) =>
  [0,1,2,0,2,3].map((index) => index + face * 4)).flat());

function shader(gl: WebGLRenderingContext, type: number, source: string): WebGLShader {
  const result = gl.createShader(type);
  if (!result) throw new Error("Could not create a WebGL shader.");
  gl.shaderSource(result, source);
  gl.compileShader(result);
  if (!gl.getShaderParameter(result, gl.COMPILE_STATUS)) {
    const details = gl.getShaderInfoLog(result);
    gl.deleteShader(result);
    throw new Error(`WebGL shader compilation failed: ${details}`);
  }
  return result;
}

export default function CubeGame({ active, setPlaying }: Props) {
  const canvas = useRef<HTMLCanvasElement>(null);
  const activeRef = useRef(active);
  const pausedRef = useRef(false);
  const angles = useRef({ x: 0.4, y: 0.4 });
  const drag = useRef<{ x: number; y: number } | null>(null);
  const [paused, setPaused] = useState(false);
  const [error, setError] = useState("");
  activeRef.current = active;
  pausedRef.current = paused;

  useEffect(() => {
    const element = canvas.current;
    if (!element) return;
    let frame = 0;
    let last = 0;
    let dispose = () => {};
    const initialize = () => {
      try {
        const gl = element.getContext("webgl", { antialias: true });
        if (!gl) throw new Error("WebGL is unavailable on this device.");
        const program = gl.createProgram();
        if (!program) throw new Error("Could not create the WebGL program.");
        const vertex = shader(gl, gl.VERTEX_SHADER, vertexShader);
        const fragment = shader(gl, gl.FRAGMENT_SHADER, fragmentShader);
        gl.attachShader(program, vertex);
        gl.attachShader(program, fragment);
        gl.linkProgram(program);
        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(`WebGL program failed: ${gl.getProgramInfoLog(program)}`);
        gl.useProgram(program);
        const buffers: WebGLBuffer[] = [];
        const attribute = (name: string, values: Float32Array) => {
          const buffer = gl.createBuffer();
          if (!buffer) throw new Error("Could not create a WebGL buffer.");
          buffers.push(buffer);
          gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
          gl.bufferData(gl.ARRAY_BUFFER, values, gl.STATIC_DRAW);
          const location = gl.getAttribLocation(program, name);
          gl.enableVertexAttribArray(location);
          gl.vertexAttribPointer(location, 3, gl.FLOAT, false, 0, 0);
        };
        attribute("position", vertices);
        attribute("color", colors);
        const indexBuffer = gl.createBuffer();
        if (!indexBuffer) throw new Error("Could not create the WebGL index buffer.");
        buffers.push(indexBuffer);
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);
        gl.enable(gl.DEPTH_TEST);
        gl.clearColor(0.063, 0.086, 0.149, 1);
        const render = (now: number) => {
          frame = requestAnimationFrame(render);
          if (!activeRef.current) { last = 0; return; }
          const delta = last ? Math.min((now - last) / 1000, 0.1) : 0;
          last = now;
          if (!pausedRef.current && !drag.current) angles.current.y += delta * 0.6;
          const width = Math.max(1, Math.round(element.clientWidth * devicePixelRatio));
          const height = Math.max(1, Math.round(element.clientHeight * devicePixelRatio));
          if (element.width !== width || element.height !== height) { element.width = width; element.height = height; }
          gl.viewport(0, 0, width, height);
          gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
          gl.uniform1f(gl.getUniformLocation(program, "angleX"), angles.current.x);
          gl.uniform1f(gl.getUniformLocation(program, "angleY"), angles.current.y);
          gl.uniform1f(gl.getUniformLocation(program, "aspect"), width / height);
          gl.drawElements(gl.TRIANGLES, indices.length, gl.UNSIGNED_SHORT, 0);
        };
        frame = requestAnimationFrame(render);
        dispose = () => {
          cancelAnimationFrame(frame);
          buffers.forEach((buffer) => gl.deleteBuffer(buffer));
          gl.deleteShader(vertex);
          gl.deleteShader(fragment);
          gl.deleteProgram(program);
        };
        setError("");
        setPlaying(!pausedRef.current);
      } catch (cause) {
        setError(cause instanceof Error ? cause.message : String(cause));
        setPlaying(false);
      }
    };
    const lost = (event: Event) => { event.preventDefault(); cancelAnimationFrame(frame); setError("WebGL context lost. Waiting for recovery."); setPlaying(false); };
    const restored = () => { dispose(); initialize(); };
    element.addEventListener("webglcontextlost", lost);
    element.addEventListener("webglcontextrestored", restored);
    initialize();
    return () => { dispose(); element.removeEventListener("webglcontextlost", lost); element.removeEventListener("webglcontextrestored", restored); };
  }, [setPlaying]);

  const toggle = () => {
    setPaused((value) => { setPlaying(value); return !value; });
  };
  return (
    <section className="game-panel">
      {error && <p role="alert" className="error">{error}</p>}
      <canvas ref={canvas} className="cube-canvas" aria-label="Interactive WebGL cube"
        onPointerDown={(event) => { event.currentTarget.setPointerCapture(event.pointerId); drag.current = { x: event.clientX, y: event.clientY }; }}
        onPointerMove={(event) => {
          if (!drag.current) return;
          angles.current.y += (event.clientX - drag.current.x) * 0.01;
          angles.current.x += (event.clientY - drag.current.y) * 0.01;
          drag.current = { x: event.clientX, y: event.clientY };
        }}
        onPointerUp={() => { drag.current = null; }}
        onPointerCancel={() => { drag.current = null; }} />
      <button onClick={toggle} disabled={!!error}>{paused ? "Resume" : "Pause"} rotation</button>
      <p>Drag the cube with a mouse or touch. Pause stops the playtime counter.</p>
    </section>
  );
}
