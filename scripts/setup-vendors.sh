#!/bin/bash
#
# LibreVMM Vendor Setup Script
# 
# This script clones or fetches all required upstream repositories
# into the vendor directory structure.
#
# Usage: ./scripts/setup-vendors.sh [--depth <depth>] [--dry-run]
#
# Options:
#   --depth <depth>   Set git clone depth (default: 1 for shallow clone)
#   --dry-run         Show what would be cloned without executing
#   --help            Show this help message
#

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VENDOR_DIR="$PROJECT_ROOT/vendor"

# Default values
CLONE_DEPTH=1
DRY_RUN=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --depth)
            CLONE_DEPTH="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --help)
            echo "Usage: $0 [--depth <depth>] [--dry-run]"
            echo ""
            echo "Options:"
            echo "  --depth <depth>   Set git clone depth (default: 1 for shallow clone)"
            echo "  --dry-run         Show what would be cloned without executing"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Clone or update a repository
# Arguments: $1 = target directory, $2 = git URL, $3 = optional branch/tag
clone_or_update() {
    local target_dir="$1"
    local git_url="$2"
    local branch="${3:-main}"
    local dir_name
    dir_name=$(basename "$target_dir")
    
    if [ "$DRY_RUN" = true ]; then
        log_info "[DRY-RUN] Would clone $git_url -> $target_dir (branch: $branch, depth: $CLONE_DEPTH)"
        return 0
    fi
    
    if [ -d "$target_dir" ]; then
        if [ -d "$target_dir/.git" ]; then
            log_info "Updating existing repository: $target_dir"
            (cd "$target_dir" && git fetch --all && git checkout "$branch" 2>/dev/null || git checkout -b "$branch" 2>/dev/null || true)
        else
            log_warn "Directory exists but is not a git repository: $target_dir"
            log_info "Skipping (manual intervention required)"
        fi
    else
        log_info "Cloning $dir_name..."
        mkdir -p "$(dirname "$target_dir")"
        git clone --depth "$CLONE_DEPTH" --branch "$branch" --single-branch "$git_url" "$target_dir" 2>/dev/null || \
            git clone --depth "$CLONE_DEPTH" "$git_url" "$target_dir"
    fi
}

# Vendor repositories configuration
# Format: "directory|git_url|branch"
declare -a VENDOR_REPOS=(
    "virtualbox|https://github.com/kanjitalk7557/VirtualBox.git|vbox-7.2"
    "qemu|https://github.com/qemu/qemu.git|master"
    "86Box|https://github.com/86Box/86Box.git|master"
    "Bochs/bochs|https://git.code.sf.net/p/bochs/code|master"
    "seabios|https://git.seabios.org/seabios.git|master"
    "openbios|https://github.com/openbios/openbios.git|master"
    "kvm-guest-drivers-windows|https://github.com/virtio-win/kvm-guest-drivers-windows.git|main"
    "SDL|https://github.com/libsdl-org/SDL.git|main"
    "dosbox-x|https://github.com/joncampbell123/dosbox-x.git|master"
)

# Note: JDK is typically downloaded as a pre-built binary, not cloned
# Note: VirtualBox SDKs and VirtualBox-5.2.44 are special - they may need manual download
# Note: 86Box/roms is a separate repository

# Main execution
log_info "LibreVMM Vendor Setup"
log_info "======================"
log_info "Vendor directory: $VENDOR_DIR"
log_info "Clone depth: $CLONE_DEPTH"
if [ "$DRY_RUN" = true ]; then
    log_warn "DRY RUN MODE - No changes will be made"
fi
echo ""

# Create vendor directory structure
if [ "$DRY_RUN" = false ]; then
    log_info "Creating vendor directory structure..."
    mkdir -p "$VENDOR_DIR"
    mkdir -p "$VENDOR_DIR/86Box/roms"
    mkdir -p "$VENDOR_DIR/Bochs/bochs"
    mkdir -p "$VENDOR_DIR/VirtualBox-5.2.44"
    mkdir -p "$VENDOR_DIR/VirtualBoxSDK-5.2.44-139111"
    mkdir -p "$VENDOR_DIR/VirtualBoxSDK-7.2.6-172322"
    mkdir -p "$VENDOR_DIR/jdk"
fi

# Clone standard repositories
log_info "Cloning/updating standard vendor repositories..."
echo ""

for repo in "${VENDOR_REPOS[@]}"; do
    IFS='|' read -r -a parts <<< "$repo"
    dir="${parts[0]}"
    url="${parts[1]}"
    branch="${parts[2]:-main}"
    
    target_path="$VENDOR_DIR/$dir"
    clone_or_update "$target_path" "$url" "$branch"
done

# Special handling for 86Box ROMs
log_info "Setting up 86Box ROMs..."
clone_or_update "$VENDOR_DIR/86Box/roms" "https://github.com/86Box/roms.git" "master"

# JDK - Download instead of clone (pre-built binaries)
log_info ""
log_warn "JDK setup: Download OpenJDK from https://adoptium.net/ or https://github.com/adoptium/temurinXX.X-binaries/releases"
log_warn "Extract to: $VENDOR_DIR/jdk/"
log_info "Recommended: temurin-21-linux-x64.tar.gz or temurin-21-windows-x64.zip"

# VirtualBox SDKs - Manual download required
log_info ""
log_warn "VirtualBox SDK 5.2.44-139111: Manual download required"
log_warn "URL: https://download.virtualbox.org/virtualbox/5.2.44/VirtualBoxSDK-5.2.44-139111.zip"
log_warn "Extract to: $VENDOR_DIR/VirtualBoxSDK-5.2.44-139111/"
log_info ""
log_warn "VirtualBox SDK 7.2.6-172322: Manual download required"
log_warn "URL: https://download.virtualbox.org/virtualbox/7.2.6/VirtualBoxSDK-7.2.6-172322.zip"
log_warn "Extract to: $VENDOR_DIR/VirtualBoxSDK-7.2.6-172322/"

# VirtualBox 5.2.44 source - Special case
log_info ""
log_warn "VirtualBox 5.2.44 source: Manual download required"
log_warn "URL: https://download.virtualbox.org/virtualbox/5.2.44/VirtualBox-5.2.44.tar.bz2"
log_warn "Extract to: $VENDOR_DIR/VirtualBox-5.2.44/"

echo ""
log_info "======================"
log_info "Vendor setup complete!"
log_info ""
log_info "Next steps:"
log_info "1. Download and extract VirtualBox 5.2.44 source"
log_info "2. Download and extract VirtualBox SDKs"
log_info "3. Download and install JDK"
log_info "4. Review LICENSE files in each vendor directory"
log_info "5. Run build system setup (see BUILD_GUIDE.md)"
