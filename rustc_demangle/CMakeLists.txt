cmake_minimum_required(VERSION 3.19)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
corrosion_import_crate(
  MANIFEST_PATH rustc-demangle/crates/capi/Cargo.toml
  CRATE_TYPES staticlib
)
# include PUBLIC directories
set(RUSTC_DEMANGLE_PUBLIC_INCLUDE_DIRECTORIES rustc-demangle/crates/capi/include/)

target_include_directories(rustc_demangle INTERFACE ${RUSTC_DEMANGLE_PUBLIC_INCLUDE_DIRECTORIES})
