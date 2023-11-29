#![feature(start)]

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let x = 42_u32;
    let y = &x as *const u32;
    let z = unsafe { *y };
    return z as isize;
}
