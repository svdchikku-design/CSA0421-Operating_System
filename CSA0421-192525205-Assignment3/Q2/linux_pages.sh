#!/bin/bash
echo "getconf PAGESIZE = $(getconf PAGESIZE)"
grep -i hugepagesize /proc/meminfo
cat > /tmp/pf.c <<'X'
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
int main(void){ size_t n=64<<20; char*p=malloc(n); for(size_t i=0;i<n;i+=4096)p[i]=1;
 struct rusage r; getrusage(RUSAGE_SELF,&r); printf("touched 64 MB (16384 pages): minor faults %ld, major faults %ld\n",r.ru_minflt,r.ru_majflt); return 0;}
X
gcc -O0 -o /tmp/pf /tmp/pf.c && /tmp/pf
