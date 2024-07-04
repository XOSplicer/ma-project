#! /usr/bin/env bash
set -x
mkdir -p psr-output
# find crates -type f -name '*.ll' | grep deps | uniq | tee ll_files.txt
while read llfile; do
    outdir="psr-output/${llfile}/"
    mkdir -p ${outdir}
    (ulimit -v 2000000; ./bin/unsafe-drop-ts ${llfile} &>${outdir}/psr.log)
done <ll_files.txt
