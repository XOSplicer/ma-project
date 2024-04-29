/*
    Adapted vulnerability code of
    CVE-ID:     CVE-2018-20991
    RUSTSEC-ID: RUSTSEC-2018-0003
    FROM:       https://github.com/servo/rust-smallvec/issues/96
*/

#![feature(start)]

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| {

    });
    0
}

