#define _POSIX_C_SOURCE 200809L
/* Alternative concept: inverted (hashed) page table vs 2-level per-process table.
   Same mappings, 4 KB pages, 32-bit address space, 1M translations. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#define NPAGES 20000u                 /* pages mapped by the process (80 MB) */
#define NFRAMES (1u << 16)            /* 256 MB of physical memory */
#define HSIZE (1u << 17)
typedef struct { uint32_t vpn; int32_t pid, next; } ipt_e;      /* one entry per FRAME */
static int32_t *l1[1024];                                        /* 2-level: 10 + 10 bits */
static ipt_e ipt[NFRAMES]; static int32_t hash[HSIZE];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e9 + t.tv_nsec; }
int main(void)
{
    uint32_t *vpns = malloc(NPAGES * 4); srand(1);
    for (uint32_t i = 0; i < HSIZE; i++) hash[i] = -1;
    size_t l2_tables = 0;
    for (uint32_t i = 0; i < NPAGES; i++) {
        uint32_t vpn = ((uint32_t)rand() * 7919u) % (1u << 20); vpns[i] = vpn;
        if (!l1[vpn >> 10]) { l1[vpn >> 10] = calloc(1024, 4); l2_tables++; }
        l1[vpn >> 10][vpn & 1023] = (int32_t)i + 1;
        ipt[i] = (ipt_e){vpn, 1, -1}; uint32_t h = (vpn * 2654435761u) >> 15 & (HSIZE - 1);
        ipt[i].next = hash[h]; hash[h] = (int32_t)i;
    }
    long sum = 0, sum2 = 0; double t0 = now();
    for (int r = 0; r < 1000000; r++) { uint32_t v = vpns[(r * 2654435761u) % NPAGES]; sum += l1[v >> 10][v & 1023]; }
    double t_ml = (now() - t0) / 1e6; t0 = now();
    long chain = 0;
    for (int r = 0; r < 1000000; r++) {
        uint32_t v = vpns[(r * 2654435761u) % NPAGES], h = (v * 2654435761u) >> 15 & (HSIZE - 1);
        for (int32_t k = hash[h]; k >= 0; k = ipt[k].next) { chain++; if (ipt[k].vpn == v) { sum2 += k + 1; break; } }
    }
    double t_ipt = (now() - t0) / 1e6;
    printf("frame checksums: multi-level %ld, inverted %ld -> both methods find the same frames: %s\n", sum, sum2, sum == sum2 ? "yes" : "NO");
    printf("2-level table : %.1f ns per translation, 2 memory references, space %zu KB for this process (%zu second-level tables)\n", t_ml, (1024 * 8 + l2_tables * 4096) / 1024, l2_tables);
    printf("inverted+hash : %.1f ns per translation, %.2f entries examined on average, space %zu KB for the WHOLE system (%u frames x %zu B + hash %zu KB)\n",
           t_ipt, chain / 1e6, (sizeof ipt + sizeof hash) / 1024, NFRAMES, sizeof(ipt_e), sizeof hash / 1024);
    return 0;
}
