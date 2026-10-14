/* Semantics deliberately match nv2a_vsh_interp.c, including its ARL epsilon,
 * RCC bounds and invalid-register behavior. This is not a claim of bit-exact
 * NV2A transcendental math. IEEE-strict D3DCompile is required by the caller. */
#include "nv2a_vsh_translate.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
typedef struct { char *p; size_t cap, len; int failed; } Text;
static void emit(Text *t, const char *fmt, ...) {
    int n; va_list ap;
    if (t->failed) return;
    va_start(ap, fmt); n = vsnprintf(t->p + t->len, t->cap - t->len, fmt, ap); va_end(ap);
    if (n < 0 || (size_t)n >= t->cap - t->len) { t->failed = 1; return; }
    t->len += (size_t)n;
}
static uint32_t f(const uint32_t *w, int d, int b, int n) { return (w[d] >> b) & ((1u << n)-1); }
static void source(Text *t, const uint32_t *w, int which, const char *name) {
    uint32_t neg, sw, mux, reg; char base[96]; const char *xyzw = "xyzw";
    if (which == 0) { neg=f(w,1,8,1);sw=f(w,1,0,8);mux=f(w,2,26,2);reg=f(w,2,28,4); }
    else if (which == 1) { neg=f(w,2,25,1);sw=f(w,2,17,8);mux=f(w,2,11,2);reg=f(w,2,13,4); }
    else { neg=f(w,2,10,1);sw=f(w,2,2,8);mux=f(w,3,28,2);reg=(f(w,2,0,2)<<2)|f(w,3,30,2); }
    if (mux==1) {
        if (reg==12) snprintf(base,sizeof base,"opos");
        else snprintf(base,sizeof base,"r[%u]",reg<12?reg:0);
    } else if (mux==2) snprintf(base,sizeof base,"inputs[vi].attr[%u]",f(w,1,9,4));
    else {
        emit(t,"int ci%s=%u%s;\n",name,f(w,1,13,8),f(w,3,1,1)?"+a0":"");
        snprintf(base,sizeof base,"c[(ci%s>=0&&ci%s<192)?ci%s:0]",name,name,name);
    }
    emit(t,"precise float4 %s=%s%s.%c%c%c%c;\n",name,neg?"-":"",base,
         xyzw[(sw>>6)&3],xyzw[(sw>>4)&3],xyzw[(sw>>2)&3],xyzw[sw&3]);
}
static void masked(Text *t, const char *dst, const char *src, uint32_t mask) {
    const char *s="xyzw"; for(int i=0;i<4;i++) if(mask&(8u>>i)) emit(t,"%s.%c=%s.%c;\n",dst,s[i],src,s[i]);
}
static void temp_write(Text *t,uint32_t reg,const char *value,uint32_t mask) {
    char d[24]; if(reg==12) snprintf(d,sizeof d,"opos");
    else if(reg<12) snprintf(d,sizeof d,"r[%u]",reg); else return;
    masked(t,d,value,mask);
}
int nv2a_vsh_context_writes(const uint32_t program[NV2A_VSH_SLOTS][4], uint32_t start, int enabled) {
    if (!enabled || !program || start >= NV2A_VSH_SLOTS) return 0;
    for (uint32_t slot=start;slot<NV2A_VSH_SLOTS;slot++) {
        const uint32_t *w=program[slot];
        if(f(w,3,12,4) && !f(w,3,11,1) && f(w,3,3,8)<NV2A_VSH_CONSTANTS) return 1;
        if(f(w,3,0,1)) break;
    }
    return 0;
}
size_t nv2a_vsh_translate(const uint32_t program[NV2A_VSH_SLOTS][4],uint32_t start,
                          int context_write,char *out,size_t cap,char *error,size_t ecap) {
    Text t={out,cap,0,0}; uint32_t end=start; int found=0;
    int serial=nv2a_vsh_context_writes(program,start,context_write);
    if(error&&ecap) error[0]=0;
    if(!out||!cap||!program||start>=NV2A_VSH_SLOTS) {
        if(error&&ecap) snprintf(error,ecap,"invalid program, start or source buffer"); return 0;
    }
    out[0]=0;
    for(end=start;end<NV2A_VSH_SLOTS;end++) {
        if(f(program[end],1,21,4)>13) { if(error&&ecap) snprintf(error,ecap,"unsupported MAC opcode %u at slot %u",f(program[end],1,21,4),end); return 0; }
        if(f(program[end],3,0,1)) { found=1;break; }
    }
    if(!found) { if(error&&ecap) snprintf(error,ecap,"no FINAL instruction from slot %u",start); return 0; }
    emit(&t,"cbuffer Constants:register(b0){float4 %s[192];uint4 meta;}\n"
            "struct Input{float4 attr[16];};struct Output{float4 pos,d0,d1,fog;float4 tex[4];};\n"
            "StructuredBuffer<Input> inputs:register(t0);RWStructuredBuffer<Output> outputs:register(u0);\n",serial?"initial_c":"c");
    if(serial)emit(&t,"RWStructuredBuffer<float4> final_c:register(u1);\n");
    emit(&t,"float4 rcc(float x){float v=1.0/x;if(abs(v)<5.42101e-20)v=v<0?-5.42101e-20:5.42101e-20;"
            "if(abs(v)>1.884467e19)v=v<0?-1.884467e19:1.884467e19;return v.xxxx;}\n"
            "float4 logv(float x){float ax=abs(x);if(ax==0)return float4(asfloat(0xff800000),1,asfloat(0xff800000),1);"
            "float e=floor(log2(ax));return float4(e,ax/exp2(e),log2(ax),1);}\n"
            "float4 litv(float4 x){float nl=x.x>0?x.x:0;float nh=x.y>0?x.y:0;float p=x.w<-127.9961?-127.9961:(x.w>127.9961?127.9961:x.w);"
            "return float4(1,nl,x.x>0?pow(nh,p):0,1);}\n"
            "[numthreads(%u,1,1)]void main(uint3 tid:SV_DispatchThreadID){\n",serial?1:64);
    if(serial)emit(&t,"precise float4 c[192];[unroll]for(uint k=0;k<192;k++)c[k]=initial_c[k];\n[loop]for(uint vi=0;vi<meta.x;vi++){\n");
    else emit(&t,"uint vi=tid.x;if(vi>=meta.x)return;\n");
    emit(&t,"float4 r[12];[unroll]for(uint k=0;k<12;k++)r[k]=0;float4 opos=0;"
            "float4 o[13];[unroll]for(uint k=0;k<13;k++)o[k]=0;"
            "o[3].w=o[4].w=1;o[9].w=o[10].w=o[11].w=o[12].w=1;int a0=0;\n");
    for(uint32_t s=start;s<=end;s++) {
        const uint32_t *w=program[s]; uint32_t mac=f(w,1,21,4),ilu=f(w,1,25,3),dst=f(w,3,20,4);
        emit(&t,"{ // slot %u\n",s);source(&t,w,0,"A");source(&t,w,1,"B");source(&t,w,2,"C");
        emit(&t,"precise float4 m=0,i=0;\n");
        switch(mac) {
        case 0:break;case 1:emit(&t,"m=A;\n");break;case 2:emit(&t,"m=A*B;\n");break;
        case 3:emit(&t,"m=A+C;\n");break;case 4:emit(&t,"m=A*B+C;\n");break;
        case 5:emit(&t,"m=(A.x*B.x+A.y*B.y+A.z*B.z).xxxx;\n");break;
        case 6:emit(&t,"m=(A.x*B.x+A.y*B.y+A.z*B.z+B.w).xxxx;\n");break;
        case 7:emit(&t,"m=(A.x*B.x+A.y*B.y+A.z*B.z+A.w*B.w).xxxx;\n");break;
        case 8:emit(&t,"m=float4(1,A.y*B.y,A.z,B.w);\n");break;
        case 9:case 10:case 11:case 12:
            for(int q=0;q<4;q++) {char ch="xyzw"[q]; if(mac==9||mac==10)emit(&t,"m.%c=A.%c%sB.%c?A.%c:B.%c;\n",ch,ch,mac==9?"<":">=",ch,ch,ch);
                else emit(&t,"m.%c=A.%c%sB.%c?1:0;\n",ch,ch,mac==11?"<":">=",ch); } break;
        case 13:emit(&t,"m=A;a0=(int)floor(A.x+0.001);\n");break;
        }
        switch(ilu) {
        case 0:break;case 1:emit(&t,"i=C;\n");break;
        case 2:emit(&t,"i=(C.x==0?asfloat(0x7f800000):1.0/C.x).xxxx;\n");break;
        case 3:emit(&t,"i=rcc(C.x);\n");break;case 4:emit(&t,"i=(1.0/sqrt(abs(C.x))).xxxx;\n");break;
        case 5:emit(&t,"float fl=floor(C.x);i=float4(exp2(fl),C.x-fl,exp2(C.x),1);\n");break;
        case 6:emit(&t,"i=logv(C.x);\n");break;case 7:emit(&t,"i=litv(C);\n");break;
        }
        if(mac&&mac!=13)temp_write(&t,dst,"m",f(w,3,24,4));
        if(ilu)temp_write(&t,mac?1:dst,"i",f(w,3,16,4));
        if(f(w,3,12,4)) {
            char d[32]; uint32_t addr=f(w,3,3,8); int write=1;
            if(!f(w,3,11,1)) { if(serial && addr<192)snprintf(d,sizeof d,"c[%u]",addr);else write=0; }
            else if(!addr)snprintf(d,sizeof d,"opos");else if(addr<13)snprintf(d,sizeof d,"o[%u]",addr);else write=0;
            if(write)masked(&t,d,f(w,3,2,1)?"i":"m",f(w,3,12,4));
        }
        emit(&t,"}\n");
    }
    emit(&t,"Output result;result.pos=opos;result.d0=o[3];result.d1=o[4];result.fog=o[5];"
            "[unroll]for(uint k=0;k<4;k++)result.tex[k]=o[9+k];outputs[vi]=result;\n");
    if(serial)emit(&t,"}\n[unroll]for(uint k=0;k<192;k++)final_c[k]=c[k];\n");
    emit(&t,"}\n");
    if(t.failed) { out[0]=0;if(error&&ecap)snprintf(error,ecap,"HLSL output buffer too small");return 0; }
    return t.len;
}
