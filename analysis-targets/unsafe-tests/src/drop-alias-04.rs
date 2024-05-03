#![feature(start)]

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| unsafe {
        let mut s1 = String::from("foo");
        let ptr = s1.as_mut_ptr();
        let mut s2 = String::from_raw_parts(ptr, s1.len(), s1.len());
        drop(s1);
        let _c = s2.chars().next().unwrap();
    });
    return 0;
}
