#!/bin/bash
# OpenSentry Camera Node - Teardown Script
# Run: chmod +x teardown.sh && ./teardown.sh

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║           OpenSentry Camera Node - Teardown                   ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Check if container is running
if docker ps -q -f name=opensentry-node-camera1 | grep -q .; then
    echo "🛑 Stopping Camera Node..."
    docker compose down
    echo "✅ Camera Node stopped"
else
    echo "ℹ️  Camera Node is not running"
fi

echo ""
read -p "Remove Docker image? This will require rebuild on next setup. (y/N): " remove_image
if [ "$remove_image" = "y" ] || [ "$remove_image" = "Y" ]; then
    echo "🗑️  Removing Docker image..."
    docker rmi opensentrynode-opensentry-node 2>/dev/null || true
    echo "✅ Image removed"
fi

echo ""
read -p "Remove configuration (.env file)? (y/N): " remove_config
if [ "$remove_config" = "y" ] || [ "$remove_config" = "Y" ]; then
    rm -f .env
    echo "✅ Configuration removed"
fi

echo ""
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                  Teardown Complete!                           ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""
echo "To set up again, run: ./setup.sh"
