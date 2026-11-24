/* Encryption library, version 2 (toy XOR cipher standing in for the bank's real library). */
#include <stddef.h>
const char *enc_version(void) { return "libenc version 2 (SECURITY FIX, key 0xC3, rotate)"; }
void encrypt_buf(unsigned char *b, size_t n) { for (size_t i = 0; i < n; i++) b[i] = (unsigned char)((b[i] ^ 0xC3) << 1 | (b[i] ^ 0xC3) >> 7); }
