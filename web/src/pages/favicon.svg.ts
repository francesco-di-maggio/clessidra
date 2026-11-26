// The icon: the hourglass glyph on black, from the tokens
import { COLOUR, matrix } from "../lib/tokens";

const glyph = matrix(["hourglass"], COLOUR.black.accent, 'x="2" y="1" width="5" height="7"');
const svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 9 9"><rect width="9" height="9" rx="1.6" fill="${COLOUR.black.bg}"/>${glyph}</svg>`;

export const GET = () => new Response(svg, { headers: { "Content-Type": "image/svg+xml" } });
