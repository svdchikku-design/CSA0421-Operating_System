/* Time-critical loop with and without mlock(). A "reclaimer" thread imitates memory pressure by taking
   the loop's pages away (madvise MADV_DONTNEED, which is what page reclaim/swap does). Linux refuses this for locked pages. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <errno.h>
#include <sched.h>
#define PG 4096UL
#define PAGES 1024                      /* 4 MB buffer used by the control loop */
#define ITER 3000
static char *buf; static volatile int stop; static long refused, taken;
static double now_us(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e6 + t.tv_nsec / 1e3; }
static void *reclaimer(void *arg)
{
    (void)arg;
    while (!stop) {
        struct timespec ts = {0, 2000000}; nanosleep(&ts, NULL);                  /* every 2 ms */
        if (madvise(buf, PAGES * PG, MADV_DONTNEED) == 0) taken++; else if (errno == EINVAL) refused++;
    }
    return NULL;
}
int cmp(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }
int main(int argc, char **argv)
{
    int lock = argc > 1 && strcmp(argv[1], "lock") == 0, fifo = argc > 2 && strcmp(argv[2], "fifo") == 0;
    if (fifo) { struct sched_param sp = {50}; if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) perror("SCHED_FIFO"); }
    buf = mmap(NULL, PAGES * PG, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (lock && mlock(buf, PAGES * PG) != 0) { perror("mlock"); return 1; }
    memset(buf, 1, PAGES * PG);                                                    /* pre-fault everything */
    pthread_t th; pthread_create(&th, NULL, reclaimer, NULL);
    static double d[ITER]; struct rusage r0, r1; getrusage(RUSAGE_SELF, &r0);
    for (int i = 0; i < ITER; i++) {
        double t0 = now_us();
        for (int k = 0; k < 16; k++) buf[((size_t)(i * 16 + k) % PAGES) * PG] += 1;   /* control computation touches 16 pages */
        struct timespec ts = {0, 300000}; nanosleep(&ts, NULL);                    /* rest of the 0.3 ms period */
        d[i] = now_us() - t0;
    }
    getrusage(RUSAGE_SELF, &r1); stop = 1; pthread_join(th, NULL);
    double mx = 0, sum = 0; for (int i = 0; i < ITER; i++) { sum += d[i]; if (d[i] > mx) mx = d[i]; }
    qsort(d, ITER, sizeof d[0], cmp);
    printf("%-10s: mean %.1f us  median %.1f us  p99 %.1f us  p99.9 %.1f us  WORST %.1f us | page faults in loop %ld | reclaimer took pages %ld times, refused %ld times\n",
           fifo ? "mlock+FIFO" : lock ? "mlock" : "no lock", sum / ITER, d[ITER / 2], d[(int)(ITER * 0.99)], d[(int)(ITER * 0.999)], mx, r1.ru_minflt - r0.ru_minflt, taken, refused);
    return 0;
}
