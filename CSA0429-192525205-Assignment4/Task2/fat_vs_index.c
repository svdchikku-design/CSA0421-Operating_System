/* Reaching block k of a file: FAT chain (O(k) look-ups) versus inode-style index (O(1) / a few reads), 4 GB file = 1,048,576 blocks. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#define NB (1u << 20)
#define PER 1024u
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e9 + t.tv_nsec; }
int main(void)
{
    uint32_t *fat = malloc((size_t)NB * 4), *perm = malloc((size_t)NB * 4);
    for (uint32_t i = 0; i < NB; i++) perm[i] = i;
    srand(9); for (uint32_t i = NB - 1; i > 0; i--) { uint32_t j = (uint32_t)rand() % (i + 1), t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
    /* the file's blocks are scattered over the disk: block k of the file is perm[k]; FAT[perm[k]] = perm[k+1] */
    for (uint32_t k = 0; k + 1 < NB; k++) { fat[perm[k]] = perm[k + 1]; }
    fat[perm[NB - 1]] = 0xFFFFFFFF;
    /* inode: 12 direct, single indirect (1024), double indirect (1024 x 1024) as arrays of block addresses */
    uint32_t direct[12], *single = malloc(PER * 4), **dbl = malloc(PER * sizeof *dbl);
    for (uint32_t k = 0; k < 12; k++) direct[k] = perm[k];
    for (uint32_t k = 0; k < PER; k++) single[k] = perm[12 + k];
    for (uint32_t i = 0; i < PER; i++) { dbl[i] = malloc(PER * 4); for (uint32_t j = 0; j < PER; j++) { uint32_t k = 12 + PER + i * PER + j; dbl[i][j] = k < NB ? perm[k] : 0; } }
    uint32_t ks[] = {0, 11, 100, 1000, 10000, 100000, 1000000, NB - 1};
    printf("  k (block of file)   FAT chain look-ups   FAT time (ns)   index reads   index time (ns)   speed-up\n");
    for (int q = 0; q < 8; q++) {
        uint32_t k = ks[q]; int reps = k > 100000 ? 20 : 2000; volatile uint32_t sink = 0;
        double t0 = now();
        for (int r = 0; r < reps; r++) { uint32_t b = perm[0]; for (uint32_t i = 0; i < k; i++) b = fat[b]; sink += b; }
        double tf = (now() - t0) / reps; if (sink == 7) puts("");
        int reads; uint32_t got = 0; t0 = now();
        for (int r = 0; r < 2000000; r++) {
            if (k < 12) { got = direct[k]; reads = 1; } else if (k < 12 + PER) { got = single[k - 12]; reads = 2; } else { uint32_t x = k - 12 - PER; got = dbl[x / PER][x % PER]; reads = 3; }
            sink += got;
        }
        double ti = (now() - t0) / 2000000;
        printf("%10u %20u %15.0f %13d %17.1f %10.0fx   (block %u, check %s)\n", k, k, tf, reads, ti, tf / ti, got, got == perm[k] ? "ok" : "WRONG");
    }
    printf("memory for the FAT of this 4 GB file's disk region: %u entries x 4 B = %u KB; index: 12 + 1 + 1 + 1024 index blocks x 4 KB = %u KB\n", NB, NB * 4 / 1024, (3 + PER) * 4);
    return 0;
}
