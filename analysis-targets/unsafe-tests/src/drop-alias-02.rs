#![feature(start)]

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    unsafe {
        let mut v = Vec::from([1, 2]);
        let ptr = v.as_mut_ptr();
        let v1 = Vec::from_raw_parts(ptr, v.len(), v.len());
        // DF
        drop(v1);
        drop(v);
    }
    return 0;
}
