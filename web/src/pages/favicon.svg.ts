// The icon: the hourglass glyph on black, from the tokens
import { COLOUR, matrix } from "../lib/tokens";

// Dots larger than the logo's, to read at 32 px
// One stroke: too small for the wordmark's bold
const glyph = matrix(["hourglass"], COLOUR.black.accent, 'x="1.75" y="0.5" width="5" height="7"', 0.8, false);
const svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 8.5 8"><rect width="8.5" height="8" rx="1.4" fill="${COLOUR.black.bg}"/>${glyph}</svg>`;

export const GET = () => new Response(svg, { headers: { "Content-Type": "image/svg+xml" } });
