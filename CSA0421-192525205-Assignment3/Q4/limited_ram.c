/* Demand paging with a limited number of frames, done for real: a program with WS pages uses only R frames.
   When a page outside the resident set is touched, the oldest resident page is released with madvise(MADV_DONTNEED)
   (FIFO eviction), so touching it again really raises a kernel page fault. Counts the kernel's minor faults with getrusage. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#define PG 4096UL
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static long minflt(void) { struct rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
int main(int argc, char **argv)
{
    size_t WS = 4096, N = 2000000;                           /* 16 MB working set, 2M accesses */
    int local = argc > 1 && strcmp(argv[1], "locality") == 0;
    char *a = mmap(NULL, WS * PG, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("%s access, working set %zu pages (%zu MB), %zu accesses\n", local ? "locality (80% of accesses to 20% of pages)" : "uniform random", WS, WS * PG >> 20, N);
    printf("frames  frames/WS   model faults   kernel faults   fault rate   time (ms)   ns/access\n");
    size_t frames[] = {4096, 3072, 2048, 1024, 512, 256};
    for (int k = 0; k < 6; k++) {
        size_t R = frames[k]; madvise(a, WS * PG, MADV_DONTNEED);
        char *res = calloc(WS, 1); size_t *ring = calloc(R, sizeof *ring), head = 0, used = 0, faults = 0; rs = 88172645463325252ULL;
        long k0 = minflt(); double t0 = now();
        for (size_t i = 0; i < N; i++) {
            size_t p = local ? ((rnd() % 100 < 80) ? rnd() % (WS / 5) : WS / 5 + rnd() % (WS - WS / 5)) : rnd() % WS;
            if (!res[p]) {
                faults++;
                if (used == R) { size_t v = ring[head]; res[v] = 0; madvise(a + v * PG, PG, MADV_DONTNEED); ring[head] = p; head = (head + 1) % R; }
                else ring[used++] = p;
                res[p] = 1;
            }
            a[p * PG] += 1;
        }
        double t1 = now(); long k1 = minflt();
        printf("%6zu  %9.2f  %12zu  %14ld  %10.4f  %10.1f  %9.1f\n", R, (double)R / WS, faults, k1 - k0, (double)faults / N, t1 - t0, (t1 - t0) * 1e6 / N);
        free(res); free(ring);
    }
    return 0;
}
