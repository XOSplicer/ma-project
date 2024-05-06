#![feature(start)]

const LEN: usize = 4;

pub unsafe fn get_vec() -> *mut u8 {
    let mut v = std::mem::ManuallyDrop::new(Vec::with_capacity(LEN));
    let ptr = v.as_mut_ptr();
    let _v2 = Vec::from_raw_parts(ptr, LEN, LEN);
    ptr
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| unsafe {
        let p = get_vec();
        // p is now dangling
        let _r: u8 = std::ptr::read(p);
    });
    return 0;
}
