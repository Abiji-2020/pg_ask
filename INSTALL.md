# Installation Guide for pg_ask

## Quick Start

### Linux / macOS
```bash
# One-liner installation (auto-detects your platform)
curl -fsSL https://$GITHUB_PAGES_URL/install.sh | bash
```

### Windows (PowerShell)
```powershell
# Run as Administrator
Invoke-WebRequest -Uri "https://$GITHUB_PAGES_URL/install.ps1" -OutFile "install.ps1"
.\install.ps1
```

### Docker
```bash
docker pull ghcr.io/$GITHUB_REPOSITORY:latest
docker run -it \
  -e POSTGRES_PASSWORD=postgres \
  -e PG_ASK_AI_KEY="your-api-key" \
  -p 5432:5432 \
  ghcr.io/$GITHUB_REPOSITORY:latest
```

### With docker-compose
```bash
cat > docker-compose.yaml << 'EOF'
version: "3.9"

services:
  postgres:
    image: ghcr.io/$GITHUB_REPOSITORY:latest
    environment:
      POSTGRES_USER: postgres
      POSTGRES_PASSWORD: postgres
      POSTGRES_DB: askdb
      PG_ASK_AI_KEY: ${API_KEY}
    ports:
      - "5432:5432"
    volumes:
      - pgdata:/var/lib/postgresql/data

  pgadmin:
    image: dpage/pgadmin4:latest
    environment:
      PGADMIN_DEFAULT_EMAIL: admin@example.com
      PGADMIN_DEFAULT_PASSWORD: admin
    depends_on:
      - postgres
    ports:
      - "8080:80"

volumes:
  pgdata:
EOF

export API_KEY="your-api-key"
docker-compose up
```

## Direct Downloads

All pre-built binaries are available at:
**https://$GITHUB_PAGES_URL/releases/**

Or browse: **https://$GITHUB_PAGES_URL/**

## Manual Installation

### Prerequisites
- PostgreSQL 14+ with development headers installed
- CMake 3.10+
- C++ compiler (GCC, Clang, or MSVC)
- ai-sdk-cpp library (included in repository)

### Build from Source
```bash
git clone https://github.com/$GITHUB_REPOSITORY.git
cd pg_ask
mkdir build && cd build
cmake -DUSE_AI_SDK=ON ..
make
sudo make install
```

### Create Extension
```sql
CREATE EXTENSION pg_ask;
```

### Verify Installation
```sql
SELECT pg_gen_query('show me all tables');
```

## Docker Advanced Usage

### With docker-compose
```bash
cat > docker-compose.yaml << 'EOF'
version: "3.9"

services:
  postgres:
    image: ghcr.io/Abiji-2020/pg_ask:latest
    environment:
      POSTGRES_USER: postgres
      POSTGRES_PASSWORD: postgres
      POSTGRES_DB: askdb
      PG_ASK_AI_KEY: ${API_KEY}
    ports:
      - "5432:5432"
    volumes:
      - pgdata:/var/lib/postgresql/data

  pgadmin:
    image: dpage/pgadmin4:latest
    environment:
      PGADMIN_DEFAULT_EMAIL: admin@example.com
      PGADMIN_DEFAULT_PASSWORD: admin
    depends_on:
      - postgres
    ports:
      - "8080:80"

volumes:
  pgdata:
EOF

export API_KEY="your-api-key"
docker-compose up
```

## Environment Variables

### Docker/Runtime
- `PG_ASK_AI_KEY`: OPENAI API key (required for SQL generation)
- `POSTGRES_USER`: PostgreSQL user (default: postgres)
- `POSTGRES_PASSWORD`: PostgreSQL password (required)
- `POSTGRES_DB`: Initial database name (default: postgres)

### Installation Scripts
- `--version VERSION`: Specify release version (default: latest)
- `--pg-version PG_VERSION`: PostgreSQL major version (auto-detected if not specified)

## Troubleshooting

### Extension not found after installation
```sql
-- Check extension is installed
\dx pg_ask

-- Check extension files exist
SELECT pg_catalog.pg_ls_dir(setting || '/extension') 
FROM pg_settings WHERE name = 'sharedir';
```

### Permission denied errors on Linux/macOS
- Ensure you have write permissions or use `sudo`
- Try: `sudo -E bash -c 'curl ... | bash'` to preserve env vars

### Build errors
Ensure PostgreSQL development headers:
```bash
# Ubuntu/Debian
sudo apt-get install postgresql-server-dev-16

# CentOS/RHEL
sudo yum install postgresql-devel

# macOS
brew install postgresql
```

### Docker connection issues
```bash
# Test connection
docker exec pg_ask_postgres psql -U postgres -d postgres -c "SELECT version();"

# View logs
docker logs pg_ask_postgres
```

## Support

- Issues: https://github.com/$GITHUB_REPOSITORY/issues
- Releases: https://github.com/$GITHUB_REPOSITORY/releases
- Pages: https://$GITHUB_PAGES_URL/pg_ask/
