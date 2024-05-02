#! /usr/bin/env bash
set -ex
mkdir -p bin
cp ../build/tools/unsafe-drop-ts/unsafe-drop-ts ./bin/
cp ../build/tools/unsafe-drop-analysis/unsafe-drop-analysis ./bin/
cp ../build/tools/unsafe-taint-check/unsafe-taint-check ./bin/
docker build -t ma-project-experiments:latest -f experiments.dockerfile .
