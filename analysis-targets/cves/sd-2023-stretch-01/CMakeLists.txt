# Import targets defined in a package or workspace manifest `Cargo.toml` file
corrosion_import_crate(
  MANIFEST_PATH Cargo.toml
  CRATE_TYPES bin
  PROFILE dev
)

# NOTE: .ll files are copied on build.sh
