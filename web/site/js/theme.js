// Theme: auto follows the system, light and dark override it. The button
// cycles through them. As on the family's sites.
const THEMES = ["auto", "light", "dark"];
const THEME_COLOR = { light: "#f2f2f0", dark: "#0b0b0c" };
const button = document.getElementById("theme");
document.getElementById("year").textContent = new Date().getFullYear();

function apply(theme) {
  if (theme === "auto") delete document.documentElement.dataset.theme;
  else document.documentElement.dataset.theme = theme;
  for (const meta of document.querySelectorAll('meta[name="theme-color"]')) meta.content = THEME_COLOR[theme] ?? meta.dataset.auto;
  button.dataset.state = theme;
  button.setAttribute("aria-label", "Theme: " + theme);
}

apply(document.documentElement.dataset.theme ?? "auto");
button.addEventListener("click", () => {
  const theme = THEMES[(THEMES.indexOf(button.dataset.state) + 1) % THEMES.length];
  apply(theme);
  try {
    localStorage.setItem("theme", theme);
  } catch {}
});
