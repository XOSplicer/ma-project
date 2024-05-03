#![feature(start)]


/*
    Adapted From:

    Z. Li, J. Wang, M. Sun, and J. C. S. Lui,
    “MirChecker: Detecting Bugs in Rust Programs via Static Analysis,”
    in Proceedings of the 2021 ACM SIGSAC Conference on Computer and
    Communications Security, Virtual Event Republic of Korea: ACM, Nov. 2021,
    pp. 2183–2196. doi: 10.1145/3460120.3484541.

*/

const LEN: usize = 4;

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let m = &mut [2.0f32, 1.0, 7.0, 4.0];
    unsafe {
        let mut _p;
        let a = std::slice::from_raw_parts(m as *mut f32, LEN);
        let det = a[0] * a[3] - a[2] * a[1];
        if det == 0.0 {
            _p =  std::ptr::null_mut();
        }
        let mut v = std::mem::ManuallyDrop::new(Vec::with_capacity(LEN));
        let ptr = v.as_mut_ptr();
        // Fix this by replacing line with the following line:
        // let mat = slice::from_raw_parts_mut(ptr as *mut f32, LEN);
        let mut mat = Vec::from_raw_parts(ptr as *mut f32, LEN, LEN);
        let det = 1f32 / det;
        mat[0] = a[3] * det;
        mat[1] = -a[1] * det;
        mat[2] = -a[2] * det;
        mat[3] = a[0] * det;
        _p = ptr;
        drop(det);
        drop(mat);
        drop(ptr);
        drop(v);
        drop(a);

        // p is now dangling
    }
    return 0;
}


