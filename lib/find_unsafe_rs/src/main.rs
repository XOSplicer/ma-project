use find_unsafe_rs::*;

use std::{env, fs::File, io::Read, process::exit};

use syn::spanned::Spanned;
use syn::visit::Visit;

fn main() {
    let mut args = env::args();
    let exe = args.next().unwrap(); // executable name
    let filename = match (args.next(), args.next()) {
        (Some(f), None) => f,
        _ => {
            eprintln!("Usage: {} path/to/file.rs", exe);
            exit(1);
        }
    };

    let mut file = File::open(&filename).expect("Could not open file");
    let mut src = String::new();
    file.read_to_string(&mut src).expect("Could not read file");
    let ast = syn::parse_file(&src).expect("Could not parse file");

    // println!("{:#?}", ast);
    //println!("================");

    let mut visitor = UnsafeVisitor::new();
    visitor.visit_file(&ast);
    for (k, _e) in visitor.unsafe_blocks {
        println!("Unsafe block with block span: {:?}", k);
    }
    for (k, e) in visitor.unsafe_fns {
        println!(
            "Unsafe fn {} with span: {:?} (fn on {:?})",
            e.sig.ident,
            k,
            e.sig.fn_token.span().start(),
        );
    }
    for (k, e) in visitor.unsafe_traits {
        println!("Unsafe trait {} with span: {:?}", e.ident, k);
    }
    for (k, _e) in visitor.unsafe_impls {
        println!("Unsafe impl with span: {:?}", k);
    }
}