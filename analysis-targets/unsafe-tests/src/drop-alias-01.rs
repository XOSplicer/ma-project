/*
    Example code that exhibits double free via an unsafe drop alias.
    Taken from [1] Fig. 2 (a) Source code of dropping aliases.

    [1] M. Cui, C. Chen, H. Xu, and Y. Zhou,
        “SafeDrop: Detecting Memory Deallocation Bugs of Rust Programs via Static Data-flow Analysis,”
        ACM Trans. Softw. Eng. Methodol., vol. 32, no. 4, pp. 1–21, Oct. 2023, doi: 10.1145/3542948.
*/

#![feature(start)]

fn genvec() -> Vec<u8> {
    let mut s = String::from("a tmp string");
    let ptr = s.as_mut_ptr();
    let v;
    unsafe {
        v = Vec::from_raw_parts(ptr, s.len(), s.len());
    }
    // mem::forget(s); // do not drop s
    // otherwise, s is dropped before return
    return v;
}
#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _v = genvec();
    // use _v -> use after free
    // drop _v before return -> double free
    return 0;
}
