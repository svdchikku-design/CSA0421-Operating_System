/* Addresses at the start and end of a page and beyond the last page (page size 4096, 4 pages). */
#include <stdio.h>
#define PAGE_SIZE 4096u
#define PAGES 4
int table[PAGES] = {9, 3, 6, 1};          /* frame numbers; all pages present */
static void tr(unsigned a)
{
    unsigned p = a / PAGE_SIZE, d = a % PAGE_SIZE;
    if (p >= PAGES) printf("%6u -> page %u offset %4u : INVALID (beyond last page, trap to OS)\n", a, p, d);
    else printf("%6u -> page %u offset %4u : physical %u (frame %d)\n", a, p, d, table[p] * PAGE_SIZE + d, table[p]);
}
int main(void)
{
    unsigned t[] = {0, 4095, 4096, 8191, 8192, 16383, 16384, 20000};
    for (int i = 0; i < 8; i++) tr(t[i]);
    return 0;
}
