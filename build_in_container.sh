#!/bin/sh

set -e

echo "Building Docker image..."
docker build -t vidgrok-build .
echo "Removing previous Docker container..."
docker container rm vidgrok-build || true
echo "Launching build.sh inside container..."
docker run -ti --name vidgrok-build -v "$(pwd):/project" vidgrok-build ./build.sh

