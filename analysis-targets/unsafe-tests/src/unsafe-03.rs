#![feature(start)]

#[inline(never)]
#[no_mangle]
fn sink(x: u32) -> u32 {
    x
}

#[inline(never)]
unsafe fn foo(x: *const u32) -> u32 {
    *x
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let x = 42_u32;
    let y = &x as *const u32;
    let z = unsafe { foo(y) };
    let q = sink(z);
    return q as isize;
}
