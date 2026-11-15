/* Reads cmd.txt, checks a mesh's thresholds in memory, writes only if the old values match, answers in res.txt.
   Both files sit in the folder of this DLL, found at run time. No Lua API: UE4SS exports none, so Lua passes its orders through a file. */
typedef unsigned long long u64;typedef unsigned int u32;typedef void*HANDLE;
__declspec(dllimport)HANDLE __stdcall CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE);
__declspec(dllimport)int __stdcall ReadFile(HANDLE,void*,u32,u32*,void*);
__declspec(dllimport)int __stdcall WriteFile(HANDLE,const void*,u32,u32*,void*);
__declspec(dllimport)int __stdcall CloseHandle(HANDLE);
__declspec(dllimport)u64 __stdcall VirtualQuery(const void*,void*,u64);
__declspec(dllimport)void*__stdcall VirtualAlloc(void*,u64,u32,u32);
__declspec(dllimport)int __stdcall VirtualFree(void*,u64,u32);
__declspec(dllimport)u32 __stdcall GetModuleFileNameA(HANDLE,char*,u32);
extern char __ImageBase; /* this DLL's own base, defined by the linker */
#define OUTMAX (1u<<24)
static char dir[1024];static u32 dirn; /* folder of the DLL, with its last separator */
static int where(void){u32 n=GetModuleFileNameA((HANDLE)&__ImageBase,dir,sizeof dir);if(!n||n>=sizeof dir-16)return 0;
 while(n&&dir[n-1]!='\\'&&dir[n-1]!='/')n--;dirn=n;return n>0;}
static const char*at(const char*f){u32 i=0;while(f[i]){dir[dirn+i]=f[i];i++;}dir[dirn+i]=0;return dir;} /* dir + file name, in place */
typedef struct{u64 base,alloc;u32 aprot,pad0;u64 size;u32 state,prot,type,pad1;}MBI;
static int region_ok(u64 p,u64 n,int wr){
 u64 end=p+n;if(end<p)return 0; /* a span that wraps past 2^64 is never valid */
 while(p<end){MBI m;if(!VirtualQuery((void*)p,&m,sizeof m))return 0;
  if(m.state!=0x1000||(m.prot&0x101))return 0;
  u32 w=m.prot&0xff;
  if(wr){if(!(w==4||w==0x40))return 0;}else if(!(w==2||w==4||w==0x20||w==0x40))return 0;
  p=m.base+m.size;}
 return 1;}
static char*out;static u32 outn;
static void putc_(char c){if(outn<OUTMAX)out[outn++]=c;}
static void puts_(const char*s){while(*s)putc_(*s++);}
static void putu(u64 v){char b[24];int i=0;if(!v)b[i++]='0';while(v){b[i++]='0'+v%10;v/=10;}while(i)putc_(b[--i]);}
static const char*tok;
static u64 rd(void){while(*tok==' '||*tok=='\n'||*tok=='\r')tok++;u64 v=0;while(*tok>='0'&&*tok<='9')v=v*10+(*tok++-'0');return v;}
static int near_bits(u32 a,u32 b){int d=(int)a-(int)b;if(d<0)d=-d;return d<=8;}
static u64 rdptr(u64 mesh,u64 off){if(!region_ok(mesh+off,8,0))return 0;return*(u64*)(mesh+off);}
static void skip_line(void){while(*tok&&*tok!='\n')tok++;}
static u32 rdn(void){u64 n=rd();return n<1||n>8?0:(u32)n;} /* 1..8 values, the size of v and w; 0 = refused */
int pk_run(void*L){(void)L;u32 got=0;
 if(!where())return 0;
 HANDLE h=CreateFileA(at("cmd.txt"),0x80000000,1,0,3,0x80,0);if(h==(HANDLE)-1)return 0;
 char*buf=VirtualAlloc(0,1u<<25,0x3000,4);out=VirtualAlloc(0,OUTMAX,0x3000,4);outn=0;
 if(!buf||!out){CloseHandle(h);if(buf)VirtualFree(buf,0,0x8000);if(out)VirtualFree(out,0,0x8000);return 0;}
 ReadFile(h,buf,(1u<<25)-1,&got,0);CloseHandle(h);buf[got]=0;tok=buf;
 u32 v[8],w[8];static const u32 STR[]={4,8,12,16,24,32,88};
 for(;;){while(*tok==' '||*tok=='\n'||*tok=='\r')tok++;char c=*tok;if(!c)break;tok++;
  if(c=='S'){u64 m=rd();u32 n=rdn();if(!n){skip_line();puts_("S ");putu(m);puts_(" bad\n");continue;}for(u32 i=0;i<n;i++)v[i]=rd();u32 hits=0;
   for(u64 off=0x28;off<0x300;off+=8){u64 p=rdptr(m,off);if(p<0x10000||p>0x7fffffffffffULL||(p&7))continue;if(!region_ok(p,0x1000,0))continue;
    for(u32 q=0;q+4*n<0x1000;q+=4)for(u32 t=0;t<7;t++){u32 st=STR[t];if(q+st*n>=0x1000)continue;int ok=1;for(u32 i=1;i<n&&ok;i++)ok=near_bits(*(u32*)(p+q+st*i),v[i]);
     if(ok){puts_("S ");putu(m);puts_(" ");putu(off);puts_(" ");putu(q);puts_(" ");putu(st);puts_(" ");putu(*(u32*)(p+q));puts_("\n");hits++;}}}
   if(!hits){puts_("S ");putu(m);puts_(" none\n");}}
  else if(c=='V'||c=='W'){u64 m=rd(),off=rd();u32 q=rd(),st=rd(),n=rdn();
   if(!n){skip_line();putc_(c);puts_(" ");putu(m);puts_(" bad\n");continue;}
   for(u32 i=0;i<n;i++)v[i]=rd();if(c=='W')for(u32 i=0;i<n;i++)w[i]=rd();
   u64 p=rdptr(m,off);int ok=p>=0x10000&&p<0x7fffffffffffULL&&region_ok(p+q,(u64)st*n,c=='W');
   for(u32 i=0;i<n&&ok;i++)ok=near_bits(*(u32*)(p+q+(u64)st*i),v[i]);
   if(ok&&c=='W')for(u32 i=0;i<n;i++)*(volatile u32*)(p+q+(u64)st*i)=w[i];
   if(c=='V'&&!ok&&p>=0x10000&&region_ok(p+q,(u64)st*n,0)){puts_("V ");putu(m);puts_(" bad");for(u32 i=0;i<n;i++){puts_(" ");putu(*(u32*)(p+q+(u64)st*i));}puts_("\n");}
   else{putc_(c);puts_(" ");putu(m);puts_(ok?" ok\n":" bad\n");}}
  else break;}
 h=CreateFileA(at("res.txt"),0x40000000,0,0,2,0x80,0);if(h!=(HANDLE)-1){u32 wr;WriteFile(h,out,outn,&wr,0);CloseHandle(h);}
 VirtualFree(buf,0,0x8000);VirtualFree(out,0,0x8000);return 0;}
