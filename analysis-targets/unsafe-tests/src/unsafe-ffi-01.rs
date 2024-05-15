/*
   This FFI code exhibits a dangling pointer to a local variable returned from a function.

   SOURCE: https://gaultier.github.io/blog/how_to_rewrite_a_cpp_codebase_successfully.html
   ORIGINAL AUTHOR: Philippe Gaultier
   DATE: 2024-05-03
*/

#![allow(unused_unsafe)]
#![feature(start)]

use std::mem::MaybeUninit;

#[repr(C)]
// Akin to `&[u8]`, for C.
pub struct ByteSliceView {
    pub ptr: *const u8,
    pub len: usize,
}

impl From<&[u8]> for ByteSliceView {
    fn from(value: &[u8]) -> Self {
        ByteSliceView {
            ptr: value.as_ptr(),
            len: value.len(),
        }
    }
}

impl<'a> From<ByteSliceView> for &'a [u8] {
    fn from(value: ByteSliceView) -> Self {
        unsafe { std::slice::from_raw_parts(value.ptr, value.len) }
    }
}

impl<'a> From<&ByteSliceView> for &'a [u8] {
    fn from(value: &ByteSliceView) -> Self {
        unsafe { std::slice::from_raw_parts(value.ptr, value.len) }
    }
}

struct Bar {
    x: [u8; 2],
}

#[repr(C)]
struct BarC {
    x: ByteSliceView,
}

#[no_mangle]
unsafe extern "C" fn bar_parse(input: *const u8, input_len: usize, bar_c: &mut BarC) {
    let input: &[u8] = unsafe { std::slice::from_raw_parts(input, input_len) };

    let bar: Bar = Bar {
        x: [input[0], input[1]],
    };

    *bar_c = BarC {
        x: ByteSliceView {
            ptr: bar.x.as_ptr(),
            len: bar.x.len(),
        },
    };

    drop(bar);
    drop(input);
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| {
        // This mimicks how C/C++ code would call our function.
        let mut bar_c = MaybeUninit::<BarC>::uninit();
        let input = [0, 1, 2];
        unsafe {
            bar_parse(
                input.as_ptr(),
                input.len(),
                bar_c.as_mut_ptr().as_mut().unwrap(),
            );
        }

        // bar_c.ptr is now dangling as it is referencing the local
        // variable bar.x in function bar_parse

        let bar_c = unsafe { bar_c.assume_init_ref() };
        let x: &[u8] = (&bar_c.x).into();
        assert_eq!(x, [0, 1].as_slice());
    });
    0
}
