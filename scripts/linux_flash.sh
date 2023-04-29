#!/bin/bash

readonly PROJECT_DIRECTORY="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && cd .. && pwd)"
readonly IMAGE_NAME='espressif/idf:latest'

docker run --rm -v "${PROJECT_DIRECTORY}:/workspace" -v /dev:/dev \
           -w /workspace "${IMAGE_NAME}" idf.py flash
docker run --rm -v "${PROJECT_DIRECTORY}:/workspace" -v /dev:/dev \
           -w /workspace "${IMAGE_NAME}" idf.py flash monitor
