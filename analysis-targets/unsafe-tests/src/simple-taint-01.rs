#![feature(start)]

#[inline(never)]
#[no_mangle]
unsafe fn source() -> i32 {
    1029384756
}

#[inline(never)]
#[no_mangle]
fn sink(source: i32) -> i32 {
    source
}

#[inline(never)]
#[no_mangle]
fn sanitize(source: i32) -> i32 {
    source
}

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let unsanitized = unsafe { source() };
    let source = unsafe { source() };
    let sanitized = sanitize(source);
    let _sink_unsanitized = sink(unsanitized);
    let _sink_sanitized = sink(sanitized);
    return 0;
}
