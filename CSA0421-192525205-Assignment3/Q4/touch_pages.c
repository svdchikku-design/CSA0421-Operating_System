/* Touch memory sequentially and randomly, counting page faults (getrusage). */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#define PG 4096UL
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static long minflt(void) { struct rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }
int main(int argc, char **argv)
{
    size_t mb = argc > 1 ? (size_t)atol(argv[1]) : 256, n = mb << 20, pages = n / PG;
    char *a = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    long f0 = minflt(); double t0 = now();
    for (size_t i = 0; i < pages; i++) a[i * PG] = 1;                      /* pass 1: sequential first touch */
    double t1 = now(); long f1 = minflt();
    for (size_t i = 0; i < pages; i++) a[i * PG] += 1;                     /* pass 2: sequential, all resident */
    double t2 = now(); long f2 = minflt();
    printf("%zu MB = %zu pages\n", mb, pages);
    printf("sequential, first touch : %7ld faults  %8.1f ms  (%.0f ns/page)\n", f1 - f0, t1 - t0, (t1 - t0) * 1e6 / pages);
    printf("sequential, second pass : %7ld faults  %8.1f ms  (%.0f ns/page)\n", f2 - f1, t2 - t1, (t2 - t1) * 1e6 / pages);
    char *b = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    srand(5); size_t *perm = malloc(pages * sizeof *perm);
    for (size_t i = 0; i < pages; i++) perm[i] = i;
    for (size_t i = pages - 1; i > 0; i--) { size_t j = (size_t)rand() % (i + 1), t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
    long g0 = minflt(); double s0 = now();
    for (size_t i = 0; i < pages; i++) b[perm[i] * PG] = 1;                /* random order first touch */
    double s1 = now(); long g1 = minflt();
    for (size_t i = 0; i < pages; i++) b[perm[i] * PG] += 1;
    double s2 = now(); long g2 = minflt();
    printf("random, first touch     : %7ld faults  %8.1f ms  (%.0f ns/page)\n", g1 - g0, s1 - s0, (s1 - s0) * 1e6 / pages);
    printf("random, second pass     : %7ld faults  %8.1f ms  (%.0f ns/page)\n", g2 - g1, s2 - s1, (s2 - s1) * 1e6 / pages);
    return 0;
}
