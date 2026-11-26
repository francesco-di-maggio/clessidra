import { defineConfig } from "astro/config";

export default defineConfig({
  site: "https://francescodimaggio.nl",
  base: "/clessidra",
  trailingSlash: "always",
  devToolbar: { enabled: false },
  // The faceplate art, from the repo's faceplate/
  vite: { server: { fs: { allow: [".."] } } },
});
