#![feature(start)]

struct Foo {
    x: u32,
    y: [u32; 16],
}

impl Foo {
    fn new() -> Self {
        let x = source();
        Foo {
            x,
            y: [0; 16],
        }
    }
}

#[no_mangle]
fn source() -> u32 {
    42
}

#[no_mangle]
fn sink(value: u32) -> u32 {
    value
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let mut foo = Foo::new();
    sink(foo.x);
    sink(foo.y[0]);
    return 0;
}
