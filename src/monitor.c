#include "monitor.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int cpu(char *o,size_t n){FILE *f=fopen("/proc/loadavg","r"); if(!f)return -1; double a,b,c; if(fscanf(f,"%lf %lf %lf",&a,&b,&c)!=3){fclose(f);return -1;} fclose(f); snprintf(o,n,"Load Average: %.2f %.2f %.2f\n",a,b,c); return 0;}
static int mem(char *o,size_t n){FILE *f=fopen("/proc/meminfo","r"); if(!f)return -1; unsigned long long t=0,a=0,v; char k[64],u[16]; while(fscanf(f,"%63s %llu %15s",k,&v,u)==3){if(strcmp(k,"MemTotal:")==0)t=v;else if(strcmp(k,"MemAvailable:")==0)a=v;} fclose(f); if(!t)return -1; snprintf(o,n,"Memory: %.2f%% used (%llu / %llu MB)\n",100.0*(double)(t-a)/(double)t,(t-a)/1024ULL,t/1024ULL); return 0;}
static int up(char *o,size_t n){FILE *f=fopen("/proc/uptime","r"); if(!f)return -1; double s; if(fscanf(f,"%lf",&s)!=1){fclose(f);return -1;} fclose(f); unsigned long long x=(unsigned long long)s; snprintf(o,n,"Uptime: %llud %lluh %llum\n",x/86400ULL,(x%86400ULL)/3600ULL,(x%3600ULL)/60ULL); return 0;}
int monitor_handle(const char *c,char *o,size_t n){if(strcmp(c,"cpu")==0)return cpu(o,n);if(strcmp(c,"memory")==0)return mem(o,n);if(strcmp(c,"uptime")==0)return up(o,n);if(strcmp(c,"load")==0)return cpu(o,n);if(strcmp(c,"help")==0){snprintf(o,n,"Commands: cpu | memory | uptime | load | status | help | quit\n");return 0;}if(strcmp(c,"status")==0){char a[256],b[256],d[256];if(cpu(a,sizeof(a))<0||mem(b,sizeof(b))<0||up(d,sizeof(d))<0)return -1;snprintf(o,n,"%s%s%s",a,b,d);return 0;}snprintf(o,n,"ERR unknown command: %s\n",c);return 0;}
