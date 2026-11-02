export function url(path = "") {
  const base = import.meta.env.BASE_URL.replace(/\/$/, "");
  const clean = path.replace(/^\//, "");
  // Pages end in a slash, files keep their name
  const file = /\.[a-z0-9]+$/i.test(clean);
  return clean ? `${base}/${clean}${clean.endsWith("/") || file ? "" : "/"}` : `${base}/`;
}
