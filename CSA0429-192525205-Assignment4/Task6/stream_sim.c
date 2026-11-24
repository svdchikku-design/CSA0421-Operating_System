/* Task 6 Stage 5: lecture-video start-up simulator. Two designs, Zipf popularity, random seeks.
   Disk model (assumptions): HDD 8 ms seek + 4 ms rotation + 4 MB / 150 MB/s; SSD 0.1 ms + 4 MB / 500 MB/s; RAM hit 0.05 ms.
   Each disk is a single FIFO server. Start-up = read index chunk + chunk at the chosen minute. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define V 400            /* videos */
#define CH 256           /* 4 MB chunks per video (1 GB) */
#define SRV 3
static double rnd(void){ return (rand()+1.0)/(RAND_MAX+2.0); }
typedef struct { int cap; long *key; long *stamp; long clk; int n; } Lru;
static int lru_hit(Lru *c,long k){ for(int i=0;i<c->n;i++) if(c->key[i]==k){ c->stamp[i]=++c->clk; return 1;} return 0; }
static void lru_put(Lru *c,long k){ if(c->n<c->cap){ c->key[c->n]=k; c->stamp[c->n++]=++c->clk; return; }
  int o=0; for(int i=1;i<c->n;i++) if(c->stamp[i]<c->stamp[o]) o=i; c->key[o]=k; c->stamp[o]=++c->clk; }
static Lru *mk(int cap){ Lru *c=calloc(1,sizeof *c); c->cap=cap; c->key=malloc(cap*sizeof(long)); c->stamp=malloc(cap*sizeof(long)); return c; }
static int cmp(const void*a,const void*b){ double x=*(double*)a,y=*(double*)b; return (x>y)-(x<y); }
static double zcdf[V];
static int zipf(void){ double u=rnd(); int lo=0,hi=V-1; while(lo<hi){ int m=(lo+hi)/2; if(zcdf[m]<u) lo=m+1; else hi=m; } return lo; }
static void run(const char *name,int ssd,double rate,int N,int ramchunks,int ssdchunks){
  Lru *ram[SRV],*ss[SRV]; double hdd_free[SRV]={0},ssd_free[SRV]={0};
  for(int s=0;s<SRV;s++){ ram[s]=mk(ramchunks); ss[s]=mk(ssd?ssdchunks:1); }
  double *lat=malloc(N*sizeof(double)); double t=0; long rh=0,sh=0,hd=0,tot=0;
  for(int i=0;i<N;i++){
    t += -log(rnd())/rate*1000.0;                    /* ms */
    int v=zipf(), m=(int)(rnd()*60), c=m*4>CH-1?CH-1:m*4, s=v%SRV; double done=t;
    int chunks[2]={0,c};
    for(int k=0;k<2;k++){
      long key=(long)v*CH+chunks[k]; tot++;
      if(lru_hit(ram[s],key)){ rh++; done+=0.05; continue; }
      if(ssd && lru_hit(ss[s],key)){ sh++; double st=done>ssd_free[s]?done:ssd_free[s]; done=st+0.1+4.0/500*1000; ssd_free[s]=done; lru_put(ram[s],key); continue; }
      hd++; double st=done>hdd_free[s]?done:hdd_free[s]; double svc=8+4+4.0/150*1000; done=st+svc; hdd_free[s]=done;
      lru_put(ram[s],key); if(ssd) lru_put(ss[s],key);
    }
    lat[i]=done-t;
  }
  qsort(lat,N,sizeof(double),cmp); double sum=0; for(int i=0;i<N;i++) sum+=lat[i];
  printf("%-34s %6.0f/s  mean %8.1f  p50 %8.1f  p95 %8.1f  p99 %8.1f  | RAM hit %4.1f%%  SSD hit %4.1f%%  HDD reads %4.1f%%\n",
    name,rate,sum/N,lat[N/2],lat[(int)(N*.95)],lat[(int)(N*.99)],100.0*rh/tot,100.0*sh/tot,100.0*hd/tot);
}
int main(void){
  srand(42); double s=0; for(int i=0;i<V;i++){ s+=1.0/(i+1); zcdf[i]=s; } for(int i=0;i<V;i++) zcdf[i]/=s;
  printf("400 videos x 256 chunks of 4 MB (1 GB each, 400 GB total); Zipf(1.0) popularity; 3 servers; 60,000 start-ups per run; latencies in ms\n");
  printf("top 10 videos get %.0f%% of views, top 40 get %.0f%%\n\n",zcdf[9]*100,zcdf[39]*100);
  int ram=1024;   /* 4 GB RAM cache per server (1024 chunks) */
  int ssdc=25600; /* 100 GB SSD per server */
  for(int k=0;k<3;k++){ double rate=(double[]){5,12,25}[k];
    run("Design A: HDD + 4 GB RAM cache",0,rate,60000,ram,0);
    run("Design B: HDD + SSD hot tier + RAM",1,rate,60000,ram,ssdc); printf("\n"); }
  return 0; }
