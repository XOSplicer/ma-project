#!/usr/bin/env bash
echo "Downloading $1-$2"
set -ex
mkdir -p crates
TMPDIR=$(mktemp --directory --tmpdir "tmp.$1-$2.XXXXXX")
pushd $TMPDIR
curl -L -f -o $TMPDIR/$1-$2.crate https://crates.io/api/v1/crates/$1/$2/download
tar -xf $TMPDIR/$1-$2.crate
popd
mv $TMPDIR/$1-$2 crates/
rm $TMPDIR/$1-$2.crate
rmdir $TMPDIR

