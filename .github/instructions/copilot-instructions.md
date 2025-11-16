---
applyTo: '**'
---
This repository builds a PostgreSQL extension (shared object) that integrates an AI SDK (ai-sdk-cpp) to generate SQL from natural-language prompts.

Key project layout and conventions
- `src/` -- C++ source files that implement the extension entry point and helpers (server-side code compiled into the extension).
- `include/` -- project headers exposed to `src/` (and installed headers, if you add an install step). Put `ai_engine.h` and `explorer.h` here.
- `third_party/` -- external libraries (ai-sdk-cpp is included here). The third-party library should expose headers in `third_party/include/` so CMake can add that directory to the include path.
- `pg/` -- PostgreSQL extension metadata: `pg_ask.control` and `pg_ask--1.0.sql` (SQL objects to install the extension).
- `docker/` -- Dockerfile(s) used to build/test the extension in a containerized Postgres environment.
- `docker-compose.yaml` -- helper to run Postgres + pgAdmin for manual testing; sensitive values (like API keys) should be provided via an `.env` file.

Coding & API guidelines for contributors
- Prefer function-based C++ helpers for catalog inspection (e.g. `buildDatabaseMap()`, `formatSchema()` in `explorer.cpp`). Keep PostgreSQL catalog APIs inside `extern "C" { ... }` only where needed. Higher-level ai-engine code can be normal C++.
- Keep server-only code in `src/` and do not depend on libc++ features that conflict with PostgreSQL build flags. Use PostgreSQL memory allocation (palloc/pfree) when returning char* into PG functions.
- When calling catalog APIs after performing DDL in the same transaction, ensure you make the changes visible with `InvalidateSystemCaches()` and `CommandCounterIncrement()`, and use `GetLatestSnapshot()` for scans if you need to see newly committed state.

AI engine integration
- Put a small C++ wrapper in `include/ai_engine.h` and `src/ai_engine.cpp` exposing a clean API such as:
	- `Engine make_engine(const std::string &api_key, const std::string &model, const std::string &base_url)`
	- `std::string build_prompt(const std::string &database_schema)`
	- `std::string generate_sql(Engine &eng, const std::string &prompt, const std::string &user_query)`
- Use the ai-sdk-cpp library placed in `third_party/ai-sdk-cpp`. Add `third_party/ai-sdk-cpp/include` to the CMake target include directories and link the SDK library in `target_link_libraries`.
- Add a compile-time option / preprocessor macro such as `USE_AI_SDK` to guard SDK-specific code during development and CI. If you always have the SDK available, you can omit the guards and call the SDK directly.

Building and CMake notes
- Ensure `include/` and `third_party/ai-sdk-cpp/include` are added via `target_include_directories(your_target PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/third_party/ai-sdk-cpp/include)`.
- Link the ai-sdk-cpp target (or library) in `target_link_libraries(your_target PUBLIC ai-sdk-cpp)`; if you vendor the SDK as a subproject, use `add_subdirectory(third_party/ai-sdk-cpp)` and link the created target.
- Define `-DUSE_AI_SDK` in `target_compile_definitions(your_target PRIVATE USE_AI_SDK)` if you want to enable SDK code paths.

Running locally (docker)
- Create a `.env` file next to `docker-compose.yaml` with a line: `API_KEY=your_real_api_key`.
- `docker-compose` automatically loads `.env`. The compose file substitutes `${API_KEY}` and passes it into the container as `PG_ASK_AI_KEY`.

Postgres extension runtime notes
- The extension is a PostgreSQL server extension. To call C++ code from SQL you must expose a PG_FUNCTION (see `pg_ask.cpp` and `PG_FUNCTION_INFO_V1(pg_gen_query)`).
- Use `cstring_to_text()` to convert returned `std::string`/`char*` results to `text*` for `PG_RETURN_TEXT_P()`.
- Prefer building the SDK client (ai::Client) inside C++ code (no need to export it with `extern "C"`); wrap only the minimal PG-facing function in `extern "C"`.

Developer workflow & quality gates
- Build inside the same Postgres version you plan to run (header/API differences exist across PG versions). The Dockerfile in `docker/` can help pin the correct server/devel headers.
- Quick tests:
	- Build the extension: `make` or your CMake-based build steps from the Dockerfile.
	- Load into Postgres and run: `CREATE EXTENSION pg_ask; SELECT pg_gen_query('...');`
- If you change public behavior, add a minimal test or example SQL in `pg/`.

Contact points
- If changes to the SDK usage are required, update `include/ai_engine.h` and `src/ai_engine.cpp`.
- If you add new server-facing functions, update `pg/pg_ask--1.0.sql` accordingly.

