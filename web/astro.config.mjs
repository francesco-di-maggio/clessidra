import { defineConfig } from "astro/config";

export default defineConfig({
  site: "https://francescodimaggio.nl",
  base: "/clessidra",
  trailingSlash: "always",
  devToolbar: { enabled: false },
});
