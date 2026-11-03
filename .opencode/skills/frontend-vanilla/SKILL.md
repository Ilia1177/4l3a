---
name: frontend-vanilla
description: Standards for plain HTML, JavaScript and CSS frontend with no framework. Use when editing anything under public/.
---
- Semantic HTML (`main`, `nav`, `button`, `label` linked to inputs). Accessible: keyboard usable, alt text, visible focus.
- JavaScript: ES modules, `const`/`let`, strict equality, no globals, no inline `onclick`. Small modules: `api.js` (fetch calls), `state.js`, `ui.js`, `main.js`.
- All `fetch` calls go through `api.js`: handle non-2xx, network errors, and show user feedback.
- Never use `innerHTML` with dynamic data; use `textContent` or DOM APIs.
- CSS: custom properties for colors/spacing, mobile-first, flex/grid, BEM-like class names, no `!important`.
- File layout: `public/index.html`, `public/css/`, `public/js/`.
- Check with `make lint-frontend` (Biome). No build step required.
