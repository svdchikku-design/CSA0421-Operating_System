/* Branch program: encrypts a transaction using the library (linked statically or dynamically). */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "enc.h"
int main(int argc, char **argv)
{
    int hold = argc > 1 && strcmp(argv[1], "hold") == 0;
    unsigned char msg[] = "PAY 1000 TO ACC-4471";
    encrypt_buf(msg, strlen((char *)msg));
    printf("%s\n", enc_version());
    printf("cipher bytes:");
    for (size_t i = 0; i < 6; i++) printf(" %02x", msg[i]);
    printf("\n&main = %p   &encrypt_buf = %p\n", (void *)main, (void *)encrypt_buf);
    if (argc > 1) {                       /* "maps" / "hold" mode: show the address map */
        FILE *f = fopen("/proc/self/maps", "r"); char line[256];
        while (fgets(line, sizeof line, f))
            if (strstr(line, "bank") || strstr(line, "libenc") || strstr(line, "libc.so") || strstr(line, "ld-linux") || strstr(line, "[stack]") || strstr(line, "[heap]"))
                fputs(line, stdout);
        fclose(f);
    }
    if (hold) sleep(3);                   /* "hold": stay alive so /proc/<pid>/smaps can be read */
    return 0;
}
