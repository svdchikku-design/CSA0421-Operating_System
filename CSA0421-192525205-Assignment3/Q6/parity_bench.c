/* RAID 5 parity (XOR) vs RAID 6 parity (XOR + Reed-Solomon Q over GF(2^8)) for a stripe of 4 data disks x 4 KB. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define D 4
#define B 4096
static uint8_t exp_[512], log_[256];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e9 + t.tv_nsec; }
static uint8_t gmul(uint8_t a, uint8_t b) { return (a && b) ? exp_[log_[a] + log_[b]] : 0; }
int main(void)
{
    int x = 1; for (int i = 0; i < 255; i++) { exp_[i] = (uint8_t)x; log_[x] = (uint8_t)i; x <<= 1; if (x & 0x100) x ^= 0x11d; }
    for (int i = 255; i < 512; i++) exp_[i] = exp_[i - 255];
    static uint8_t data[D][B], P[B], Q[B], P2[B]; srand(1);
    for (int d = 0; d < D; d++) for (int i = 0; i < B; i++) data[d][i] = (uint8_t)rand();
    int N = 20000; volatile uint8_t sink = 0; double t0 = now();
    for (int r = 0; r < N; r++) { memset(P, 0, B); for (int d = 0; d < D; d++) for (int i = 0; i < B; i++) P[i] ^= data[d][i]; sink ^= P[r & 4095]; }
    double t_p = (now() - t0) / N; t0 = now();
    for (int r = 0; r < N; r++) {
        memset(P2, 0, B); memset(Q, 0, B);
        for (int d = 0; d < D; d++) { uint8_t g = exp_[d]; for (int i = 0; i < B; i++) { P2[i] ^= data[d][i]; Q[i] ^= gmul(g, data[d][i]); } }
        sink ^= Q[r & 4095];
    }
    double t_pq = (now() - t0) / N;
    /* recover a lost data block with P (RAID 5) to prove the code is right */
    uint8_t rec[B]; memcpy(rec, P, B); for (int d = 1; d < D; d++) for (int i = 0; i < B; i++) rec[i] ^= data[d][i];
    printf("RAID 5 parity recovery of lost disk 0: %s\n", memcmp(rec, data[0], B) == 0 ? "correct" : "WRONG");
    /* RAID 6: recover two lost blocks (disks 0 and 2) from P and Q */
    int ok = 1;
    for (int i = 0; i < B; i++) {
        uint8_t Pxy = P2[i], Qxy = Q[i]; for (int d = 0; d < D; d++) if (d != 0 && d != 2) { Pxy ^= data[d][i]; Qxy ^= gmul(exp_[d], data[d][i]); }
        /* Pxy = Da ^ Db ; Qxy = g0*Da ^ g2*Db  with g0 = 1, g2 = exp(2) */
        uint8_t g2 = exp_[2]; uint8_t db = gmul(Qxy ^ Pxy, exp_[(255 - log_[1 ^ g2]) % 255]); uint8_t da = Pxy ^ db;
        if (da != data[0][i] || db != data[2][i]) ok = 0;
    }
    printf("RAID 6 recovery of TWO lost disks (0 and 2) from P and Q: %s\n", ok ? "correct" : "WRONG");
    printf("parity for one 16 KB stripe (4 data blocks x 4 KB): RAID 5 (P only) %.0f ns, RAID 6 (P and Q) %.0f ns = %.1fx more CPU work\n", t_p, t_pq, t_pq / t_p);
    printf("a hard disk write takes ~5,000,000 ns, so the RAID 6 parity CPU time (unoptimised scalar log/exp-table code; production code uses SIMD) is only %.2f%% of one disk write: the real extra cost is the 2 additional disk operations\n", 100 * t_pq / 5e6);
    return (int)sink * 0;
}
