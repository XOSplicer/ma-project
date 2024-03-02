extern char * __rust_alloc(long long int x1, long long int x2);
extern char * __rust_alloc_zeroed(long long int x1, long long int x2);
extern void __rust_dealloc(char * p, long long int x1, long long int x2);

char sink(char * p) {
    return *p;
}

int main() {
    char * x = __rust_alloc(16, 16);
    sink(x);
    __rust_dealloc(x, 16, 16);
    return 0;
}