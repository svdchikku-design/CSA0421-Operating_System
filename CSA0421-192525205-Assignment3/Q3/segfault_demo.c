#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
static void handler(int sig, siginfo_t *si, void *ctx) {
    (void)ctx; char m[192]; int n = snprintf(m, sizeof m, "  child: caught SIGSEGV (signal %d), si_code=%d (%s), faulting address %p\n", sig, si->si_code, si->si_code == SEGV_MAPERR ? "SEGV_MAPERR: address not mapped" : "SEGV_ACCERR: permission violation", si->si_addr);
    if (write(1, m, (size_t)n) < 0) _exit(12);
    _exit(11);
}
int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    const char *lit = "read-only text";           /* lives in a read-only segment */
    int tests[3] = {0, 1, 2};
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (tests[i] == 2) {                   /* handler only for the third test to show siginfo */
                struct sigaction sa; memset(&sa, 0, sizeof sa); sa.sa_sigaction = handler; sa.sa_flags = SA_SIGINFO; sigaction(SIGSEGV, &sa, NULL);
            }
            if (tests[i] == 0) { volatile int *p = NULL; printf("test 0: write through NULL pointer\n"); fflush(stdout); *p = 1; }
            if (tests[i] == 1) { printf("test 1: write into a string literal (read-only segment)\n"); fflush(stdout); *(volatile char *)lit = 'X'; }
            if (tests[i] == 2) { printf("test 2: read far outside every segment (address 0x7ffff0000000), handler installed\n"); fflush(stdout); volatile int *p = (int *)0x7ffff0000000; (void)*p; }
            _exit(0);
        }
        int st; waitpid(pid, &st, 0);
        if (WIFSIGNALED(st)) printf("  parent: child killed by signal %d (%s)\n", WTERMSIG(st), strsignal(WTERMSIG(st)));
        else printf("  parent: child exited with status %d\n", WEXITSTATUS(st));
    }
    return 0;
}
