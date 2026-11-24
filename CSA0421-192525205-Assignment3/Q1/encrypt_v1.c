/* Encryption library, version 1 (toy XOR cipher standing in for the bank's real library). */
#include <stddef.h>
const char *enc_version(void) { return "libenc version 1 (key 0x5A)"; }
void encrypt_buf(unsigned char *b, size_t n) { for (size_t i = 0; i < n; i++) b[i] ^= 0x5A; }
