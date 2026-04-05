import React, { useEffect, useMemo, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { motion } from "framer-motion";
import { RotateCcw, RotateCw, MousePointerClick, Disc3, Keyboard, Layers3 } from "lucide-react";

const MATRIX_ROWS = 3;
const MATRIX_COLS = 3;
const LED_COUNT = 9;

const LAYERS = [
  {
    id: 0,
    name: "BASE",
    color: "rgb(80,150,255)",
    sub: "Rainbow/Breathing · Encoder = Scroll",
    keys: [
      ["Win+Tab", "↑", "Alt+Tab"],
      ["←", "Enter", "→"],
      ["Ctrl+Z", "↓", "Ctrl+R"],
    ],
    encoder: { ccw: "Mouse Wheel -", cw: "Mouse Wheel +" },
    rgbMode: "breathing rainbow",
  },
  {
    id: 1,
    name: "NAV",
    color: "rgb(70,220,255)",
    sub: "Blue/Turquoise · Encoder = Next/Prev Window",
    keys: [
      ["Win+Tab", "↑", "Alt+Tab"],
      ["←", "Enter", "→"],
      ["Ctrl+Z", "↓", "Ctrl+R"],
    ],
    encoder: { ccw: "Prev Window", cw: "Next Window" },
    rgbMode: "blue turquoise breathing",
  },
  {
    id: 2,
    name: "EDIT",
    color: "rgb(90,235,110)",
    sub: "Green/Yellow · Encoder = Select prev/next",
    keys: [
      ["Ctrl+A", "Ctrl+C", "Ctrl+V"],
      ["Ctrl+X", "Ctrl+Enter", "—"],
      ["Ctrl+Shift+Z", "Space", "Backspace"],
    ],
    encoder: { ccw: "Select Prev", cw: "Select Next" },
    rgbMode: "green yellow breathing",
  },
  {
    id: 3,
    name: "MEDIA",
    color: "rgb(255,95,210)",
    sub: "Pink/Yellow · Encoder = Volume",
    keys: [
      ["Prev", "Select", "Next"],
      ["Rewind", "Play/Pause", "Fwd"],
      ["Vol-", "Stop", "Vol+"],
    ],
    encoder: { ccw: "Volume -", cw: "Volume +" },
    rgbMode: "pink yellow breathing",
  },
  {
    id: 4,
    name: "FN",
    color: "rgb(255,165,60)",
    sub: "Orange/Yellow · Encoder = Left/Right",
    keys: [
      ["F13", "F14", "F15"],
      ["F16", "F17", "F18"],
      ["F19", "F20", "F21"],
    ],
    encoder: { ccw: "Arrow Left", cw: "Arrow Right" },
    rgbMode: "orange yellow breathing",
  },
  {
    id: 5,
    name: "RGB",
    color: "rgb(180,100,255)",
    sub: "Purple/Orange · Encoder = Brightness",
    keys: [
      ["SPD+", "SPD-", "RGB Toggle"],
      ["HUE+", "HUE-", "VAL+"],
      ["SAT+", "SAT-", "VAL-"],
    ],
    encoder: { ccw: "Brightness -", cw: "Brightness +" },
    rgbMode: "purple orange breathing",
  },
  {
    id: 6,
    name: "SELECT",
    color: "rgb(255,70,70)",
    sub: "Red blink · Turn encoder while held to change layer",
    keys: [
      ["TO 1", "TO 2", "TO 3"],
      ["TO 4", "TO 0", "—"],
      ["—", "—", "—"],
    ],
    encoder: { ccw: "Prev Layer", cw: "Next Layer" },
    rgbMode: "red selector pulse",
  },
];

function clamp(v, min, max) {
  return Math.min(max, Math.max(min, v));
}

function wrapLayer(v) {
  const total = LAYERS.length;
  return ((v % total) + total) % total;
}

function hsvToRgb(h, s, v) {
  let r = 0, g = 0, b = 0;
  const i = Math.floor(h * 6);
  const f = h * 6 - i;
  const p = v * (1 - s);
  const q = v * (1 - f * s);
  const t = v * (1 - (1 - f) * s);
  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
  return `rgb(${Math.round(r * 255)}, ${Math.round(g * 255)}, ${Math.round(b * 255)})`;
}

function parseRgb(rgb) {
  return rgb.match(/\d+/g)?.map(Number) || [80, 150, 255];
}

function mix(a, b, t) {
  const [ar, ag, ab] = parseRgb(a);
  const [br, bg, bb] = parseRgb(b);
  return `rgb(${Math.round(ar + (br - ar) * t)}, ${Math.round(ag + (bg - ag) * t)}, ${Math.round(ab + (bb - ab) * t)})`;
}

function keyLabelFromIndex(index, activeLayer) {
  const row = Math.floor(index / MATRIX_COLS);
  const col = index % MATRIX_COLS;
  return LAYERS[activeLayer].keys[row][col];
}

function buildLedColors(activeLayer, time, brightness, pressedIndex, selectorHeld) {
  const layer = LAYERS[activeLayer];
  const pulse = (Math.sin(time * 2.1) + 1) / 2;
  const fastPulse = (Math.sin(time * 6.4) + 1) / 2;
  const leds = [];

  for (let i = 0; i < LED_COUNT; i++) {
    const row = Math.floor(i / MATRIX_COLS);
    const col = i % MATRIX_COLS;
    let color = layer.color;
    let glow = 0.5;

    if (activeLayer === 0) {
      const hue = ((i / LED_COUNT) + time * 0.08) % 1;
      const rainbow = hsvToRgb(hue, 0.85, 0.9);
      color = mix(layer.color, rainbow, 0.6 + pulse * 0.25);
      glow = 0.45 + pulse * 0.5;
    } else if (activeLayer === 6 || selectorHeld) {
      color = mix("rgb(40,0,0)", "rgb(255,70,70)", 0.45 + fastPulse * 0.55);
      glow = 0.35 + fastPulse * 0.65;
    } else {
      const local = (Math.sin(time * 2.6 - i * 0.75) + 1) / 2;
      color = mix("rgb(18,18,24)", layer.color, 0.4 + local * 0.6);
      glow = 0.3 + local * 0.55;
    }

    if (pressedIndex === i) {
      color = mix(color, "rgb(255,255,255)", 0.45);
      glow = 1;
    }

    if (row === col && activeLayer === 5) {
      color = mix(color, "rgb(255,190,80)", 0.2 + pulse * 0.2);
    }

    leds.push({ color, glow: glow * (0.35 + brightness * 0.65) });
  }

  return leds;
}

function OledPreview({ activeLayer, lastAction, coord, selectorHeld }) {
  const layer = LAYERS[activeLayer];
  return (
    <div className="rounded-2xl border border-zinc-700 bg-black p-4 shadow-inner">
      <div className="mx-auto w-full max-w-[280px] rounded-xl border border-zinc-800 bg-zinc-950 px-4 py-3 font-mono text-[11px] text-zinc-200">
        <div className="flex items-center justify-between border-b border-zinc-800 pb-2">
          <span>OLED</span>
          <span className="text-zinc-400">SSD1306 mock</span>
        </div>
        <div className="pt-2">
          <div className="text-xs text-zinc-500">ACTIVE LAYER</div>
          <div className="text-lg font-bold" style={{ color: layer.color }}>{selectorHeld ? "SELECT" : layer.name}</div>
        </div>
        <div className="mt-3 grid grid-cols-3 gap-1 text-center text-[10px]">
          {LAYERS.slice(0, 6).map((item, i) => (
            <div
              key={item.name}
              className={`rounded border px-1 py-1 ${i === activeLayer ? "border-zinc-300 text-white" : "border-zinc-800 text-zinc-500"}`}
            >
              {item.name}
            </div>
          ))}
          <div className={`rounded border px-1 py-1 ${activeLayer === 6 ? "border-zinc-300 text-white" : "border-zinc-800 text-zinc-500"}`}>SEL</div>
          <div className="rounded border border-zinc-900 px-1 py-1 text-zinc-700">·</div>
          <div className="rounded border border-zinc-900 px-1 py-1 text-zinc-700">·</div>
        </div>
        <div className="mt-3 text-xs text-zinc-500">LAST ACTION</div>
        <div className="truncate text-zinc-200">{lastAction}</div>
        <div className="mt-2 text-xs text-zinc-500">MATRIX COORD</div>
        <div className="text-zinc-200">{coord}</div>
      </div>
    </div>
  );
}

export default function RGBAnimationStylesPreview() {
  const [activeLayer, setActiveLayer] = useState(0);
  const [time, setTime] = useState(0);
  const [selectorHeld, setSelectorHeld] = useState(false);
  const [pressedIndex, setPressedIndex] = useState(null);
  const [brightness, setBrightness] = useState(0.82);
  const [lastAction, setLastAction] = useState("Boot complete");
  const [coord, setCoord] = useState("r0 c0");
  const [log, setLog] = useState(["System ready"]);

  useEffect(() => {
    let frame;
    let last = performance.now();
    const loop = (now) => {
      const dt = (now - last) / 1000;
      last = now;
      setTime((t) => t + dt);
      frame = requestAnimationFrame(loop);
    };
    frame = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(frame);
  }, []);

  function pushLog(entry) {
    setLog((prev) => [entry, ...prev].slice(0, 10));
  }

  function pressKey(index) {
    const row = Math.floor(index / MATRIX_COLS);
    const col = index % MATRIX_COLS;
    const label = keyLabelFromIndex(index, activeLayer);
    setPressedIndex(index);
    setCoord(`r${row} c${col}`);
    setLastAction(`Key ${label}`);
    pushLog(`PRESS ${LAYERS[activeLayer].name} · r${row} c${col} · ${label}`);

    if (LAYERS[activeLayer].name === "SELECT" && label.startsWith("TO ")) {
      const target = Number(label.replace("TO ", ""));
      setActiveLayer(target);
      setLastAction(`Layer changed to ${LAYERS[target].name}`);
      pushLog(`LAYER -> ${LAYERS[target].name}`);
    }

    setTimeout(() => setPressedIndex((current) => (current === index ? null : current)), 180);
  }

  function turnEncoder(direction) {
    if (selectorHeld) {
      const next = wrapLayer(activeLayer + (direction === "cw" ? 1 : -1));
      setActiveLayer(next);
      setLastAction(`Selector ${direction === "cw" ? "next" : "prev"} -> ${LAYERS[next].name}`);
      pushLog(`ENC ${direction.toUpperCase()} · LAYER -> ${LAYERS[next].name}`);
      return;
    }

    const action = LAYERS[activeLayer].encoder[direction];
    setLastAction(`Encoder ${direction.toUpperCase()} · ${action}`);
    setCoord("enc");
    pushLog(`ENC ${direction.toUpperCase()} · ${LAYERS[activeLayer].name} · ${action}`);

    if (activeLayer === 5) {
      setBrightness((b) => clamp(b + (direction === "cw" ? 0.06 : -0.06), 0.08, 1));
    }
  }

  const leds = useMemo(
    () => buildLedColors(activeLayer, time, brightness, pressedIndex, selectorHeld),
    [activeLayer, time, brightness, pressedIndex, selectorHeld]
  );

  const currentLayer = LAYERS[activeLayer];

  return (
    <div className="min-h-screen bg-zinc-950 text-zinc-100 p-6">
      <div className="mx-auto max-w-7xl space-y-6">
        <div className="flex flex-col gap-3 md:flex-row md:items-end md:justify-between">
          <div>
            <h1 className="text-3xl font-bold tracking-tight">9key Makro Master Simulator</h1>
            <p className="mt-1 text-zinc-400">3×3 Matrix · RP2040 · 1 Encoder · 9 WS2812 LEDs · OLED mockup</p>
          </div>
          <div className="flex flex-wrap gap-2">
            <Badge variant="secondary" className="rounded-xl px-3 py-1">Rows GP2 GP3 GP4</Badge>
            <Badge variant="secondary" className="rounded-xl px-3 py-1">Cols GP5 GP6 GP7</Badge>
            <Badge variant="secondary" className="rounded-xl px-3 py-1">Encoder GP9/GP10</Badge>
            <Badge variant="secondary" className="rounded-xl px-3 py-1">RGB GP13</Badge>
          </div>
        </div>

        <div className="grid gap-6 xl:grid-cols-[1.2fr_0.8fr]">
          <Card className="rounded-3xl border-zinc-800 bg-zinc-900/80 shadow-2xl">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-2xl">
                <Keyboard className="h-5 w-5" />
                Live keypad
              </CardTitle>
            </CardHeader>
            <CardContent className="space-y-6">
              <div className="grid gap-4 lg:grid-cols-[1fr_280px]">
                <div className="rounded-3xl border border-zinc-800 bg-zinc-950 p-5">
                  <div className="mb-4 flex items-center justify-between gap-4">
                    <div>
                      <div className="text-xs uppercase tracking-[0.22em] text-zinc-500">Current layer</div>
                      <div className="text-2xl font-bold" style={{ color: currentLayer.color }}>{currentLayer.name}</div>
                      <div className="text-sm text-zinc-400">{selectorHeld ? "Selector held: encoder changes layer" : currentLayer.sub}</div>
                    </div>
                    <div className="text-right text-sm text-zinc-400">
                      <div>Brightness</div>
                      <div className="font-mono text-zinc-100">{Math.round(brightness * 100)}%</div>
                    </div>
                  </div>

                  <div className="grid grid-cols-3 gap-4">
                    {Array.from({ length: 9 }).map((_, index) => {
                      const label = keyLabelFromIndex(index, activeLayer);
                      const led = leds[index];
                      return (
                        <motion.button
                          key={index}
                          whileTap={{ scale: 0.95 }}
                          onClick={() => pressKey(index)}
                          className="group relative overflow-hidden rounded-[28px] border border-zinc-800 bg-zinc-900 p-4 text-left transition-transform"
                          style={{ boxShadow: `0 0 ${10 + led.glow * 28}px ${led.color}` }}
                        >
                          <div
                            className="absolute inset-x-3 top-3 h-2 rounded-full"
                            style={{ background: led.color, opacity: 0.95 }}
                          />
                          <div className="pt-4">
                            <div className="text-[10px] uppercase tracking-[0.18em] text-zinc-500">K{index + 1}</div>
                            <div className="mt-2 text-sm font-semibold leading-tight text-zinc-100">{label}</div>
                            <div className="mt-3 text-[11px] text-zinc-500">r{Math.floor(index / 3)} c{index % 3}</div>
                          </div>
                          <div className="pointer-events-none absolute inset-0 rounded-[28px] border border-white/5" />
                        </motion.button>
                      );
                    })}
                  </div>
                </div>

                <div className="space-y-4">
                  <div className="rounded-3xl border border-zinc-800 bg-zinc-950 p-4">
                    <div className="mb-3 flex items-center gap-2 text-sm font-semibold text-zinc-300">
                      <Disc3 className="h-4 w-4" /> Encoder
                    </div>
                    <div className="grid grid-cols-2 gap-3">
                      <Button className="h-14 rounded-2xl" variant="secondary" onClick={() => turnEncoder("ccw")}>
                        <RotateCcw className="mr-2 h-4 w-4" /> CCW
                      </Button>
                      <Button className="h-14 rounded-2xl" variant="secondary" onClick={() => turnEncoder("cw")}>
                        <RotateCw className="mr-2 h-4 w-4" /> CW
                      </Button>
                    </div>
                    <Button
                      className={`mt-3 h-14 w-full rounded-2xl ${selectorHeld ? "ring-2 ring-red-400" : ""}`}
                      variant={selectorHeld ? "default" : "secondary"}
                      onMouseDown={() => {
                        setSelectorHeld(true);
                        setLastAction("Selector held");
                        setCoord("enc btn");
                        pushLog("ENC BTN HOLD · SELECTOR ACTIVE");
                      }}
                      onMouseUp={() => {
                        setSelectorHeld(false);
                        setLastAction("Selector released");
                        pushLog("ENC BTN RELEASE");
                      }}
                      onMouseLeave={() => setSelectorHeld(false)}
                      onClick={() => {
                        setSelectorHeld((v) => !v);
                      }}
                    >
                      <MousePointerClick className="mr-2 h-4 w-4" />
                      {selectorHeld ? "Selector active" : "Encoder button / layer selector"}
                    </Button>
                  </div>

                  <OledPreview activeLayer={activeLayer} lastAction={lastAction} coord={coord} selectorHeld={selectorHeld} />
                </div>
              </div>
            </CardContent>
          </Card>

          <div className="space-y-6">
            <Card className="rounded-3xl border-zinc-800 bg-zinc-900/80 shadow-2xl">
              <CardHeader>
                <CardTitle className="flex items-center gap-2 text-xl">
                  <Layers3 className="h-5 w-5" /> Layers
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="grid grid-cols-1 gap-3">
                  {LAYERS.map((layer) => (
                    <button
                      key={layer.id}
                      onClick={() => {
                        setActiveLayer(layer.id);
                        setLastAction(`Layer set to ${layer.name}`);
                        pushLog(`MANUAL LAYER -> ${layer.name}`);
                      }}
                      className={`rounded-2xl border px-4 py-3 text-left transition ${activeLayer === layer.id ? "border-zinc-300 bg-zinc-950" : "border-zinc-800 bg-zinc-950/50 hover:bg-zinc-950"}`}
                    >
                      <div className="flex items-center justify-between gap-3">
                        <div>
                          <div className="font-semibold" style={{ color: layer.color }}>{layer.name}</div>
                          <div className="text-xs text-zinc-500">{layer.sub}</div>
                        </div>
                        <div className="h-4 w-4 rounded-full" style={{ background: layer.color }} />
                      </div>
                    </button>
                  ))}
                </div>
              </CardContent>
            </Card>

            <Card className="rounded-3xl border-zinc-800 bg-zinc-900/80 shadow-2xl">
              <CardHeader>
                <CardTitle className="text-xl">Debug log</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="rounded-2xl border border-zinc-800 bg-zinc-950 p-3 font-mono text-xs text-zinc-300">
                  <div className="mb-2 text-zinc-500">last_action = {lastAction}</div>
                  <div className="mb-3 text-zinc-500">coord = {coord}</div>
                  <div className="space-y-2">
                    {log.map((entry, i) => (
                      <div key={`${entry}-${i}`} className="rounded-lg border border-zinc-900 bg-zinc-900/60 px-2 py-2">
                        {entry}
                      </div>
                    ))}
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </div>
      </div>
    </div>
  );
}
