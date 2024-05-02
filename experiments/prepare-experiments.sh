#! /usr/bin/env bash
set -e
mkdir -p crates
while read line; do
    linearray=(${line})
    crate=${linearray[0]}
    version=${linearray[1]}
    cratedir="crates/${crate}-${version}"
    cargodir="${cratedir}/.cargo"
    if [ ! -d "$cratedir" ]; then
        ./download-crate.sh ${crate} ${version}
    fi
    mkdir -p ${cargodir}
    cp cargo-config.toml ${cargodir}/config.toml
    cp rust-toolchain.toml ${cratedir}/rust-toolchain.toml
done <crates.txt