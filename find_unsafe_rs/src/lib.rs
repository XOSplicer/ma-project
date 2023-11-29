#![allow(dead_code)]

mod ffi;

#[cfg(test)]
mod test;

use std::collections::HashMap;
use std::io;
use std::path::{Path, PathBuf};
use std::{fs::File, io::Read};

use proc_macro2::{LineColumn, Span};
use syn::spanned::Spanned;
use syn::visit::Visit;
use syn::{self, ExprUnsafe, ItemFn, ItemImpl, ItemTrait};

// NOTE: LLVM debug symbols for location of litems correspond to the line of the fn item of the function, not of the attributes, as in syn

#[derive(Debug, Clone, Default)]
pub struct UnsafeVisitor<'ast> {
    pub unsafe_blocks: HashMap<SimpleSpan, &'ast ExprUnsafe>,
    pub unsafe_fns: HashMap<SimpleSpan, &'ast ItemFn>,
    pub unsafe_traits: HashMap<SimpleSpan, &'ast ItemTrait>,
    pub unsafe_impls: HashMap<SimpleSpan, &'ast ItemImpl>,
}

impl<'ast> UnsafeVisitor<'ast> {
    pub fn new() -> Self {
        Self::default()
    }
}

impl<'ast> Visit<'ast> for UnsafeVisitor<'ast> {
    fn visit_expr_unsafe(&mut self, node: &'ast ExprUnsafe) {
        self.unsafe_blocks
            .insert(SimpleSpan::from_span(node.span()), node);
        syn::visit::visit_expr_unsafe(self, node);
    }
    fn visit_item_fn(&mut self, node: &'ast ItemFn) {
        if node.sig.unsafety.is_some() {
            self.unsafe_fns
                .insert(SimpleSpan::from_span(node.span()), node);
        }
        syn::visit::visit_item_fn(self, node);
    }
    fn visit_item_trait(&mut self, node: &'ast ItemTrait) {
        if node.unsafety.is_some() {
            self.unsafe_traits
                .insert(SimpleSpan::from_span(node.span()), node);
        }
        syn::visit::visit_item_trait(self, node);
    }
    fn visit_item_impl(&mut self, node: &'ast ItemImpl) {
        if node.unsafety.is_some() {
            self.unsafe_impls
                .insert(SimpleSpan::from_span(node.span()), node);
        }
        syn::visit::visit_item_impl(self, node);
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct SimpleSpan {
    pub start: LineColumn,
    pub end: LineColumn,
}

impl SimpleSpan {
    pub fn from_span(span: Span) -> Self {
        SimpleSpan {
            start: span.start(),
            end: span.end(),
        }
    }
    pub fn contains(&self, location: LineColumn) -> bool {
        self.start <= location && location <= self.end
    }
}

#[derive(Debug, Default)]
pub struct FindUnsafeRs {
    inner: std::sync::RwLock<HashMap<PathBuf, UnsafeLocations>>,
}

impl FindUnsafeRs {
    pub fn new() -> Self {
        Self::default()
    }
}

#[derive(Debug, Clone, Default)]
pub struct UnsafeLocations {
    blocks: Vec<SimpleSpan>,
    functions: Vec<(SimpleSpan, String)>,
    traits: Vec<(SimpleSpan, String)>,
    impls: Vec<SimpleSpan>,
}

#[derive(Debug, thiserror::Error)]
pub enum Error {
    #[error("file error")]
    FileError(#[from] io::Error),
    #[error("could not parse file")]
    ParseError(#[from] syn::Error),
    #[error("read write lock poisoned")]
    RwLockError,
}

impl FindUnsafeRs {
    pub fn analyze_file<P: AsRef<Path>>(&self, path: P) -> Result<(), Error> {
        let filename = PathBuf::from(path.as_ref());
        let mut file = File::open(&filename)?;
        let mut src = String::new();
        file.read_to_string(&mut src)?;
        let ast = syn::parse_file(&src)?;
        let mut visitor = UnsafeVisitor::new();
        visitor.visit_file(&ast);

        let locations = UnsafeLocations {
            blocks: visitor.unsafe_blocks.into_keys().collect(),
            functions: visitor
                .unsafe_fns
                .into_iter()
                .map(|(k, v)| (k, v.sig.ident.to_string()))
                .collect(),
            traits: visitor
                .unsafe_traits
                .into_iter()
                .map(|(k, v)| (k, v.ident.to_string()))
                .collect(),
            impls: visitor.unsafe_impls.into_keys().collect(),
        };
        self.inner
            .write()
            .map_err(|_| Error::RwLockError)?
            .insert(filename, locations);
        Ok(())
    }

    pub fn already_analyzed<P: AsRef<Path>>(&self, path: P) -> Result<bool, Error> {
        let filename = PathBuf::from(path.as_ref());
        Ok(self
            .inner
            .read()
            .map_err(|_| Error::RwLockError)?
            .contains_key(&filename))
    }

    pub fn check_unsafe_location<P: AsRef<Path>>(
        &self,
        path: P,
        location: LineColumn,
    ) -> Result<Vec<UnsafeLocationKind>, Error> {
        if !self.already_analyzed(&path)? {
            self.analyze_file(&path)?;
        }
        let filename = PathBuf::from(path.as_ref());
        // NOTE(unwrap): entry existed previously or was just inserted
        let lock = self.inner.read().map_err(|_| Error::RwLockError)?;
        let locations = lock.get(&filename).unwrap();
        Ok(locations.get_matching(location))
    }

    pub fn is_any_unsafe_location<P: AsRef<Path>>(
        &self,
        path: P,
        location: LineColumn,
    ) -> Result<bool, Error> {
        let kinds = self.check_unsafe_location(path, location)?;
        Ok(!kinds.is_empty())
    }
}

impl UnsafeLocations {
    fn get_matching(&self, location: LineColumn) -> Vec<UnsafeLocationKind> {
        std::iter::empty()
            .chain(
                self.blocks
                    .iter()
                    .filter(|s| s.contains(location))
                    .map(|_| UnsafeLocationKind::Block),
            )
            .chain(
                self.functions
                    .iter()
                    .filter(|(s, _)| s.contains(location))
                    .map(|(_, name)| UnsafeLocationKind::Function(name.clone())),
            )
            .chain(
                self.traits
                    .iter()
                    .filter(|(s, _)| s.contains(location))
                    .map(|(_, name)| UnsafeLocationKind::Trait(name.clone())),
            )
            .chain(
                self.impls
                    .iter()
                    .filter(|s| s.contains(location))
                    .map(|_| UnsafeLocationKind::Impl),
            )
            .collect()
    }
}

// TODO: include information from SimpleSpan
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum UnsafeLocationKind {
    Block,
    Function(String),
    Trait(String),
    Impl,
}
