/*
    Adapted vulnerability code of
    RUSTSEC-ID: RUSTSEC-2021-0052
    FROM:       https://github.com/andrewhickman/id-map/issues/3
    BY:         Ammar Askar
*/

#![feature(start)]
#![forbid(unsafe_code)]

use id_map::IdMap;

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let _ = std::panic::catch_unwind(|| {
        clone_from_panic_will_drop_invalid_memory();
        get_or_insert_with_leaves_state_inconsistent();
    });
    0
}

struct DropDetector(u32);

impl Drop for DropDetector {
    fn drop(&mut self) {
        println!("Dropping {}", self.0);
    }
}
impl Clone for DropDetector {
    fn clone(&self) -> Self {
        panic!("Panic on clone!");
    }
}

fn clone_from_panic_will_drop_invalid_memory() {
    let mut map = IdMap::new();
    map.insert(DropDetector(1));

    let mut map_2 = IdMap::new();
    map_2.insert(DropDetector(2));
    map_2.clone_from(&map);
}

fn get_or_insert_with_leaves_state_inconsistent() {
    let mut map: IdMap<DropDetector> = IdMap::with_capacity(0);
    map.get_or_insert_with(0, || panic!("Panic in insertion function!"));
}

struct PanicsOnDrop(u32, bool);

impl Drop for PanicsOnDrop {
    fn drop(&mut self) {
        println!("Dropping {}", self.0);

        if self.1 {
            self.1 = false;
            panic!("Panicking on drop");
        }
    }
}
