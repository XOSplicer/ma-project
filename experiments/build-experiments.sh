#! /usr/bin/env bash
set -ex

mkdir -p crates
for dir in $(find ./crates -mindepth 1 -maxdepth 1 -type d) ; do
    pushd ${dir}
    cargo check && cargo build && cargo build --tests
    popd
done
