use crate::{Error, FindUnsafeRs};
use proc_macro2::LineColumn;
use std::ffi::CStr;

/// Allocate a new instance of `FindUnsafeRs``
///
/// Must be freed by calling `find_unsafe_rs_free` on it
#[no_mangle]
extern "C" fn find_unsafe_rs_new() -> Box<FindUnsafeRs> {
    Box::new(FindUnsafeRs::new())
}

/// Free an instance of `FindUnsafeRs` that was previously allocated using
/// `find_unsafe_rs_new``
#[no_mangle]
extern "C" fn find_unsafe_rs_free(f: Option<Box<FindUnsafeRs>>) {
    drop(f)
}

/// Wrapper for `FindUnsafeRs.analysze_file``
///
/// Returns 0 on success
///
/// Returns errno or negative value on error
#[no_mangle]
extern "C" fn find_unsafe_rs_analyze_file(
    f: *const FindUnsafeRs,
    path: *const libc::c_char,
) -> libc::c_int {
    if f.is_null() || path.is_null() {
        return libc::EINVAL;
    }
    unsafe {
        let path = String::from_utf8_lossy(CStr::from_ptr(path).to_bytes()).to_string();
        if let Err(e) = (*f).analyze_file(path) {
            return e.status_code();
        }
    }
    0
}

/// Wrapper for FindUnsafeRs.is_any_unsafe_location
///
/// Returns 0 on success
///
/// Returns errno or negative value on error
///
/// Writes 0 into out_ret if no unsafe location matches
///
/// Writes 1 into out_ret if any unsafe location matches
#[no_mangle]
extern "C" fn find_unsafe_rs_is_any_unsafe_location(
    f: *const FindUnsafeRs,
    path: *const libc::c_char,
    line: libc::c_uint,
    column: libc::c_uint,
    out_ret: *mut libc::c_int,
) -> libc::c_int {
    if f.is_null() || path.is_null() || out_ret.is_null() {
        return libc::EINVAL;
    }
    unsafe {
        let path = String::from_utf8_lossy(CStr::from_ptr(path).to_bytes()).to_string();
        let location = LineColumn {
            line: line as usize,
            column: column as usize,
        };
        match (*f).is_any_unsafe_location(path, location) {
            Ok(b) => *out_ret = b as libc::c_int,
            Err(e) => {
                return e.status_code();
            }
        }
    }
    0
}

impl Error {
    fn status_code(&self) -> libc::c_int {
        match *self {
            Error::FileError(ref e) => e.raw_os_error().unwrap_or(libc::EIO),
            Error::ParseError(_) => -16,
            Error::RwLockError => libc::ENOLCK,
        }
    }
}
