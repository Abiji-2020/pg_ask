#!/bin/bash

# pg_ask Extension Installer for Linux
# Usage: ./install.sh [--version VERSION] [--pg-version PG_VERSION]

set -e

VERSION="latest"
PG_VERSION=$(pg_config --version 2>/dev/null | grep -oP '\d+' | head -1 || echo "16")
GITHUB_REPOSITORY="${GITHUB_REPOSITORY:-Abiji-2020/pg_ask}"
PAGES_URL="https://abiji-2020.github.io/pg_ask"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --version) VERSION="$2"; shift 2 ;;
        --pg-version) PG_VERSION="$2"; shift 2 ;;
        *) log_error "Unknown option: $1"; ;;
    esac
done

log_info "Installing pg_ask extension (version: $VERSION, PostgreSQL: $PG_VERSION)"

# Check if pg_config is available
if ! command -v pg_config &> /dev/null; then
    log_error "PostgreSQL development files not found. Please install postgresql-dev or postgresql-server-devel."
fi

# Get PostgreSQL directories
PKGLIBDIR=$(pg_config --pkglibdir)
SHAREDIR=$(pg_config --sharedir)

log_info "Using PostgreSQL at: $PKGLIBDIR"

# Determine platform
OS=$(uname -s)
case "$OS" in
    Linux)
        PLATFORM="linux-x86_64"
        EXTENSION_FILE="pg_ask.so"
        ;;
    *)
        log_error "Unsupported platform: $OS"
        ;;
esac

log_info "Detected platform: $PLATFORM"

# Download extension
DOWNLOAD_URL="${PAGES_URL}/releases/${PLATFORM}/${EXTENSION_FILE}"
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

log_info "Downloading extension from: $DOWNLOAD_URL"
if ! curl -fsSL -o "$TEMP_DIR/$EXTENSION_FILE" "$DOWNLOAD_URL"; then
    log_error "Failed to download extension. Check the URL and your connection."
fi

# Download SQL and control files
curl -fsSL -o "$TEMP_DIR/pg_ask.control" "${PAGES_URL}/releases/${PLATFORM}/pg_ask.control" || \
    log_error "Failed to download pg_ask.control"
curl -fsSL -o "$TEMP_DIR/pg_ask--1.0.sql" "${PAGES_URL}/releases/${PLATFORM}/pg_ask--1.0.sql" || \
    log_error "Failed to download pg_ask--1.0.sql"

# Install files
log_info "Installing extension files..."
sudo cp "$TEMP_DIR/$EXTENSION_FILE" "$PKGLIBDIR/pg_ask.so" || \
    log_error "Failed to install extension binary to $PKGLIBDIR"
sudo cp "$TEMP_DIR/pg_ask.control" "$SHAREDIR/extension/pg_ask.control" || \
    log_error "Failed to install control file to $SHAREDIR/extension"
sudo cp "$TEMP_DIR/pg_ask--1.0.sql" "$SHAREDIR/extension/pg_ask--1.0.sql" || \
    log_error "Failed to install SQL file to $SHAREDIR/extension"

log_info "Extension installed successfully!"
log_info "Create the extension with: CREATE EXTENSION pg_ask;"
