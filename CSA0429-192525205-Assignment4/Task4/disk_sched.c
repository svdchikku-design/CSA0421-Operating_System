/* Disk-scheduling study: FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK.
   Part 1: the 8-request queue of the assignment (order and total head movement).
   Part 2: time-based simulation of 10,000 random requests under light, medium and heavy load.
   Part 3: starvation test: a stream of requests near cylinder 60 every millisecond while cylinder 183 waits.
   Disk model: seek = 0 if the head does not move, else 2 ms + 0.1 ms per cylinder; rotation 4 ms; transfer 0.5 ms. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXCYL 199
enum { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, NALG };
static const char *NAME[NALG] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};
typedef struct { int cyl; double arr; double start; int done; } Req;
static double seek_ms(int d) { return d == 0 ? 0 : 2.0 + 0.1 * d; }
#define XFER 4.5

/* ---- part 1: static queue, as in the lecture ---- */
static int cmpi(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
static void static_queue(int alg, const int *req, int n, int head, int *order, int *cnt, int *total, int *jump)
{
    int s[32], m = 0, pos = head; memcpy(s, req, n * sizeof *s); qsort(s, n, sizeof *s, cmpi); *jump = 0; *total = 0;
    if (alg == FCFS) { for (int i = 0; i < n; i++) order[m++] = req[i]; }
    else if (alg == SSTF) { int dn[32] = {0}; for (int k = 0; k < n; k++) { int b = -1; for (int i = 0; i < n; i++) if (!dn[i] && (b < 0 || abs(req[i] - pos) < abs(req[b] - pos))) b = i; dn[b] = 1; order[m++] = req[b]; pos = req[b]; } }
    else if (alg == SCAN) { for (int i = n - 1; i >= 0; i--) if (s[i] < head) order[m++] = s[i]; order[m++] = 0; for (int i = 0; i < n; i++) if (s[i] > head) order[m++] = s[i]; }
    else if (alg == CSCAN) { for (int i = 0; i < n; i++) if (s[i] > head) order[m++] = s[i]; order[m++] = MAXCYL; order[m++] = 0; for (int i = 0; i < n; i++) if (s[i] < head) order[m++] = s[i]; }
    else if (alg == LOOK) { for (int i = n - 1; i >= 0; i--) if (s[i] < head) order[m++] = s[i]; for (int i = 0; i < n; i++) if (s[i] > head) order[m++] = s[i]; }
    else { for (int i = 0; i < n; i++) if (s[i] > head) order[m++] = s[i]; for (int i = 0; i < n; i++) if (s[i] < head) order[m++] = s[i]; }
    pos = head;
    for (int i = 0; i < m; i++) { int d = abs(order[i] - pos); if ((alg == CSCAN && order[i] == 0 && i > 0 && order[i - 1] == MAXCYL) || (alg == CLOOK && i > 0 && order[i] < order[i - 1])) *jump = d; else *total += d; pos = order[i]; }
    *cnt = m;
}

/* ---- part 2/3: time-based simulation ---- */
typedef struct { double avg_move, avg_wait, max_wait, p99_wait, util; double t183; long moved; } Res;
static int cmpd(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }
static Res simulate(int alg, Req *r, int n, int head0, int track183)
{
    int head = head0, dir = +1, left = n; double t = 0, busy = 0; long moved = 0; Res res = {0}; res.t183 = -1;
    double *waits = malloc(n * sizeof *waits); int nw = 0, nextarr = 0; double sum = 0, mx = 0;
    /* r[] is sorted by arrival time */
    int *pend = malloc(n * sizeof *pend); int np = 0;
    while (left > 0) {
        while (nextarr < n && r[nextarr].arr <= t) pend[np++] = nextarr++;
        if (np == 0) { t = r[nextarr].arr; continue; }
        int pick = -1;
        if (alg == FCFS) { pick = 0; /* pend[] is in arrival order */ }
        else if (alg == SSTF) { int bd = 1 << 30; for (int i = 0; i < np; i++) { int d = abs(r[pend[i]].cyl - head); if (d < bd) { bd = d; pick = i; } } }
        else {
            int bd; int tries = 0;
            for (;;) {                                      /* find the nearest request in the direction of travel */
                bd = 1 << 30; pick = -1;
                for (int i = 0; i < np; i++) { int d = (r[pend[i]].cyl - head) * dir; if (d >= 0 && d < bd) { bd = d; pick = i; } }
                if (pick >= 0) break;
                /* nothing ahead: SCAN/C-SCAN go to the disk end first; LOOK reverses at once; C-LOOK jumps to the lowest request */
                if (alg == SCAN) { int end = dir > 0 ? MAXCYL : 0; int d = abs(end - head); moved += d; double tm = seek_ms(d); t += tm; busy += tm; head = end; dir = -dir; }
                else if (alg == CSCAN) { int d1 = abs(MAXCYL - head); moved += d1 + MAXCYL; double tm = seek_ms(d1) + seek_ms(MAXCYL); t += tm; busy += tm; head = 0; }
                else if (alg == LOOK) { dir = -dir; }
                else { int lo = 1 << 30; for (int i = 0; i < np; i++) if (r[pend[i]].cyl < lo) lo = r[pend[i]].cyl; int d = abs(head - lo); moved += d; double tm = seek_ms(d); t += tm; busy += tm; head = lo; }
                while (nextarr < n && r[nextarr].arr <= t) pend[np++] = nextarr++;
                if (++tries > 4) { pick = 0; break; }
            }
        }
        int id = pend[pick]; memmove(&pend[pick], &pend[pick + 1], (np - pick - 1) * sizeof *pend); np--;
        int d = abs(r[id].cyl - head); moved += d; double svc = seek_ms(d) + XFER;
        r[id].start = t; double w = t - r[id].arr; waits[nw++] = w; sum += w; if (w > mx) mx = w;
        t += svc; busy += svc; head = r[id].cyl; left--;
        if (track183 && r[id].cyl == 183) res.t183 = t - r[id].arr;
    }
    qsort(waits, nw, sizeof *waits, cmpd);
    res.avg_move = (double)moved / n; res.avg_wait = sum / n; res.max_wait = mx; res.p99_wait = waits[(int)(nw * 0.99)]; res.util = busy / t; res.moved = moved;
    free(waits); free(pend); return res;
}
static unsigned long long rs = 20260101ULL;
static double urand(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return ((rs >> 11) + 0.5) / 9007199254740992.0; }
int main(void)
{
    int req[8] = {98, 183, 37, 122, 14, 124, 65, 67}, order[40], cnt, total, jump;
    printf("=== Part 1: queue 98 183 37 122 14 124 65 67, head at 53, cylinders 0-199 ===\n");
    for (int a = 0; a < NALG; a++) {
        static_queue(a, req, 8, 53, order, &cnt, &total, &jump);
        printf("%-7s 53", NAME[a]); for (int i = 0; i < cnt; i++) printf(" %d", order[i]);
        printf("  -> %d cylinders", total); if (jump) printf(" (+%d for the return jump = %d)", jump, total + jump); printf("\n");
    }
    printf("\n=== Part 2: 10,000 random requests (uniform cylinders), Poisson arrivals ===\n");
    printf("model: seek = 2 ms + 0.1 ms/cylinder, rotation 4 ms, transfer 0.5 ms (mean random service about 11.5 ms)\n");
    double loads[3] = {40.0, 14.0, 11.0}; const char *ln[3] = {"light  (mean inter-arrival 40 ms)", "medium (mean inter-arrival 14 ms)", "heavy  (mean inter-arrival 11 ms)"};
    int n = 10000; Req *base = malloc(n * sizeof *base);
    for (int L = 0; L < 3; L++) {
        rs = 20260101ULL + L; double t = 0; for (int i = 0; i < n; i++) { t += -loads[L] * log(urand()); base[i].cyl = (int)(urand() * (MAXCYL + 1)); base[i].arr = t; base[i].done = 0; }
        printf("\n%s\nalgorithm   avg head movement (cyl/request)   avg wait (ms)   p99 wait (ms)   max wait (ms)   disk busy\n", ln[L]);
        for (int a = 0; a < NALG; a++) { Req *r = malloc(n * sizeof *r); memcpy(r, base, n * sizeof *r); Res x = simulate(a, r, n, 100, 0);
            printf("%-10s %20.1f %24.1f %15.1f %15.1f %10.0f%%\n", NAME[a], x.avg_move, x.avg_wait, x.p99_wait, x.max_wait, x.util * 100); free(r); }
    }
    printf("\n=== Part 3: starvation: the 8-request queue at t=0, then one request near cylinder 60 every millisecond (3,000 requests) ===\n");
    int m = 8 + 3000; Req *r0 = malloc(m * sizeof *r0); rs = 777;
    for (int i = 0; i < 8; i++) { r0[i].cyl = req[i]; r0[i].arr = 0; }
    for (int i = 0; i < 3000; i++) { r0[8 + i].cyl = 55 + (int)(urand() * 11); r0[8 + i].arr = 1.0 + i; }
    printf("algorithm   time until cylinder 183 is served (ms from its arrival)\n");
    for (int a = 0; a < NALG; a++) { Req *r = malloc(m * sizeof *r); memcpy(r, r0, m * sizeof *r); Res x = simulate(a, r, m, 53, 1);
        printf("%-10s %10.1f ms%s\n", NAME[a], x.t183, x.t183 > 3000 ? "   <- STARVED: served only after the whole backlog of near-60 requests (3,000 requests arriving in 3 s) had been served" : ""); free(r); }
    return 0;
}
