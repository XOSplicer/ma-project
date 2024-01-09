#! /usr/bin/env bash

set -eo pipefail

echo "Running PhASAR unit tests..."

NUM_FAILED_TESTS=0

pushd "build/phasar/unittests"
for x in $(find . -type f -executable -print); do
    pushd "${x%/*}" && ./"${x##*/}" || { echo "Test ${x} failed."; NUM_FAILED_TESTS=$((NUM_FAILED_TESTS+1)); };
    popd;
    done
popd

echo "Finished running PhASAR unittests"
echo "${NUM_FAILED_TESTS} tests failed"
