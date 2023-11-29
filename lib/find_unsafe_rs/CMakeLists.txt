cmake_minimum_required(VERSION 3.19)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
corrosion_import_crate(
  MANIFEST_PATH Cargo.toml
  CRATE_TYPES staticlib
)
# include PUBLIC directories
set(FIND_UNSAFE_RS_PUBLIC_INCLUDE_DIRECTORIES include/)

target_include_directories(find_unsafe_rs INTERFACE ${FIND_UNSAFE_RS_PUBLIC_INCLUDE_DIRECTORIES})
