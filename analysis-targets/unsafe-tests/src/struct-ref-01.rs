#![feature(start)]

struct Foo {
    x: u32,
    y: [u32; 16],
}

impl Foo {
    fn new() -> Self {
        Foo { x: 0, y: [0; 16] }
    }
}

#[no_mangle]
fn bar(f: &mut Foo, x: u32, y0: u32) {
    f.x = x;
    f.y[0] = y0;
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
    let x = source();
    let y0 = source();
    bar(&mut foo, x, y0);
    sink(foo.x);
    sink(foo.y[0]);
    return 0;
}
