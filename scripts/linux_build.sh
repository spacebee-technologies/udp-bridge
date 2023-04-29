#!/bin/bash

readonly PROJECT_DIRECTORY="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && cd .. && pwd)"
readonly IMAGE_NAME='espressif/idf:latest'

rm -r -f "${PROJECT_DIRECTORY}/build"
docker run --rm -v "${PROJECT_DIRECTORY}:/workspace" -w /workspace "${IMAGE_NAME}" idf.py build
