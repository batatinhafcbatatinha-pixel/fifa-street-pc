"""Execute the actual lifted free-list insertion, independently of game boot.

The reference checks sorted bidirectional links and guest register outputs.
No sentinel guard or allocator repair is supplied by this test.
Run with a Windows C compiler environment (vcvars64 for clang).
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

PRELUDE = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t eax,ebx,ecx,edx,esi,edi,esp;
static unsigned yields;
struct region { uint32_t base; uint8_t bytes[256]; } regions[16];
static unsigned nr;
static void *mem(uint32_t a) {
    for(unsigned i=0;i<nr;i++) if(a-regions[i].base<256) return regions[i].bytes+(a-regions[i].base);
    fprintf(stderr,"unmapped %08x\n",a); exit(2);
}
static void region(uint32_t a) { regions[nr++].base=a; }
#define MEM32(a) (*(uint32_t*)mem(a))
#define MEM16(a) (*(uint16_t*)mem(a))
#define MEM8(a) (*(uint8_t*)mem(a))
#define PUSH32(s,v) do { (s)-=4; MEM32(s)=(v); } while(0)
#define POP32(s,v) do { (v)=MEM32(s); (s)+=4; } while(0)
#define CMP_BE(a,b) ((uint32_t)(a)<=(uint32_t)(b))
#define CMP_B(a,b) ((uint32_t)(a)<(uint32_t)(b))
#define CMP_A(a,b) ((uint32_t)(a)>(uint32_t)(b))
static void guest_cpu_yield(void) { if(++yields>64) {fprintf(stderr,"loop\n");exit(3);} }
static void recomp_resource_trace(const char *event,uint32_t a,uint32_t b,uint32_t c) {(void)event;(void)a;(void)b;(void)c;}
#define CHECK(x) do {if(!(x)){fprintf(stderr,"check line %d\n",__LINE__);exit(4);}}while(0)
"""

BODY = r"""
static void run(uint32_t root, const uint32_t *nodes, unsigned count, uint32_t block) {
    uint32_t s=root+16, stack=0x7fff0100u;
    memset(regions,0,sizeof regions); nr=0; yields=0;
    region(root); region(0x7fff0080u); region(block);
    for(unsigned i=0;i<count;i++) region(nodes[i]);
    MEM32(s+16)=count?nodes[0]:s; MEM32(s+20)=count?nodes[count-1]:s;
    for(unsigned i=0;i<count;i++) {
        MEM32(nodes[i]+16)=i+1<count?nodes[i+1]:s;
        MEM32(nodes[i]+20)=i?nodes[i-1]:s;
    }
    MEM32(block+8)=block+0x84; MEM8(block+3)=0x08;
    uint32_t first=MEM32(s+16),last=MEM32(s+20),diff=last-first;
    int64_t signed_diff=(int32_t)diff;
    uint32_t midpoint=first+(uint32_t)(signed_diff/2);
    eax=root; ecx=block; edx=0x12345678; ebx=0x4321; esi=0x5678; edi=0x9abc;
    esp=stack; MEM32(stack)=0xface;
    sub_00161E9E();
    CHECK(esp==stack+4 && ebx==0x4321 && esi==0x5678 && edi==0x9abc);
    CHECK(eax==midpoint && edx==((int32_t)diff<0?0xffffffffu:0) && ecx==block);
    CHECK(MEM32(block+4)==0x84 && MEM16(block)==0x4246 && MEM8(block+3)==0x48);
    uint32_t expected[8];
    /* Build the expected order independently by sorting the input addresses. */
    for(unsigned i=0;i<count;i++) expected[i]=nodes[i]; expected[count]=block;
    for(unsigned i=0;i<count+1;i++) for(unsigned j=i+1;j<count+1;j++)
        if(expected[i]>expected[j]) {uint32_t t=expected[i];expected[i]=expected[j];expected[j]=t;}
    uint32_t at=MEM32(s+16),previous=s;
    for(unsigned i=0;i<count+1;i++) {CHECK(at==expected[i]);CHECK(MEM32(at+20)==previous);previous=at;at=MEM32(at+16);}
    CHECK(at==s && MEM32(s+20)==previous);
}
int main(void) {
    uint32_t nodes[]={0x80002000u,0x80004000u,0x80008000u};
    uint32_t inserts[]={0x80001800u,0x80003000u,0x80006000u,0x80009000u};
    for(unsigned i=0;i<4;i++) run(0x80001010u,nodes,3,inserts[i]);
    run(0x80001010u,nodes,0,0x80002000u);
    /* Empty lists permit an address below the sentinel as well. */
    run(0x80001010u,nodes,0,0x10000000u);
    /* Signed midpoint/CDQ behavior across a >2 GiB span, tail insertion. */
    uint32_t wide[]={0x10000000u,0xf0000000u};
    run(0x08000000u,wide,2,0xf1000000u);
    puts("7 heap insertion cases passed"); return 0;
}
"""

class HeapInsertionTest(unittest.TestCase):
    def test_generated_insertion(self):
        manual=(ROOT/'src/recomp_manual.c').read_text(encoding='utf-8')
        self.assertNotIn('fixed_sub_00161E9E',manual)
        generated=(ROOT/'src/recomp/gen/recomp_0025.c').read_text(encoding='utf-8')
        fn=re.search(r'void sub_00161E9E\(void\)\n\{.*?\n\}',generated,re.S)
        self.assertIsNotNone(fn)
        cc=shutil.which('clang-cl') or shutil.which('cl')
        if not cc:
            fallback=Path('C:/Program Files/LLVM/bin/clang-cl.exe')
            cc=str(fallback) if fallback.exists() else None
        self.assertIsNotNone(cc,'C compiler required')
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'heap.c'; exe=Path(tmp)/'heap.exe'
            c.write_text(PRELUDE+fn.group()+BODY,encoding='utf-8')
            built=subprocess.run([cc,'/nologo','/O2','/w',str(c),'/Fe:'+str(exe),'/Fo:'+str(Path(tmp)/'heap.obj')],capture_output=True,text=True)
            self.assertEqual(built.returncode,0,built.stdout+built.stderr)
            result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=5)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            print(result.stdout.strip())

if __name__=='__main__': unittest.main()



