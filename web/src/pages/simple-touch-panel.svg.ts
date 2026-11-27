// The Simple Touch panel drawing, (c) Roey Tsemah / Synthux Academy, adapted
// by Fabio Bartali, used with permission. Served as one file, cached, in the
// faceplate's colours: the drawing reads them from these custom properties.
import panel from "../assets/simple-touch-panel.svg?raw";
import { COLOUR } from "../lib/tokens";

const c = COLOUR.black;
const style = `<style>:root{--pbg:${c.bg};--pline:${c.accent};--pshape:${c.bg};--pmid:${c.well};--pctl:${c.well};--pctlline:${c.accent};--ptex:${c.tan};--ptexop:.25;--psw:${c.accent};--pswop:.35}</style>`;
const svg = panel.replace(/(<svg[^>]*>)/, `$1${style}`);

export const GET = () => new Response(svg, { headers: { "Content-Type": "image/svg+xml" } });
