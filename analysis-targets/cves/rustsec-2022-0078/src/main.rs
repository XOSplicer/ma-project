/*
    Adapted vulnerability code of
    RUSTSEC-ID: RUSTSEC-2022-0078
    FROM:       https://rustsec.org/advisories/RUSTSEC-2022-0078.html
*/

#![feature(start)]

use bumpalo::{collections::Vec, Bump};

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| {
        let bump = Bump::new();
        let mut vec = Vec::new_in(&bump);
        vec.extend([0x01u8; 32]);
        let into_iter = vec.into_iter();
        drop(bump);

        for _ in 0..100 {
            let reuse_bump = Bump::new();
            let _reuse_alloc = reuse_bump.alloc([0x41u8; 10]);
        }

        for x in into_iter {
            print!("0x{:02x} ", x);
        }
        println!();
    });
    0
}
