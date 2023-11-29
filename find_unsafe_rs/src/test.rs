use crate::{FindUnsafeRs, Error};
use proc_macro2::LineColumn;

#[test]
fn file_unsafe_01() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-01.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 7, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 5, column: 13 })?);
    Ok(())
}

#[test]
fn file_unsafe_02() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-02.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 6, column: 11 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 7, column: 5 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 14, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 12, column: 13 })?);
    Ok(())
}

#[test]
fn file_unsafe_03() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-03.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 3, column: 14 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 4, column: 8 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 9, column: 13 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 10, column: 8 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 11, column: 9 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 17, column: 11 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 18, column: 5 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 25, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 23, column: 13 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 26, column: 12 })?);
    Ok(())
}

#[test]
fn file_unsafe_04() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-04.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 16, column: 9 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 23, column: 9 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 24, column: 9 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 29, column: 14 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 29, column: 52 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 7, column: 7 })?);
    Ok(())
}

#[test]
fn multiple_files() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-01.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 7, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 5, column: 13 })?);
    let path = "test/unsafe-02.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 6, column: 11 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 7, column: 5 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 14, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 12, column: 13 })?);
    let path = "test/unsafe-03.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 3, column: 14 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 4, column: 8 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 9, column: 13 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 10, column: 8 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 11, column: 9 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 17, column: 11 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 18, column: 5 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 25, column: 22 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 23, column: 13 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 26, column: 12 })?);
    let path = "test/unsafe-04.rs";
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 16, column: 9 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 23, column: 9 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 24, column: 9 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 29, column: 14 })?);
    assert!(f.is_any_unsafe_location(path, LineColumn { line: 29, column: 52 })?);
    assert!(!f.is_any_unsafe_location(path, LineColumn { line: 7, column: 7 })?);
    Ok(())
}

#[test]
fn analyze_files() -> Result<(), Error> {
    let f = FindUnsafeRs::new();
    let path = "test/unsafe-01.rs";
    f.analyze_file(path)?;
    let path = "test/unsafe-02.rs";
    f.analyze_file(path)?;
    let path = "test/unsafe-03.rs";
    f.analyze_file(path)?;
    let path = "test/unsafe-04.rs";
    f.analyze_file(path)?;
    let path = "test/does-not-exist.rs";
    match f.analyze_file(path) {
        Ok(_) => panic!("found non-existing file"),
        Err(Error::FileError(e)) =>  match e.kind() {
            std::io::ErrorKind::NotFound => { /* ok */ },
            _ => panic!("returned wrong error kind")
        }
        Err(_) => panic!("returned wrong error variant"),
    }

    Ok(())
}