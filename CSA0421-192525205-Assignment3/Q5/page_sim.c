/* Page-replacement study: FIFO, LRU, Optimal and Clock on reference strings with locality and random access. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAXF 16
#define NPAGES 64
static unsigned long long rs = 1234567ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static int fifo(const int *r, int n, int fr) { int f[MAXF], u = 0, nx = 0, fl = 0; for (int i = 0; i < n; i++) { int h = 0; for (int j = 0; j < u; j++) if (f[j] == r[i]) { h = 1; break; } if (h) continue; fl++; if (u < fr) f[u++] = r[i]; else { f[nx] = r[i]; nx = (nx + 1) % fr; } } return fl; }
static int lru(const int *r, int n, int fr) { int f[MAXF], l[MAXF], u = 0, fl = 0; for (int i = 0; i < n; i++) { int k = -1; for (int j = 0; j < u; j++) if (f[j] == r[i]) { k = j; break; } if (k >= 0) { l[k] = i; continue; } fl++; if (u < fr) { f[u] = r[i]; l[u++] = i; } else { int v = 0; for (int j = 1; j < fr; j++) if (l[j] < l[v]) v = j; f[v] = r[i]; l[v] = i; } } return fl; }
static int clockalg(const int *r, int n, int fr) { int f[MAXF], bit[MAXF], u = 0, hand = 0, fl = 0; for (int i = 0; i < n; i++) { int k = -1; for (int j = 0; j < u; j++) if (f[j] == r[i]) { k = j; break; } if (k >= 0) { bit[k] = 1; continue; } fl++; if (u < fr) { f[u] = r[i]; bit[u++] = 1; } else { while (bit[hand]) { bit[hand] = 0; hand = (hand + 1) % fr; } f[hand] = r[i]; bit[hand] = 1; hand = (hand + 1) % fr; } } return fl; }
static int opt(const int *r, int n, int fr, int *nextuse)
{   /* nextuse[i] = index of next use of r[i] after i (n if none): O(n) preprocessing makes Optimal fast */
    int f[MAXF], nu[MAXF], u = 0, fl = 0;
    for (int i = 0; i < n; i++) { int k = -1; for (int j = 0; j < u; j++) if (f[j] == r[i]) { k = j; break; }
        if (k >= 0) { nu[k] = nextuse[i]; continue; } fl++;
        if (u < fr) { f[u] = r[i]; nu[u++] = nextuse[i]; } else { int v = 0; for (int j = 1; j < fr; j++) if (nu[j] > nu[v]) v = j; f[v] = r[i]; nu[v] = nextuse[i]; } }
    return fl;
}
static void gen_locality(int *r, int n) { int base = 0; for (int i = 0; i < n; i++) { if (i % 2000 == 0) base = (int)(rnd() % (NPAGES - 6)); r[i] = (rnd() % 100 < 90) ? base + (int)(rnd() % 5) : (int)(rnd() % NPAGES); } }
static void gen_random(int *r, int n) { for (int i = 0; i < n; i++) r[i] = (int)(rnd() % NPAGES); }
int main(int argc, char **argv)
{
    int n = 100000; int *r = malloc(n * sizeof *r), *nx = malloc(n * sizeof *nx); int last[NPAGES];
    FILE *csv = fopen("page_sim.csv", "w"); fprintf(csv, "workload,frames,FIFO,LRU,OPT,CLOCK\n");
    for (int w = 0; w < 2; w++) {
        rs = 1234567ULL; if (w == 0) gen_locality(r, n); else gen_random(r, n);
        for (int p = 0; p < NPAGES; p++) { last[p] = n; }
        for (int i = n - 1; i >= 0; i--) { nx[i] = last[r[i]]; last[r[i]] = i; }
        printf("\n%s reference string (%d references, %d pages)\nframes   FIFO     LRU   Optimal   Clock   (page faults)\n", w == 0 ? "LOCALITY" : "RANDOM", n, NPAGES);
        for (int fr = 3; fr <= 10; fr++) {
            int a = fifo(r, n, fr), b = lru(r, n, fr), c = opt(r, n, fr, nx), d = clockalg(r, n, fr);
            printf("%4d %8d %8d %8d %8d\n", fr, a, b, c, d); fprintf(csv, "%s,%d,%d,%d,%d,%d\n", w == 0 ? "locality" : "random", fr, a, b, c, d);
        }
    }
    fclose(csv);
    gen_locality(r, n); for (int p = 0; p < NPAGES; p++) { last[p] = n; }
        for (int i = n - 1; i >= 0; i--) { nx[i] = last[r[i]]; last[r[i]] = i; }
    printf("\nRunning time for 100,000 references (locality string, best of 5), per-reference cost and extra state:\nframes  FIFO(ms)  LRU(ms)  OPT(ms)  Clock(ms)\n");
    int fl[] = {4, 8, 16};
    for (int q = 0; q < 3; q++) { int fr = fl[q]; double bt[4] = {1e9, 1e9, 1e9, 1e9}; volatile int sink = 0;
        for (int rep = 0; rep < 5; rep++) { double t;
            t = now(); sink += fifo(r, n, fr); t = now() - t; if (t < bt[0]) bt[0] = t;
            t = now(); sink += lru(r, n, fr); t = now() - t; if (t < bt[1]) bt[1] = t;
            t = now(); sink += opt(r, n, fr, nx); t = now() - t; if (t < bt[2]) bt[2] = t;
            t = now(); sink += clockalg(r, n, fr); t = now() - t; if (t < bt[3]) bt[3] = t; }
        printf("%4d %10.3f %8.3f %8.3f %9.3f\n", fr, bt[0], bt[1], bt[2], bt[3]); }
    (void)argc; (void)argv; return 0;
}
