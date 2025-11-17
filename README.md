# pg_ask - AI-Powered SQL Generation for PostgreSQL

`pg_ask` is a PostgreSQL extension that integrates AI capabilities directly into your database, allowing you to generate and execute SQL queries using natural language.

## Features

- **Natural Language to SQL**: Convert plain English questions into SQL queries
- **Two Operation Modes**:
  - `pg_gen_query()`: Generate SQL without executing (preview mode)
  - `pg_gen_execute()`: Generate and execute SQL, returning a cursor for result fetching
- **Schema-Aware**: Automatically inspects your database schema to generate contextually appropriate queries
- **AI Provider Support**: Works with OpenAI-compatible APIs (OpenAI, Groq, etc.)
- **Built for PostgreSQL 16+**: Native C++ extension for optimal performance

## Quick Start with Docker

The fastest way to try `pg_ask` is using our pre-built Docker image:

```bash
# Pull the image
docker pull ghcr.io/abiji-2020/pg_ask:latest

# Run with your API key
docker run -d \
  --name pg_ask \
  -e POSTGRES_PASSWORD=mysecretpassword \
  -e PG_ASK_AI_KEY=your_api_key_here \
  -p 5432:5432 \
  ghcr.io/abiji-2020/pg_ask:latest

# Connect and use
psql -h localhost -U postgres -d postgres
```

Once connected, the extension is already installed:

```sql
-- Generate SQL query (preview mode)
SELECT pg_gen_query('show me all users who registered this month');

-- Generate and execute SQL using cursor
BEGIN;
SELECT pg_gen_execute('list all tables in the database');
FETCH ALL FROM ai_query_result;
COMMIT;
```

## Installation

### Option 1: Using Docker (Recommended)

#### Using Docker Compose

Create a `.env` file:

```env
POSTGRES_PASSWORD=your_secure_password
PG_ASK_AI_KEY=your_api_key_here
```

Create `docker-compose.yaml`:

```yaml
version: '3.8'

services:
  postgres:
    image: ghcr.io/abiji-2020/pg_ask:latest
    environment:
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD}
      POSTGRES_DB: postgres
      POSTGRES_USER: postgres
      PG_ASK_AI_KEY: ${PG_ASK_AI_KEY}
    ports:
      - "5432:5432"
    volumes:
      - pgdata:/var/lib/postgresql/data

volumes:
  pgdata:
```

Start the service:

```bash
docker-compose up -d
```

### Option 2: Manual Installation from Binary (Linux only)

Download and install the pre-built extension:

```bash
# Download the installer script
curl -fsSL https://abiji-2020.github.io/pg_ask/install.sh -o install.sh
chmod +x install.sh

# Install the extension
sudo ./install.sh

# Restart PostgreSQL
sudo systemctl restart postgresql
```

**Note**: Set your API key as an environment variable before starting PostgreSQL (see Configuration section below).

### Option 3: Build from Source (Linux only)

#### Prerequisites

- **Operating System**: Linux (Ubuntu 20.04+, Debian 11+, or compatible)
- PostgreSQL 16+ (with development headers)
- CMake 3.15+
- C++20 compatible compiler (GCC 10+, Clang 11+)
- Git

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/Abiji-2020/pg_ask.git
cd pg_ask

# Initialize submodules (ai-sdk-cpp)
git submodule update --init --recursive

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake -DUSE_AI_SDK=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=20 \
      ..

# Build the extension
make -j$(nproc)

# Install (requires sudo)
sudo make install
```

#### Enable the Extension

```sql
-- Connect to your database
psql -U postgres -d your_database

-- Create the extension
CREATE EXTENSION pg_ask;

-- Verify installation
\dx pg_ask
```

## Configuration

### Environment Variables

The extension requires configuration via environment variables. **API keys should never be stored in `postgresql.conf` for security reasons.**

| Variable | Description | Required | Default |
|----------|-------------|----------|---------|
| `PG_ASK_AI_KEY` | API key for AI provider | Yes | - |
| `POSTGRES_PASSWORD` | PostgreSQL superuser password | Yes (Docker) | - |
| `POSTGRES_USER` | PostgreSQL username | No | `postgres` |
| `POSTGRES_DB` | Default database name | No | `postgres` |

### PostgreSQL Configuration Parameters

You can customize the AI model and endpoint using PostgreSQL configuration parameters:

| Parameter | Description | Default |
|-----------|-------------|---------|
| `pg_ask.model` | AI model to use for SQL generation | `gpt-4o` (OpenAI default) |
| `pg_ask.endpoint` | API endpoint URL for the AI provider | `https://api.openai.com/v1` (OpenAI) |

**Setting Configuration Parameters:**

```sql
-- Set for current session
SET pg_ask.model = 'gpt-4';
SET pg_ask.endpoint = 'https://api.openai.com/v1';

-- Use Groq instead
SET pg_ask.model = 'mixtral-8x7b-32768';
SET pg_ask.endpoint = 'https://api.groq.com/openai';

-- Set in postgresql.conf for persistent configuration
-- Add these lines to postgresql.conf:
-- pg_ask.model = 'gpt-3.5-turbo'
-- pg_ask.endpoint = 'https://api.openai.com/v1'

-- Set at database level
ALTER DATABASE mydb SET pg_ask.model = 'gpt-4';

-- Set at user level
ALTER USER myuser SET pg_ask.endpoint = 'https://api.groq.com/openai';
```

**Supported Models and Endpoints:**

- **OpenAI** (default): 
  - Endpoint: `https://api.openai.com/v1`
  - Models: `gpt-4o` (default), `gpt-4`, `gpt-3.5-turbo`
- **Groq**:
  - Endpoint: `https://api.groq.com/openai`
  - Models: `mixtral-8x7b-32768`, `llama-3.1-70b-versatile`
- **Azure OpenAI**:
  - Endpoint: Your Azure endpoint
  - Models: Your deployed model names
- **OpenRouter**:
  - Endpoint: `https://openrouter.ai/api/v1`
  - Models: Various models via OpenRouter

### Setting Environment Variables

#### For Docker:

Use a `.env` file with `docker-compose`:

```env
PG_ASK_AI_KEY=your_api_key_here
POSTGRES_PASSWORD=your_secure_password
```

Or pass directly in `docker run`:

```bash
docker run -e PG_ASK_AI_KEY=your_key -e POSTGRES_PASSWORD=pass ...
```

#### For System PostgreSQL (systemd):

Edit the PostgreSQL systemd service file:

```bash
sudo systemctl edit postgresql
```

Add the environment variable:

```ini
[Service]
Environment="PG_ASK_AI_KEY=your_api_key_here"
```

Reload and restart:

```bash
sudo systemctl daemon-reload
sudo systemctl restart postgresql
```

#### For System PostgreSQL (environment file):

Create an environment file:

```bash
sudo nano /etc/postgresql/16/main/environment
```

Add:

```
PG_ASK_AI_KEY=your_api_key_here
```

Ensure PostgreSQL reads this file on startup.

## Usage

### Function: `pg_gen_query(text)`

Generates SQL from natural language but does NOT execute it. Use this to preview the generated query.

**Syntax:**
```sql
pg_gen_query(natural_language_query text) RETURNS text
```

**Examples:**

```sql
-- Preview the generated SQL
SELECT pg_gen_query('show me all users created in the last 7 days');

-- Result (example):
-- SELECT * FROM users WHERE created_at >= CURRENT_DATE - INTERVAL '7 days';

-- Use the generated SQL as needed
SELECT pg_gen_query('count users by registration month');
```

### Function: `pg_gen_execute(text)`

Generates SQL from natural language and executes it, returning a **cursor** for fetching results.

**Syntax:**
```sql
pg_gen_execute(natural_language_query text) RETURNS refcursor
```

**Important:** This function returns a cursor reference. You must fetch results from the cursor within a transaction block.

**Examples:**

```sql
-- Basic usage: open cursor and fetch results
BEGIN;
SELECT pg_gen_execute('list all tables in the public schema');
FETCH ALL FROM ai_query_result;
COMMIT;

-- Count records
BEGIN;
SELECT pg_gen_execute('how many users are there?');
FETCH ALL FROM ai_query_result;
COMMIT;

-- Fetch results one at a time
BEGIN;
SELECT pg_gen_execute('show me top 10 customers by revenue');
FETCH NEXT FROM ai_query_result;  -- Get first row
FETCH NEXT FROM ai_query_result;  -- Get second row
-- ... continue fetching
COMMIT;

-- Fetch specific number of rows
BEGIN;
SELECT pg_gen_execute('list all products');
FETCH 5 FROM ai_query_result;  -- Get first 5 rows
FETCH 5 FROM ai_query_result;  -- Get next 5 rows
COMMIT;

-- Use FORWARD and BACKWARD navigation
BEGIN;
SELECT pg_gen_execute('show daily sales for last 30 days');
FETCH FORWARD 10 FROM ai_query_result;  -- Get first 10
FETCH BACKWARD 5 FROM ai_query_result;   -- Go back 5
COMMIT;
```

**Cursor Operations:**

- `FETCH ALL FROM ai_query_result` - Fetch all rows
- `FETCH NEXT FROM ai_query_result` - Fetch next single row
- `FETCH n FROM ai_query_result` - Fetch next n rows
- `FETCH FORWARD n FROM ai_query_result` - Move forward and fetch n rows
- `FETCH BACKWARD n FROM ai_query_result` - Move backward and fetch n rows
- `CLOSE ai_query_result` - Explicitly close cursor (automatic on COMMIT)

**Why Cursors?**

Cursors provide several advantages:
- **Memory efficiency**: Fetch large result sets incrementally
- **Flexibility**: Navigate forward and backward through results
- **Control**: Process results at your own pace
- **No column definition needed**: Unlike SETOF RECORD, cursors don't require predefined column lists

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     PostgreSQL Server                        │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                  pg_ask Extension                      │  │
│  │  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │  │
│  │  │  pg_ask.cpp  │  │ explorer.cpp │  │ ai_engine.cpp│ │  │
│  │  │              │  │              │  │             │ │  │
│  │  │ PG Functions │─▶│ Schema       │─▶│ AI SDK      │ │  │
│  │  │              │  │ Inspector    │  │ Integration │ │  │
│  │  └──────────────┘  └──────────────┘  └─────────────┘ │  │
│  │         │                  │                 │         │  │
│  └─────────┼──────────────────┼─────────────────┼─────────┘  │
│            │                  │                 │            │
│            ▼                  ▼                 ▼            │
│    ┌──────────────┐  ┌──────────────┐  ┌──────────────┐    │
│    │   Catalog    │  │   System     │  │   Network    │    │
│    │   Tables     │  │   Caches     │  │   (HTTPS)    │    │
│    └──────────────┘  └──────────────┘  └──────────────┘    │
└─────────────────────────────────────────────────────────────┘
                                │
                                ▼
                        ┌───────────────┐
                        │  AI Provider  │
                        │ (OpenAI/Groq) │
                        └───────────────┘
```

### Components

- **pg_ask.cpp**: Entry point, exposes `pg_gen_query` and `pg_gen_execute` functions
- **explorer.cpp**: Database schema inspection and formatting
- **ai_engine.cpp**: AI SDK wrapper for prompt building and SQL generation
- **ai-sdk-cpp**: Third-party AI SDK for communicating with AI providers

## Development

### Project Structure

```
pg_ask/
├── src/                    # C++ source files
│   ├── pg_ask.cpp         # Main extension entry point
│   ├── explorer.cpp       # Schema inspection utilities
│   └── ai_engine.cpp      # AI integration wrapper
├── include/               # Public headers
│   ├── explorer.h
│   └── ai_engine.h
├── pg/                    # PostgreSQL extension metadata
│   ├── pg_ask.control     # Extension control file
│   └── pg_ask--1.0.sql    # Extension installation SQL
├── docker/                # Docker build configurations
│   ├── Dockerfile
│   ├── Dockerfile.minimal
│   └── init-extension.sql
├── scripts/               # Installation scripts
│   └── install.sh
├── third_party/           # External dependencies
│   └── ai-sdk-cpp/        # AI SDK (submodule)
├── CMakeLists.txt         # Build configuration
└── docker-compose.yaml    # Docker Compose setup
```

### Building for Development

```bash
# Clone with submodules
git clone --recursive https://github.com/Abiji-2020/pg_ask.git
cd pg_ask

# Build with debug symbols
mkdir -p build && cd build
cmake -DUSE_AI_SDK=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Install locally for testing
sudo make install
```

### Running Tests

```bash
# Start test database with Docker
docker-compose up -d

# Connect and test
psql -h localhost -U postgres -d postgres

# Run manual tests
CREATE EXTENSION pg_ask;
SELECT pg_gen_query('test query');
```

### Building Docker Image

```bash
# Build minimal image
docker build -f docker/Dockerfile.minimal -t pg_ask:local .

# Run locally
docker run -d \
  -e POSTGRES_PASSWORD=test \
  -e PG_ASK_AI_KEY=your_key \
  -p 5432:5432 \
  pg_ask:local
```

## Troubleshooting

### Extension Not Loading

```sql
-- Check if extension files are installed
SELECT * FROM pg_available_extensions WHERE name = 'pg_ask';

-- Check PostgreSQL logs
sudo tail -f /var/log/postgresql/postgresql-16-main.log
```

### API Key Issues

```bash
# Verify API key is set (check environment)
docker exec pg_ask env | grep PG_ASK_AI_KEY

# For system PostgreSQL
sudo systemctl show postgresql | grep Environment

# Test API connectivity
psql -c "SELECT pg_gen_query('simple test');"
```

### Build Errors

```bash
# Ensure PostgreSQL dev headers are installed
# Ubuntu/Debian
sudo apt-get install postgresql-server-dev-16

# RedHat/CentOS
sudo yum install postgresql16-devel

# Verify pg_config is available
pg_config --version
```

### Cursor Issues

If you get cursor-related errors:

```sql
-- ERROR: cursor "ai_query_result" does not exist
-- SOLUTION: Cursors must be used within a transaction block

-- Wrong:
SELECT pg_gen_execute('list users');
FETCH ALL FROM ai_query_result;  -- Error: no transaction

-- Correct:
BEGIN;
SELECT pg_gen_execute('list users');
FETCH ALL FROM ai_query_result;
COMMIT;

-- Cursor already closed
-- SOLUTION: Cursors are automatically closed on COMMIT
BEGIN;
SELECT pg_gen_execute('list users');
FETCH ALL FROM ai_query_result;
COMMIT;
-- Can't fetch again here - cursor is closed
```

### Docker Container Issues

```bash
# Check container logs
docker logs pg_ask

# Verify environment variables
docker exec pg_ask env | grep PG_ASK

# Connect to container
docker exec -it pg_ask psql -U postgres
```

## Performance Considerations

- **Schema Inspection**: The extension inspects your database schema on each call. For large databases with many tables, this adds overhead.
- **AI API Latency**: Query generation depends on external API calls, which typically take 1-3 seconds.
- **Caching**: Consider caching frequently used generated queries in your application layer.
- **Connection Pooling**: Use connection pooling to minimize overhead of extension initialization.

## Security

⚠️ **Important Security Notes:**

1. **API Keys**: Never commit API keys to version control. Use environment variables or secret management systems.
2. **Query Validation**: Review generated SQL before executing in production environments, especially with `pg_gen_execute`.
3. **Permissions**: The extension executes with the permissions of the connected user. Use appropriate database roles and privileges.
4. **Rate Limiting**: Consider implementing rate limiting for AI API calls to control costs and prevent abuse.
5. **SQL Injection**: While the AI generates SQL, always validate and sanitize user inputs to your application.

## Cost Considerations

- Each call to `pg_gen_query` or `pg_gen_execute` makes an API call to your AI provider
- Typical cost: $0.0001 - $0.001 per query (varies by provider and model)
- Consider caching for frequently asked questions
- Use `pg_gen_query` for validation before executing with `pg_gen_execute`

## Supported AI Providers

The extension works with OpenAI-compatible APIs:

- **OpenAI**: GPT-4, GPT-3.5-turbo
- **Groq**: Fast inference with various models
- **Azure OpenAI**: Enterprise OpenAI deployment
- **OpenRouter**: Access to multiple model providers
- Any provider with OpenAI-compatible API

Configure the provider in `src/ai_engine.cpp` or via extension parameters.

## Contributing

Contributions are welcome! Please see our [Contributing Guidelines](.github/CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ai-sdk-cpp](https://github.com/ClickHouse/ai-sdk-cpp/)
- Powered by PostgreSQL
- AI capabilities provided by OpenAI, Groq, and other providers

## Support

- **Issues**: [GitHub Issues](https://github.com/Abiji-2020/pg_ask/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Abiji-2020/pg_ask/discussions)
- **Documentation**: [Project Wiki](https://github.com/Abiji-2020/pg_ask/wiki)

---

**Made with ❤️ for the PostgreSQL community**
