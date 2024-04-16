#![feature(start)]

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    unsafe {
        let mut v = Box::new(42_u32);
        let ptr = Box::into_raw(v);
        let v1 = Box::from_raw(ptr);
        let v2 = Box::from_raw(ptr);
        // DF
        drop(v1);
        drop(v2);
    }
    return 0;
}
