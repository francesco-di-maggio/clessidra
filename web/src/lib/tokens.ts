// Colours, sizes and glyphs: the only place they are set. Base.astro writes
// them as CSS variables, the canvases read those, the logo, icon and link
// image are drawn from the glyphs.

// Black: Home, Play and the short pages. Light: the reading pages, dark again
// where the system asks for it.
const BLACK = {
  bg: "#0b0b0c",
  surface: "#111111",
  well: "#1d1c1a",
  line: "#2e2c28",
  ink: "#ecebe4",
  dim: "#a09d93",
  accent: "#d6a548",
  twin: "#c4a978",
};
const LIGHT = {
  bg: "#f7f5ef",
  surface: "#faf9f5",
  well: "#e6e3d8",
  line: "#d6d2c4",
  ink: "#111111",
  dim: "#5c5950",
  accent: "#8a6a1f",
  twin: "#8a6a1f",
};
export const COLOUR = { black: BLACK, light: LIGHT };

// Two sizes, 1.25 apart. Labels in capitals, tracked.
export const SIZE = { text: 16, title: 20, line: 24, titleLine: 28, track: "0.1em", radius: 4 };
export const FONT = `"Barlow Semi Condensed", system-ui, sans-serif`;

// Dot matrix, 7 rows, # a dot. A dot is DOT of its pitch, one empty column
// between letters.
export const DOT = 0.6;
export const GLYPHS: Record<string, string[]> = {
  C: [".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."],
  L: ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
  E: ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
  S: [".###.", "#...#", "#....", ".###.", "....#", "#...#", ".###."],
  I: ["#", "#", "#", "#", "#", "#", "#"],
  D: ["####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."],
  R: ["####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"],
  A: [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
  // The icon: a glass, sand fallen below
  hourglass: ["#####", "#...#", ".#.#.", "..#..", ".#.#.", "#.#.#", "#####"],
};

// The dots of a word or glyph, in pitch units: [x, y] of each centre, and
// the size
export function dots(chars: string[]) {
  const out: [number, number][] = [];
  let x = 0;
  chars.forEach((c, i) => {
    const g = GLYPHS[c];
    g.forEach((row, y) => [...row].forEach((d, k) => d === "#" && out.push([x + k + 0.5, y + 0.5])));
    x += g[0].length + (i < chars.length - 1 ? 1 : 0);
  });
  return { dots: out, width: x, height: 7 };
}

// An SVG of dots, filled with currentColor unless a colour is given
export function matrix(chars: string[], fill = "currentColor", attrs = "") {
  const { dots: d, width, height } = dots(chars);
  const r = DOT / 2;
  const circles = d.map(([x, y]) => `<circle cx="${x}" cy="${y}" r="${r}"/>`).join("");
  return `<svg viewBox="0 0 ${width} ${height}" fill="${fill}" ${attrs}>${circles}</svg>`;
}

const vars = (c: typeof BLACK) => Object.entries(c).map(([k, v]) => `--${k}:${v};`).join("");

// The variables: black at the root, light on .reading, dark on .reading for
// a dark system, light whenever printed
export function css() {
  const s = SIZE;
  return [
    `:root{${vars(BLACK)}--sans:${FONT};--text:${s.text}px;--title:${s.title}px;--line-text:${s.line}px;--line-title:${s.titleLine}px;--track:${s.track};--radius:${s.radius}px;color-scheme:dark}`,
    `.reading{${vars(LIGHT)}color-scheme:light}`,
    `@media (prefers-color-scheme:dark){.reading{${vars(BLACK)}color-scheme:dark}}`,
    `@media print{:root,.reading{${vars(LIGHT)}color-scheme:light}}`,
  ].join("\n");
}
