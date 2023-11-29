#![feature(start)]

unsafe trait Bar {
    fn bar(&self);
}

struct Baz;

unsafe impl Bar for Baz {
    fn bar(&self) {
        println("Bar::baz");
    }
}

#[inline(never)]
#[no_mangle]
unsafe fn foo(x: *const u32) -> u32 {
    *x
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let x = 42_u32;
    let y = &x as *const u32;
    let z = unsafe { foo(y) };
    return z as isize;
}
