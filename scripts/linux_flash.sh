#!/bin/bash

readonly PROJECT_DIRECTORY="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && cd .. && pwd)"
readonly IMAGE_NAME='ghcr.io/spacebee-technologies/arduino-environment:v0.2.0'
readonly PORT="${1:-/dev/ttyACM0}"

docker run --privileged --rm -t -i -v "${PROJECT_DIRECTORY}:/workspace" \
           -w /workspace -v /dev:/dev "${IMAGE_NAME}" \
           bash -c "arduino-cli upload -p ${PORT} --fqbn esp32:esp32:esp32 -t -v --input-dir build"
