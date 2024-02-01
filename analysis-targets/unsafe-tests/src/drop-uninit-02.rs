/*
    Example code that exhibits double free via an unsafe drop alias.
    Taken from [1] Fig. 2 (b) Source code of dropping uninitialized memory.

    [1] M. Cui, C. Chen, H. Xu, and Y. Zhou,
        “SafeDrop: Detecting Memory Deallocation Bugs of Rust Programs via Static Data-flow Analysis,”
        ACM Trans. Softw. Eng. Methodol., vol. 32, no. 4, pp. 1–21, Oct. 2023, doi: 10.1145/3542948.
*/

#![feature(start)]

use std::io::Read;
use std::mem;
use std::slice;

struct Foo {
    x: u32
}

impl Foo {
    pub unsafe fn unsafe_new() -> Foo {
        let mut foo = mem::uninitialized::<Foo>();
        foo
    }
}

#[no_mangle]
fn sink(v: Foo) {
    drop(v)
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let f = unsafe { Foo::unsafe_new()};
    sink(f);
    return 0;
}
