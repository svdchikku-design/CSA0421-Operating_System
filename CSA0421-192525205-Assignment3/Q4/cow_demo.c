/* Copy-on-write: fork() a process holding 200 MB, then let the child write to 0%, 25%, 100% of it. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static void smaps(const char *who, pid_t pid)
{
    char path[64], l[128]; snprintf(path, sizeof path, "/proc/%d/smaps_rollup", pid); FILE *f = fopen(path, "r");
    long rss = 0, pss = 0, pd = 0, sd = 0, pc = 0, sc = 0;
    while (fgets(l, sizeof l, f)) { sscanf(l, "Rss: %ld", &rss); sscanf(l, "Pss: %ld", &pss); sscanf(l, "Private_Dirty: %ld", &pd); sscanf(l, "Shared_Dirty: %ld", &sd); sscanf(l, "Private_Clean: %ld", &pc); sscanf(l, "Shared_Clean: %ld", &sc); }
    fclose(f);
    printf("  %-34s Rss %7ld kB  Pss %7ld kB  Private_Dirty %7ld kB  Shared_Dirty %7ld kB\n", who, rss / 1, pss, pd, sd);
}
int main(void)
{
    size_t n = 200UL << 20; char *a = malloc(n); memset(a, 1, n);
    printf("parent allocated and wrote 200 MB\n"); smaps("parent before fork", getpid());
    fflush(stdout); double t0 = now(); pid_t pid = fork();
    if (pid == 0) {
        double t1 = now(); (void)t1;
        int fr[] = {0, 25, 100}; struct rusage r0, r1;
        for (int i = 0; i < 3; i++) {
            size_t bytes = n / 100 * (size_t)fr[i];
            getrusage(RUSAGE_SELF, &r0); double s = now();
            for (size_t k = 0; k < bytes; k += 4096) a[k] = 2;
            double e = now(); getrusage(RUSAGE_SELF, &r1);
            char who[64]; snprintf(who, sizeof who, "child after writing %d%% of the data", fr[i]);
            smaps(who, getpid());
            printf("      (%ld COW page faults, %.1f ms)\n", r1.ru_minflt - r0.ru_minflt, e - s);
            if (fr[i] == 0) smaps("parent while child still shares", getppid());
        }
        fflush(stdout); _exit(0);
    }
    printf("fork() of the 200 MB process took %.2f ms (only the page tables are copied)\n", now() - t0);
    waitpid(pid, NULL, 0);
    double t2 = now(); char *cp = malloc(n); memset(cp, 0, n); t2 = now(); memcpy(cp, a, n); printf("for comparison, physically copying 200 MB with memcpy takes %.1f ms (checksum byte %d)\n", now() - t2, cp[n - 1]);
    return 0;
}
