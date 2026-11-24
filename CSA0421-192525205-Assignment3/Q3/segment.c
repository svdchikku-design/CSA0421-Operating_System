/* Segment-table translation with base and limit. */
#include <stdio.h>
struct seg { const char *name; unsigned base, limit; };
static struct seg table[] = {
    {"code", 1400, 1000}, {"data", 6300, 400}, {"heap", 4300, 1100}, {"stack", 3200, 400}, {"libs", 4700, 1000}
};
int main(void)
{
    struct { int s; unsigned off; } q[] = {{0, 53}, {1, 399}, {2, 1099}, {3, 0}, {4, 999}, {1, 400}, {3, 450}, {7, 10}};
    for (int i = 0; i < 8; i++) {
        int s = q[i].s; unsigned o = q[i].off;
        printf("(%d, %4u) -> ", s, o);
        if (s < 0 || s >= 5) printf("TRAP: segment %d does not exist\n", s);
        else if (o >= table[s].limit) printf("TRAP: offset %u >= limit %u of %s (addressing error)\n", o, table[s].limit, table[s].name);
        else printf("physical %u  (= base %u + %u, segment %s)\n", table[s].base + o, table[s].base, o, table[s].name);
    }
    return 0;
}
