#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "find_unsafe_rs.h"

void file_unsafe_01() {
    printf("test file_unsafe_01 ... ");
    char* path = "test/unsafe-01.rs";
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();
    int b = 0;

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 7, 22, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 5, 13, &b);
    assert(err == 0);
    assert(b == 0);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}

void file_unsafe_02() {
    printf("test file_unsafe_02 ... ");
    char* path = "test/unsafe-02.rs";
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();
    int b = 0;

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 6, 11, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 7, 5, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 14, 22, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 12, 13, &b);
    assert(err == 0);
    assert(b == 0);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}

void file_unsafe_03() {
    printf("test file_unsafe_03 ... ");
    char* path = "test/unsafe-03.rs";
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();
    int b = 0;

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 3, 14, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 4, 8, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 9, 13, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 10, 8, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 11, 9, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 17, 11, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 18, 5, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 25, 22, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 23, 13, &b);
    assert(err == 0);
    assert(b == 0);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 26, 12, &b);
    assert(err == 0);
    assert(b == 0);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}


void file_unsafe_04() {
    printf("test file_unsafe_04 ... ");
    char* path = "test/unsafe-04.rs";
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();
    int b = 0;

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 16, 9, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 23, 9, &b);
    assert(err == 0);
    assert(b == 0);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 24, 9, &b);
    assert(err == 0);
    assert(b == 0);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 29, 14, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 29, 52, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 7, 7, &b);
    assert(err == 0);
    assert(b == 0);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}

void multiple_files() {
    printf("test multiple_files ... ");
    char* path;
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();
    int b = 0;

    path = "test/unsafe-01.rs";

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 7, 22, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 5, 13, &b);
    assert(err == 0);
    assert(b == 0);

    path = "test/unsafe-02.rs";

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 6, 11, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 7, 5, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 14, 22, &b);
    assert(err == 0);
    assert(b == 1);

    err = find_unsafe_rs_is_any_unsafe_location(f, path, 12, 13, &b);
    assert(err == 0);
    assert(b == 0);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}

void analyze_files() {
    printf("test analyze_files ...");

    char* path;
    int err = 0;
    struct FindUnsafeRs *f = find_unsafe_rs_new();

    path = "test/unsafe-01.rs";
    err = find_unsafe_rs_analyze_file(f, path);
    assert(err == 0);

    path = "test/unsafe-02.rs";
    err = find_unsafe_rs_analyze_file(f, path);
    assert(err == 0);

    path = "test/unsafe-03.rs";
    err = find_unsafe_rs_analyze_file(f, path);
    assert(err == 0);

    path = "test/unsafe-04.rs";
    err = find_unsafe_rs_analyze_file(f, path);
    assert(err == 0);

    path = "test/does-not-exist.rs";
    err = find_unsafe_rs_analyze_file(f, path);
    assert(err == ENOENT);

    find_unsafe_rs_free(f);

    printf("ok.\n");
}

void new_free() {
    printf("test new_free ...");

    struct FindUnsafeRs *f = find_unsafe_rs_new();
    find_unsafe_rs_free(f);

    printf("ok.\n");
}

int main() {
    int err = 0;
    printf("Running tests\n");
    new_free();
    file_unsafe_01();
    file_unsafe_02();
    file_unsafe_03();
    file_unsafe_04();
    multiple_files();
    analyze_files();
    printf("Tests finished.\n");
    return 0;
}
