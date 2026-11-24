#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int global_init = 42; int global_bss[1000]; const char rodata[] = "read-only data";
int main(void)
{
    int local = 7; void *h = malloc(1 << 20); void *big = malloc(1 << 20);
    printf("code (main)      %p\nrodata           %p\ndata (init)      %p\nbss              %p\nheap (malloc)    %p\nheap (2nd malloc) %p   \nstack (local)    %p\n\n", (void *)main, (void *)rodata, (void *)&global_init, (void *)global_bss, h, big, (void *)&local);
    FILE *f = fopen("/proc/self/maps", "r"); char l[256];
    while (fgets(l, sizeof l, f)) if (strstr(l, "maps_demo") || strstr(l, "[heap]") || strstr(l, "[stack]") || strstr(l, "libc.so") || strstr(l, "[vdso]") || strstr(l, "[vvar]")) fputs(l, stdout);
    return 0;
}
