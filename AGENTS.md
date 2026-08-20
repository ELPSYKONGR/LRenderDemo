# LRenderDemo Agent Rules

These project rules adapt the supplied Engineering Standard v2.1 to C++/CMake.

1. Read `FILE_INDEX.md` and the top of `CHANGELOG.md` before changing code.
2. Use the dependency diagram to identify affected modules.
3. Keep production source files near 300 lines and tests below 400 lines.
4. Place temporary diagnostics in `scratch/`; runtime diagnostics belong in `logs/`.
5. Do not hardcode absolute paths, credentials, API keys, or machine-specific SDK locations.
6. Use `ComPtr` for COM ownership and RAII/value ownership elsewhere.
7. Every failure must add context, be logged at the application boundary, or be intentionally
   recoverable. Empty catch blocks are forbidden except the documented fatal-log fallback.
8. Run `cmake --build --preset vs2022-debug` and `ctest --preset vs2022-debug` before committing.
9. Update `FILE_INDEX.md`, `CHANGELOG.md`, `LESSONS.md`, and `SKILLS_USED.md` when applicable.
10. Work on `dev` or `feat/*`; do not merge or push to `main` without explicit user approval.
