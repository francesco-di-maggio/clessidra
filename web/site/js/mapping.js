// Mapping: the panel and a table of what each control does, both from
// panel.js. Pointing at a row lights its control, and the reverse.

import { CONTROLS, LED, Panel } from "./panel.js";

const table = document.querySelector("#controls tbody");
const panel = new Panel(document.getElementById("panel"), { onFocus: (c) => show(c.id) });
for (const c of CONTROLS) panel.draw(c.id, c.type === "pad" ? 0 : c.initial);

// Slices share one row
const rows = [];
for (const c of CONTROLS) {
  if (c.free) continue;
  if (c.slice > 0) continue;
  const ids = c.slice === 0 ? CONTROLS.filter((s) => s.slice !== undefined).map((s) => s.id) : [c.id];
  rows.push({ ids, label: c.slice === 0 ? "P03–P09" : c.id, name: c.slice === 0 ? "Slices" : c.name, does: c.does });
}
rows.push({ ids: CONTROLS.filter((c) => c.free).map((c) => c.id), label: "Others", name: "Free", does: "S09, P00–P02, P10 and P11: nothing yet." });

for (const r of rows) {
  const tr = document.createElement("tr");
  tr.dataset.id = r.ids.join(" ");
  tr.innerHTML = `<td></td><td></td><td></td>`;
  tr.children[0].textContent = r.label;
  tr.children[1].textContent = r.name;
  tr.children[2].textContent = r.does;
  tr.addEventListener("pointerenter", () => show(r.ids[0]));
  table.append(tr);
}

const led = document.querySelector("#led tbody");
for (const [state, meaning] of LED) {
  const tr = document.createElement("tr");
  tr.innerHTML = `<td></td><td></td>`;
  tr.children[0].textContent = state;
  tr.children[1].textContent = meaning;
  led.append(tr);
}

function show(id) {
  const row = [...table.rows].find((tr) => tr.dataset.id.split(" ").includes(id));
  for (const tr of table.rows) tr.classList.toggle("active", tr === row);
  const ids = row ? row.dataset.id.split(" ") : [id];
  for (const [key, { g }] of panel.parts) g.classList.toggle("active", ids.includes(key));
}
