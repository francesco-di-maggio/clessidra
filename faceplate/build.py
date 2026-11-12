"""Clessidra faceplate for the Synthux Simple Touch, in the web page's style:
black, white, grey, one orange. On Synthux's faceplate template (its DXF,
inches, 72 pt each): the outline (outline.json, its spline evaluated) and
every opening's centre. From the repo: python3 faceplate/build.py, writes
faceplate/clessidra-faceplate.svg."""
import json
import math
import sys
from pathlib import Path

here = Path(__file__).parent
out = here
outline = [tuple(p) for p in json.loads((here / "outline.json").read_text())]

W, H = 244.0, 205.0

# The site's dark theme (tokens.css)
BG, INK, DIM, LINE, ACCENT = "#0b0b0c", "#f2f2f2", "#828288", "#3a3a3f", "#ff5a1f"
MONO = "'IBM Plex Mono', 'SF Mono', Menlo, monospace"

# Openings, pt, centres from the DXF (F.Mask)
JACK_R, KNOB_R = 11.36, 12.77  # the template's openings
# The holes in them, for the usual parts: a 3.5 mm jack (Thonkiconn) 6 mm,
# a pot's M7 bushing 7 mm. To confirm with Synthux.
JACK_HOLE, KNOB_HOLE = 3.0 / 25.4 * 72, 3.5 / 25.4 * 72
JACK_RING = JACK_HOLE + 1.6
JACKS = {"IN": (21.92, 27.0), "OUT": (21.92, 70.2)}
KNOBS = {"S31": (56.22, 110.62), "S32": (100.2, 110.62), "S33": (144.2, 110.62), "S34": (188.17, 110.62),
         "S30": (56.22, 159.22), "S35": (188.17, 159.22)}
SLOTS = {"S36": (21.97, 105.1, 178.92), "S37": (222.36, 105.1, 178.92)}
SLOT_W = 12.2
# Screws above and below the faders, and at the foot: their drill unknown,
# drawn as a small hole in a ring, clear of the slots
# and the plate's own three: top right and the two at its foot
SCREWS = [(21.97, 98.1), (21.97, 185.93), (222.36, 98.1), (222.36, 185.93), (44.92, 191.7), (196.76, 191.7), (222.04, 82.28)]
SCREW_HOLE, SCREW_RING = 3.2, 4.6

NAMES = {"S31": ("SIZE", "RND SIZE"), "S32": ("SPRAY", "REVERSE"), "S33": ("PITCH", "RND PITCH"),
         "S34": ("SHAPE", "RND SHAPE"), "S30": ("POSITION", "FILTER"), "S35": ("LENGTH", "REVERB"),
         "S36": ("SPEED", "VOLUME"), "S37": ("DENSITY", "RND TIME")}
CENTRED = {"S31", "S33"}  # a tick at their centre, as on the page
# Each dial's range at the ends of its track, as the faders' scales
RANGES = {"S31": ("5ms", "2s"), "S32": ("0", "LOOP"), "S33": ("-24", "+24"), "S34": ("SQ", "REV"),
          "S30": ("0", "END"), "S35": ("0", "ALL")}

NAME_PT, TWIN_PT, MARK_PT = 4.0, 3.2, 2.9
FADER_PT, FADER_TWIN_PT = 5.0, 4.0
TRACK = 0.12  # em


def poly(points):
    return "M" + "L".join(f"{x:.3f} {y:.3f}" for x, y in points) + "Z"


def circle(cx, cy, r):
    return f"M{cx - r:.3f} {cy:.3f}a{r:.3f} {r:.3f} 0 1 0 {2 * r:.3f} 0a{r:.3f} {r:.3f} 0 1 0 {-2 * r:.3f} 0Z"


def slot(x, top, bottom, w):
    r = w / 2
    return f"M{x - r:.3f} {top + r:.3f}a{r:.3f} {r:.3f} 0 0 1 {w:.3f} 0V{bottom - r:.3f}a{r:.3f} {r:.3f} 0 0 1 {-w:.3f} 0Z"


def text(x, y, s, size, fill, anchor="middle", weight=500, rotate=None, track=TRACK):
    t = f' transform="rotate({rotate} {x:.2f} {y:.2f})"' if rotate is not None else ""
    return (f'<text x="{x:.2f}" y="{y:.2f}" font-size="{size}" font-weight="{weight}" fill="{fill}" '
            f'text-anchor="{anchor}" letter-spacing="{size * track:.2f}"{t}>{s}</text>')


def arc(cx, cy, r, a0, a1):
    p = lambda a: (cx + r * math.sin(math.radians(a)), cy - r * math.cos(math.radians(a)))
    (x0, y0), (x1, y1) = p(a0), p(a1)
    return f"M{x0:.3f} {y0:.3f}A{r} {r} 0 {1 if a1 - a0 > 180 else 0} 1 {x1:.3f} {y1:.3f}"


def inside(points, x, y):
    c = False
    for i in range(len(points)):
        (x0, y0), (x1, y1) = points[i], points[i - 1]
        if (y0 > y) != (y1 > y) and x < x0 + (x1 - x0) * (y - y0) / (y1 - y0):
            c = not c
    return c


holes = [circle(*c, JACK_HOLE) for c in JACKS.values()]
holes += [circle(*c, KNOB_HOLE) for c in KNOBS.values()]
# The faders' slots as narrow as the screw holes, their ring as wide as the
# screws' rings, as on Synthux's own plates
holes += [slot(x, t + 1.4, b - 1.4, 2 * SCREW_HOLE) for x, t, b in SLOTS.values()]
holes += [circle(*c, SCREW_HOLE) for c in SCREWS]

art = []

# Knobs: the page's dial, its track 270° round the opening, the name under it,
# the Shift function over it in orange
DIAL_R = KNOB_HOLE + 3.6  # the track close round the hole
defs = []
for k, (x, y) in KNOBS.items():
    name, twin = NAMES[k]
    art.append(f'<path d="{arc(x, y, DIAL_R, -135, 135)}" fill="none" stroke="{LINE}" stroke-width="1.6" stroke-linecap="round"/>')
    if k in CENTRED:
        art.append(f'<path d="M{x:.2f} {y - DIAL_R - 1.8:.2f}V{y - DIAL_R + 1.8:.2f}" stroke="{DIM}" stroke-width="0.6"/>')
    art.append(text(x, y + DIAL_R + 5.6, name, NAME_PT, INK))
    lo, hi = RANGES[k]
    e = DIAL_R * math.sin(math.radians(45))
    art.append(text(x - e - 0.6, y + e + 3.9, lo.replace("-", "−"), MARK_PT, DIM, track=0))
    art.append(text(x + e + 0.6, y + e + 3.9, hi, MARK_PT, DIM, track=0))
    # The Shift function curved along the dial's top, its own
    r = DIAL_R + 2.8
    pid = f"twin-{k}"
    defs.append(f'<path id="{pid}" d="{arc(x, y, r, -70, 70)}"/>')
    art.append(f'<text font-size="{TWIN_PT}" font-weight="500" fill="{ACCENT}" letter-spacing="{TWIN_PT * TRACK:.2f}">'
               f'<textPath href="#{pid}" startOffset="50%" text-anchor="middle">{twin}</textPath></text>')

# Jacks: a ring, the name beside
for k, (x, y) in JACKS.items():
    art.append(f'<path d="{circle(x, y, JACK_RING)}" fill="none" stroke="{LINE}" stroke-width="0.6"/>')
    art.append(text(x, y + JACK_RING + 5.6, k, NAME_PT, INK))

# Faders: the slot outlined, the page's scale on the inner side, the name and
# its Shift function along the outer edge
SPEEDS = ["0", "¼", "½", "1×", "2×", "4×"]
speed_at = [(0 if i == 0 else 5 if i == 5 else i + 1 / 6) / 5 for i in range(6)]
DENSITY = [(0.0, "0"), (1, "1"), (10, "10"), (100, "100"), (320, "320")]
none, lo, hi = 0.04, 0.25, 320.0
density_at = [0.0 if r == 0 else none + (1 - none) * math.log(r / lo) / math.log(hi / lo) for r, _ in DENSITY]
for k, (x, top, bottom) in SLOTS.items():
    name, twin = NAMES[k]
    left = x < W / 2
    art.append(f'<path d="{slot(x, top, bottom, 2 * SCREW_RING)}" fill="none" stroke="{LINE}" stroke-width="0.6"/>')
    run_top, run_bottom = top + 8.0, bottom - 8.0
    marks = list(zip(speed_at, SPEEDS)) if k == "S36" else list(zip(density_at, [l for _, l in DENSITY]))
    side = 1 if left else -1
    edge = x + side * (SCREW_RING + 2.0)
    for u, label in marks:
        y = run_bottom - u * (run_bottom - run_top)
        art.append(f'<path d="M{edge:.2f} {y:.2f}h{side * 1.8:.2f}" stroke="{DIM}" stroke-width="0.5"/>')
        art.append(text(edge + side * 2.6, y + MARK_PT * 0.35, label, MARK_PT, DIM, anchor="start" if left else "end", track=0))
    mid = (top + bottom) / 2
    ox = x - side * (SCREW_RING + 2.2)
    art.append(f'<text x="{ox:.2f}" y="{mid:.2f}" font-size="{FADER_PT}" font-weight="500" letter-spacing="{FADER_PT * TRACK:.2f}" '
               f'text-anchor="middle" transform="rotate({-90 if left else 90} {ox:.2f} {mid:.2f})">'
               f'<tspan fill="{INK}">{name}</tspan><tspan fill="{ACCENT}" font-size="{FADER_TWIN_PT}" dx="{3.6:.1f}">{twin}</tspan></text>')

# Screws: a ring round each, clear of the slots
for x, y in SCREWS:
    art.append(f'<path d="{circle(x, y, SCREW_RING)}" fill="none" stroke="{LINE}" stroke-width="0.6"/>')

NECK = 0.07
SAND_GRID, SAND_DOT = 1.7, 0.95


def glass_half(t, w):  # half the width at t of a bulb, 0 at the neck, 1 at the cap
    return w * (NECK + (1 - NECK) * (1 - (1 - t) ** 2))


def pts(points):
    return "M" + "L".join(f"{x:.2f} {y:.2f}" for x, y in points)


# The recording between Position and Length, level with them, as on the page:
# grey dots, attacks and their decays, the loop's brackets, the playhead in
# orange. Over the playhead the grain it plays: its envelope (Shape) as wide
# as Size, a mark for Pitch, the band Spray scatters it over, and the arrow of
# Speed under it.
GRID, ROW, DOT = 2.8, 2.2, 0.8
CY = KNOBS["S30"][1]
COLUMNS = 28
WX0 = (KNOBS["S30"][0] + KNOBS["S35"][0]) / 2 - (COLUMNS - 1) * 2.8 / 2
ATTACKS = (0, 6, 10, 16, 19, 25)
REACH = 3 * ROW + 2.0
level = 0.0
seed = 7
for i in range(COLUMNS):
    x = WX0 + i * GRID
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
    level = max(level * 0.62, 1.0 if i in ATTACKS else 0.0)
    n = int(min(level + 0.15 * (seed / 0x7FFFFFFF), 1.0) * 3)
    for j in range(-n, n + 1):
        art.append(f'<rect x="{x - DOT / 2:.2f}" y="{CY + j * ROW - DOT / 2:.2f}" width="{DOT}" height="{DOT}" fill="{DIM}" opacity="0.55"/>')
WX1 = WX0 + (COLUMNS - 1) * GRID
left, right = WX0 - 2.6, WX1 + 2.6
for bx, way in ((left, 1), (right, -1)):
    art.append(f'<path d="M{bx + way * 1.6:.2f} {CY - REACH:.2f}H{bx:.2f}V{CY + REACH:.2f}H{bx + way * 1.6:.2f}" fill="none" stroke="{DIM}" stroke-width="0.5"/>')
PX = WX0 + 10.5 * GRID
TOP, FOOT = CY - REACH, CY + REACH
SPRAY_W, SIZE_W = 13.0, 11.0
# The playhead, its head
art.append(f'<path d="M{PX:.2f} {TOP:.2f}V{FOOT:.2f}" stroke="{ACCENT}" stroke-width="0.6"/>')
art.append(f'<path d="M{PX - 1.4:.2f} {TOP - 1.8:.2f}H{PX + 1.4:.2f}L{PX:.2f} {TOP:.2f}Z" fill="{ACCENT}"/>')
# The hourglass above Size, as on the page: its top closed, its foot open,
# the sand pouring through. IN feeds it.
HX, HW = KNOBS["S31"][0], 14.0
HTOP = JACKS["IN"][1] - JACK_RING  # level with the top of IN
HBOT = HTOP + 44.0
hy = (HTOP + HBOT) / 2
hb = (HBOT - HTOP) / 2
steps = 24
side_l = [(HX - glass_half(1 - i / steps, HW), HTOP + hb * i / steps) for i in range(steps + 1)]
side_l += [(HX - glass_half(i / steps, HW), hy + hb * i / steps) for i in range(1, steps + 1)]
side_r = [(2 * HX - x, y) for x, y in side_l]
art.append(f'<path d="{pts(side_l)} {pts(side_r)}" fill="none" stroke="{INK}" stroke-width="0.8" stroke-linecap="round"/>')
art.append(f'<path d="M{HX - HW - 1.8:.2f} {HTOP:.2f}H{HX + HW + 1.8:.2f}" stroke="{INK}" stroke-width="1.0" stroke-linecap="round"/>')
REPOSE = 0.6
for j in range(-20, 21):
    y = hy + j * SAND_GRID
    for i in range(-10, 11):
        x = HX + i * SAND_GRID
        t = abs(y - hy) / hb
        if t >= 0.97 or t < 0.08 or abs(x - HX) > glass_half(t, HW) - 1.3:
            continue
        if y < hy:
            sand = y >= hy - hb * 0.55 + REPOSE * (HW - abs(x - HX)) * 0.5
        else:
            sand = y >= hy + hb * 0.6 + REPOSE * abs(x - HX)
        if sand:
            art.append(f'<rect x="{x - SAND_DOT / 2:.2f}" y="{y - SAND_DOT / 2:.2f}" width="{SAND_DOT}" height="{SAND_DOT}" fill="{ACCENT}"/>')
for k in range(0, 8):
    y = hy + k * SAND_GRID
    art.append(f'<rect x="{HX - SAND_DOT / 2:.2f}" y="{y - SAND_DOT / 2:.2f}" width="{SAND_DOT}" height="{SAND_DOT}" fill="{ACCENT}"/>')

# The sand falling from the open foot, straight, a little faster as it goes,
# into a small heap
for k in range(5):
    y = HBOT + 2.0 + 1.6 * k ** 1.25 * 1.3
    art.append(f'<rect x="{HX - SAND_DOT / 2:.2f}" y="{y - SAND_DOT / 2:.2f}" width="{SAND_DOT}" height="{SAND_DOT}" fill="{ACCENT}"/>')
HEAP_Y = HBOT + 17.0
for row in range(3):
    n = 3 - row
    for i in range(-n, n + 1):
        y = HEAP_Y - row * SAND_GRID
        art.append(f'<rect x="{HX + i * SAND_GRID - SAND_DOT / 2:.2f}" y="{y - SAND_DOT / 2:.2f}" width="{SAND_DOT}" height="{SAND_DOT}" fill="{ACCENT}"/>')

# IN into the glass, at IN's height
inx, iny = JACKS["IN"]
side_at = HX - glass_half(1 - (iny - HTOP) / ((HBOT - HTOP) / 2), HW)
art.append(f'<path d="M{inx + JACK_RING + 2.0:.2f} {iny:.2f}H{side_at - 2.0:.2f}" stroke="{DIM}" stroke-width="0.75" stroke-linecap="round" stroke-dasharray="0 1.6"/>')
art.append(f'<circle cx="{inx + JACK_RING + 2.0:.2f}" cy="{iny:.2f}" r="0.85" fill="{DIM}"/>')

# The wires, dotted as the page's dots, soft at their bends, a dot where each
# leaves its control
def wire(points, r=3.5):
    d = f"M{points[0][0]:.2f} {points[0][1]:.2f}"
    for (ax, ay), (bx, by), (cx_, cy_) in zip(points, points[1:], points[2:]):
        l1 = math.hypot(bx - ax, by - ay)
        l2 = math.hypot(cx_ - bx, cy_ - by)
        k1 = min(r, l1 / 2) / l1
        k2 = min(r, l2 / 2) / l2
        d += f"L{bx - (bx - ax) * k1:.2f} {by - (by - ay) * k1:.2f}Q{bx:.2f} {by:.2f} {bx + (cx_ - bx) * k2:.2f} {by + (cy_ - by) * k2:.2f}"
    d += f"L{points[-1][0]:.2f} {points[-1][1]:.2f}"
    art.append(f'<path d="{d}" fill="none" stroke="{DIM}" stroke-width="0.75" stroke-linecap="round" stroke-dasharray="0 1.6"/>')
    art.append(f'<circle cx="{points[0][0]:.2f}" cy="{points[0][1]:.2f}" r="0.85" fill="{DIM}"/>')


sx, _ = KNOBS["S30"]
lx, _ = KNOBS["S35"]
half_name = lambda n: NAME_PT * (0.6 + TRACK) * n / 2
LABEL_Y = KNOBS["S31"][1] + DIAL_R + 5.6
zx, rx, qx, hx_ = KNOBS["S31"][0], KNOBS["S32"][0], KNOBS["S33"][0], KNOBS["S34"][0]
dens_x = SLOTS["S37"][0]
spd_x, _, spd_bottom = SLOTS["S36"]
inx, iny = JACKS["IN"]
wire([(sx + DIAL_R + 2.0, CY), (left - 0.6, CY)])                                    # Position
wire([(lx - DIAL_R - 2.0, CY), (right + 0.6, CY)])                                   # Length
DENS_Y = LABEL_Y + 3.0
LOW = 184.6

# The name, low on the plate
PLATE_MID = (min(x for x, _ in outline) + max(x for x, _ in outline)) / 2
art.append(text(PLATE_MID, 193.0, "CLESSIDRA", 6.8, INK, weight=600, track=0.32))

svg = f'''<svg xmlns="http://www.w3.org/2000/svg" xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
     width="{W / 72 * 25.4:.2f}mm" height="{H / 72 * 25.4:.2f}mm" viewBox="0 0 {W} {H}" font-family="{MONO}">
  <title>Clessidra faceplate, Synthux Simple Touch</title>
  <desc>1:1, pt. Black plate, art in white, grey and orange (UV print over black mask). Outline and opening centres from Synthux's Simple Touch Faceplate template (DXF). Screw drills unknown: holes drawn 2.3 mm, to confirm.</desc>
  <g id="plate" inkscape:groupmode="layer" inkscape:label="Plate (black)">
    <path d="{poly(outline)} {' '.join(holes)}" fill="{BG}" fill-rule="evenodd"/>
  </g>
  <defs>{''.join(defs)}</defs>
  <g id="art" inkscape:groupmode="layer" inkscape:label="Art (white, grey, orange)">
{chr(10).join('    ' + a for a in art)}
  </g>
  <g id="cut" inkscape:groupmode="layer" inkscape:label="Cut (reference)" fill="none" stroke="#e33" stroke-width="0.25" opacity="0.6">
    <path d="{poly(outline)}"/>
  </g>
</svg>
'''
(out / "clessidra-faceplate.svg").write_text(svg)
print("ok")
