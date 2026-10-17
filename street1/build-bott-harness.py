from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'project/src/recomp/gen/recomp_0027.c').read_text()
def fn(name):
    start=s.index('void '+name+'(void)');end=s.index('\n}\n',start)+3
    return s[start:end]
pre=r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t mem[4*1024*1024];
static uint32_t eax,ebx,ecx,edx,esi,edi,esp,g_ebp,g_seh_ebp;
#define MEM8(a) mem[(uint32_t)(a)]
#define MEM32(a) (*(uint32_t*)(mem+(uint32_t)(a)))
#define MEM16(a) (*(uint16_t*)(mem+(uint32_t)(a)))
#define SMEM16(a) (*(int16_t*)(mem+(uint32_t)(a)))
#define ZX16(a) ((uint32_t)(uint16_t)(a))
#define LO8(a) ((uint8_t)(a))
#define HI8(a) ((uint8_t)((a)>>8))
#define ZX8(a) ((uint32_t)(uint8_t)(a))
#define SET_LO8(a,v) ((a)=((a)&0xffffff00)|(uint8_t)(v))
#define SET_HI8(a,v) ((a)=((a)&0xffff00ff)|((uint32_t)(uint8_t)(v)<<8))
#define PUSH32(s,v) ((s)-=4,MEM32(s)=(v))
#define POP32(s,v) ((v)=MEM32(s),(s)+=4)
#define CMP_EQ(a,b) ((a)==(b))
#define CMP_NE(a,b) ((a)!=(b))
#define CMP_A(a,b) ((uint32_t)(a)>(uint32_t)(b))
#define CMP_GE(a,b) ((int32_t)(a)>=(int32_t)(b))
#define CMP_BE(a,b) ((uint32_t)(a)<=(uint32_t)(b))
#define RECOMP_ABI_CALL(addr,func) func()
#define RECOMP_ITAIL(addr) do {printf("FAIL unexpected tail %x\n",(unsigned)(addr));exit(1);} while(0)
static void sub_0016F83C(void) {puts("FAIL unexpected compressed record");exit(1);}
#define TEST_Z(a,b) (!((a)&(b)))
#define TEST_NZ(a,b) ((a)&(b))
static void guest_cpu_yield(void) {}
static void recomp_resource_trace(const char *kind, uint32_t a, uint32_t b, uint32_t c) {(void)kind;(void)a;(void)b;(void)c;}
'''
post=r'''
int main(void) {
  FILE *image=fopen("xbe-mapped.bin","rb");if(!image)return 2;
  fread(mem,1,sizeof mem,image);fclose(image);
  FILE *f=fopen("bott0070.compressed.bin","rb");if(!f)return 2;
  fread(mem+0x10000,1,42700,f);fclose(f);
  esp=0x1f0000;PUSH32(esp,0);PUSH32(esp,0x10000);PUSH32(esp,0x20000);PUSH32(esp,0);
  sub_0016F8E2();
  uint8_t expected[70048];f=fopen("bott0070.decompressed.bin","rb");if(!f)return 2;
  fread(expected,1,sizeof expected,f);fclose(f);
  for(unsigned i=0;i<sizeof expected;i++)if(mem[0x20000+i]!=expected[i]) {
    printf("FAIL first-byte-difference %u generated=%02x reference=%02x\n",i,mem[0x20000+i],expected[i]);return 1;
  }
  printf("PASS generated sub0016F8E2 exact RefPack comparison: 70048 bytes; result=%u\n",eax);
  const unsigned starts[]={112,34160,51440};
  for(unsigned j=0;j<3;j++) {
    esp=0x1f0000;PUSH32(esp,0x20000+starts[j]);PUSH32(esp,0);
    sub_0016B449();printf("PASS parser chain start=%u size=%u\n",starts[j],eax);
  }
  return 0;
}
'''
names=['sub_0016A8FB','sub_0016A9FD','sub_0016DCC6','sub_0016DD61','sub_0016B449','sub_0016F8E2']
(root/'bott-refpack-review/harness.c').write_text(pre+'\n'.join(fn(n) for n in names)+post)
import struct
b=(root/'game_files/default.xbe').read_bytes();base=struct.unpack_from('<I',b,0x104)[0]
count,table=struct.unpack_from('<II',b,0x11c);image=bytearray(4*1024*1024)
for i in range(count):
    flags,va,size,raw,rawsize=struct.unpack_from('<IIIII',b,table-base+i*56)
    assert va+rawsize<=len(image)
    image[va:va+rawsize]=b[raw:raw+rawsize]
(root/'bott-refpack-review/xbe-mapped.bin').write_bytes(image)
