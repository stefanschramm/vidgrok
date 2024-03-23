#!/bin/sh

WORKDIR=$(realpath "$(dirname "$0")")

set -e

echo "Building Docker image..."
echo "Project directory is: $WORKDIR"
docker build -t vidgrok-build "$WORKDIR"
echo "Removing previous Docker container..."
docker container rm vidgrok-build || true
echo "Launching build.sh inside container..."
docker run --user $(id -u):$(id -g) -ti --name vidgrok-build -v "$WORKDIR:/project" vidgrok-build ./build.sh

