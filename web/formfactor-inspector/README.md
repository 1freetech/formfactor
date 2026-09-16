# FormFactor TypeScript Inspector

This small browser tool is a debug and evidence viewer for FormFactor frontend snapshots.

Its job is to make component state easy to inspect in a browser. It does **not** run the authoritative engineering rules and it never invents a passing result. When no C++ engineering result is supplied, the inspector displays `UNKNOWN`.

## Build

```bash
cd web/formfactor-inspector
npm install
npm run typecheck
npm run build
```

Then serve this directory with any simple local static web server and open `index.html`.

## Why TypeScript

TypeScript is useful here because browser tooling, snapshot viewers, component catalog inspection, and lightweight debug interfaces do not need to live inside the production Unity executable. Keeping those tools separate makes the main game smaller while still giving developers a fast way to inspect state.
