# FormFactor Rust Bridge

This crate is a small native helper layer for FormFactor.

Its job is to make frontend snapshots deterministic and safe to hand to the authoritative C++ engineering core. It does **not** replace the C++ core and it does **not** make electrical pass/fail decisions.

Current responsibilities:

- stable component ordering;
- unambiguous snapshot records;
- fail-closed `Unknown` behavior when no authoritative core result exists;
- unit tests for deterministic replay behavior.

Run the Rust tests with:

```bash
cd rust/formfactor_bridge
cargo test
```
