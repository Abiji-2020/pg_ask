# Full PostgreSQL image with pg_ask extension pre-installed
# Uses Debian-based postgres:16 with all standard PostgreSQL tools
FROM postgres:16 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    postgresql-server-dev-16 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source
COPY . .

# Build extension
RUN mkdir -p /build/cmake-build && \
    cd /build/cmake-build && \
    cmake -DUSE_AI_SDK=ON -DCMAKE_BUILD_PARALLEL_LEVEL=$(nproc) -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 .. && \
    make -j$(nproc)

# Final full image
FROM postgres:16

# Install only runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy built extension from builder
COPY --from=builder /build/cmake-build/pg_ask.so /usr/lib/postgresql/16/lib/
COPY --from=builder /build/pg/pg_ask.control /usr/share/postgresql/16/extension/
COPY --from=builder /build/pg/pg_ask--1.0.sql /usr/share/postgresql/16/extension/

# Create extension initialization script
RUN mkdir -p /docker-entrypoint-initdb.d
COPY docker/init-extension.sql /docker-entrypoint-initdb.d/10-pg_ask.sql

# Metadata labels (static, GitHub metadata will override on publish)
LABEL org.opencontainers.image.title="pg_ask" \
    org.opencontainers.image.description="PostgreSQL 16 with pg_ask AI-powered SQL generation extension (Full)" \
    org.opencontainers.image.url="https://github.com/Abiji-2020/pg_ask"

# Use default postgres entrypoint
# Users should provide environment variables:
#   POSTGRES_DB: database name (default: postgres)
#   POSTGRES_USER: postgres user (default: postgres)
#   POSTGRES_PASSWORD: postgres password (required)
#   PG_ASK_AI_KEY: AI API key (required for pg_ask to work)
ENTRYPOINT ["docker-entrypoint.sh"]
CMD ["postgres"]
