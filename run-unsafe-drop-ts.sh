#! /usr/bin/env bash

set -ex
DATE=$(date +%Y%m%d)
FILE=$(basename ${1})
OUTPATH="output/${DATE}-${FILE}-psr"
LOGILE="${OUTPATH}/psr-$(date -Is).log.gz"
mkdir -p ${OUTPATH}

./build/tools/unsafe-drop-ts/unsafe-drop-ts ${1} 2>&1 | gzip -1 -c - > ${LOGILE}
