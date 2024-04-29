/*
    Adapted vulnerability code of
    RUSTSEC-ID: RUSTSEC-2022-0012
    FROM:       https://github.com/jorgecarleitao/arrow2/issues/880
    BY:         Ziru Niu
*/

#![feature(start)]

use std::sync::Arc;

use arrow2::array::UInt32Array;
use arrow2::ffi::{export_array_to_c, Ffi_ArrowArray};

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| {
        let arr = Arc::new(UInt32Array::from_vec(vec![1, 2, 3, 4]));
        let mut arr_ffi = Ffi_ArrowArray::empty();

        unsafe {
            export_array_to_c(arr.clone(), &mut arr_ffi as *mut _);
        };

        let a = arr_ffi.clone();

        // Segfault on drop

        drop(a);
        drop(arr_ffi);
    });
    0
}
