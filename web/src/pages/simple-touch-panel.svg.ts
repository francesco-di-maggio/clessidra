// The Simple Touch panel drawing, (c) Roey Tsemah / Synthux Academy, adapted
// by Fabio Bartali, used with permission. Served as one file, cached, in the
// faceplate's colours: the drawing reads them from these custom properties.
import panel from "../assets/simple-touch-panel.svg?raw";
import { COLOUR } from "../lib/tokens";

const c = COLOUR.black;
const style = `<style>:root{--pbg:${c.bg};--pline:${c.accent};--pshape:${c.bg};--pmid:${c.well};--pctl:${c.well};--pctlline:${c.accent};--ptex:${c.tan};--ptexop:.25;--psw:${c.accent};--pswop:.35}</style>`;
// Each pad's drawing and each switch's lever named, so the Controls page can
// light the very shape: found by where each control sits on the drawing
const SHAPES: [string, string][] = [
  ["P00", 'points="360.75 788.38 360.95 791'], ["P01", 'points="515.17 897.81 515.08 898'],
  ["P02", 'd="M825.69,1176.29l-.7-1.55'], ["P03", 'd="M276.3,1126.35l-.22-.61-'],
  ["P04", 'd="M451.29,1331.14l-.44-1.5'], ["P05", 'd="M599.32,1357.81l-1.07-4.'],
  ["P06", 'd="M841.25,1221.03l-.51-2.3'], ["P07", 'd="M984.88,1445.28l-.18-2.0'],
  ["P08", 'd="M504.88,1501.67l-1.2-2.2'], ["P09", 'd="M870.56,1452.02l-.28-.52'],
  ["P10", 'd="M535.22,160.99l-.23-2.35'], ["P11", 'd="M717.54,205.67l-.95-2.36'],
  ["S09", 'x="387.24" y="1027.24"'], ["S07", 'x="544.82" y="1099.75"'],
];
let svg = panel.replace(/(<svg[^>]*>)/, `$1${style}`);
for (const [id, at] of SHAPES) {
  if (svg.split(at).length !== 2) throw new Error(`simple-touch-panel.svg: ${id} not found once`);
  svg = svg.replace(at, `id="shape-${id}" ${at}`);
}

export const GET = () => new Response(svg, { headers: { "Content-Type": "image/svg+xml" } });
