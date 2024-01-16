use std::io::Read;
use std::mem;
use std::slice;

struct Foo {
    vec: Vec<i32>,
}

impl Foo {
    pub unsafe fn read_from(src: &mut impl Read) -> Foo {
        let mut foo = mem::uninitialized::<Foo>();
        println!("unsafe fn in impl block");
        foo
    }
}