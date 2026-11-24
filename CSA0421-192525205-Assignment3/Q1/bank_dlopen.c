/* Same program, but the library is loaded only when encryption is needed (dlopen/dlsym). */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "./libenc.so";
    unsigned char msg[] = "PAY 1000 TO ACC-4471";
    printf("program started, library not loaded yet\n");
    void *h = dlopen(path, RTLD_NOW);
    if (!h) {                                    /* error handling: library missing or broken */
        fprintf(stderr, "dlopen failed: %s\n-> falling back: transaction refused safely (exit 2)\n", dlerror());
        return 2;
    }
    const char *(*ver)(void) = (const char *(*)(void))dlsym(h, "enc_version");
    void (*enc)(unsigned char *, size_t) = (void (*)(unsigned char *, size_t))dlsym(h, "encrypt_buf");
    if (!ver || !enc) { fprintf(stderr, "dlsym failed: %s\n", dlerror()); dlclose(h); return 3; }
    enc(msg, strlen((char *)msg));
    printf("%s\ncipher bytes:", ver());
    for (size_t i = 0; i < 6; i++) printf(" %02x", msg[i]);
    printf("\n&encrypt_buf (loaded at run time) = %p\n", (void *)enc);
    dlclose(h);
    return 0;
}
