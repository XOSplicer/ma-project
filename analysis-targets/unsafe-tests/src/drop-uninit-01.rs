/*
    Example code that exhibits double free via an unsafe drop alias.
    Taken from [1] Fig. 2 (b) Source code of dropping uninitialized memory.

    [1] M. Cui, C. Chen, H. Xu, and Y. Zhou,
        “SafeDrop: Detecting Memory Deallocation Bugs of Rust Programs via Static Data-flow Analysis,”
        ACM Trans. Softw. Eng. Methodol., vol. 32, no. 4, pp. 1–21, Oct. 2023, doi: 10.1145/3542948.
*/

#![feature(start)]
#![allow(deprecated)]
#![allow(dead_code)]

use std::io::Read;
use std::mem;
use std::slice;

struct Foo {
    vec: Vec<i32>,
}

impl Foo {
    pub unsafe fn read_from(src: &mut impl Read) -> Foo {
        #[allow(invalid_value)]
        let mut foo = mem::uninitialized::<Foo>();
        let s = slice::from_raw_parts_mut(&mut foo as *mut _ as *mut u8, mem::size_of::<Foo>());
        let _ = src.read_exact(s);
        foo
    }
}

#[no_mangle]
fn sink(v: Foo) -> Foo {
    v
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let f = unsafe { Foo::read_from(&mut &[0_u8, 0, 0, 0][..]) };
    sink(f);
    return 0;
}
