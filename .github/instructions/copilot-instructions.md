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
- The extension is a PostgreSQL server extension. To call C++ code from SQL you must expose a PG_FUNCTION (see `pg_ask.cpp` and `PG_FUNCTION_INFO_V1(pg_gen_query)` and `PG_FUNCTION_INFO_V1(pg_gen_execute)`).
- Use `cstring_to_text()` to convert returned `std::string`/`char*` results to `text*` for `PG_RETURN_TEXT_P()`.
- Prefer building the SDK client (ai::Client) inside C++ code (no need to export it with `extern "C"`); wrap only the minimal PG-facing function in `extern "C"`.

Configuration
- The extension reads the API key from the `PG_ASK_AI_KEY` environment variable (never store in postgresql.conf for security).
- Model and endpoint are configurable via PostgreSQL GUC (Grand Unified Configuration) variables:
  - `pg_ask.model` (default: empty string, which triggers ai_engine to use OpenAI's default model gpt-4o) - The AI model to use
  - `pg_ask.endpoint` (default: empty string, which triggers ai_engine to use OpenAI's endpoint https://api.openai.com/v1) - The API endpoint URL
- These GUC variables are defined in `_PG_init()` using `DefineCustomStringVariable()` with `PGC_USERSET` context and empty string defaults.
- Users can set these per-session (`SET pg_ask.model = '...'`), in postgresql.conf, or at database/user level (`ALTER DATABASE/USER SET ...`).
- The C++ code reads these values from the static `char*` variables (`pg_ask_model`, `pg_ask_endpoint`) set by PostgreSQL's GUC system.
- When both model and endpoint are empty/unset, the `ai_engine::make_engine()` function uses OpenAI defaults (gpt-4o model and https://api.openai.com/v1 endpoint).

Extension functions
- `pg_gen_query(text) RETURNS text`: Generates SQL from a natural-language query and returns it as text without executing it.
- `pg_gen_execute(text) RETURNS refcursor`: Generates SQL from a natural-language query and executes it, returning a cursor reference for result fetching.
  - Implemented in PL/pgSQL (not C++), wraps `pg_gen_query` and executes the generated SQL.
  - Returns a refcursor named `'ai_query_result'` that users can fetch from within a transaction block.
  - Usage pattern: `BEGIN; SELECT pg_gen_execute('query'); FETCH ALL FROM ai_query_result; COMMIT;`
  - The cursor approach provides memory efficiency for large result sets and doesn't require predefined column definitions.
  - Users can navigate cursors with FETCH NEXT, FETCH n, FETCH ALL, FETCH FORWARD, FETCH BACKWARD, etc.
  - Cursors are automatically closed on COMMIT/ROLLBACK.

C++ implementation notes
- Only `pg_gen_query` is implemented in C++ (`src/pg_ask.cpp`). It uses SPI for schema inspection and the AI SDK for SQL generation.
- `pg_gen_execute` is a PL/pgSQL wrapper function defined in `pg/pg_ask--1.0.sql` that calls `pg_gen_query`, then opens a cursor with `OPEN cursor FOR EXECUTE generated_sql`.
- If modifying the C++ code, focus on `pg_gen_query`. The SRF/SPI integration for direct execution is not used in the current design.

Developer workflow & quality gates
- Build inside the same Postgres version you plan to run (header/API differences exist across PG versions). The Dockerfile in `docker/` can help pin the correct server/devel headers.
- Quick tests:
	- Build the extension: `make` or your CMake-based build steps from the Dockerfile.
	- Load into Postgres and run: `CREATE EXTENSION pg_ask; SELECT pg_gen_query('...');`
- If you change public behavior, add a minimal test or example SQL in `pg/`.

Contact points
- If changes to the SDK usage are required, update `include/ai_engine.h` and `src/ai_engine.cpp`.
- If you add new server-facing functions, update `pg/pg_ask--1.0.sql` accordingly.

