#![feature(start)]

struct Foo {
    x: u32,
    y: u32
}

impl Foo {
    fn new() -> Self {
        let x = unsafe { source() };
        Foo {
            x,
            y: 0
        }
    }
}

#[no_mangle]
unsafe fn source() -> u32 {
    42
}

#[no_mangle]
fn sink(value: u32) -> u32 {
    value
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let foo = Foo::new();
    sink(foo.x);
    sink(foo.y);
    return 0;
}
