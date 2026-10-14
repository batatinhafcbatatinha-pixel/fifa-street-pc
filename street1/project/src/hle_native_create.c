#include <stdint.h>
#include <string.h>
#include "hle_win32.h"
#include "recomp/gen/recomp_types.h"
#include "hle_d3d11.h"

uint32_t hle_guest_call(uint32_t, int, const uint32_t *);
uint32_t hle_resource_format(uint32_t);
uint32_t hle_resource_pitch(uint32_t, uint32_t);
uint32_t hle_resource_layout(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,int,int,uint32_t*,uint32_t*);
void hle_resource_surface_header(uint32_t,uint32_t,uint32_t,uint32_t);
void hle_resource_addref(uint32_t);
void hle_resource_private_release(uint32_t);
void hle_state_render(uint32_t,uint32_t);
void hle_state_texture(uint32_t,uint32_t,uint32_t);
void hle_state_transform(uint32_t,uint32_t);
void hle_native_drain(void);
void hle_native_wait_idle(void);
void hle_api_set_vertex_shader(uint32_t);
void hle_api_set_render_target(uint32_t,uint32_t);
void hle_api_set_texture(uint32_t,uint32_t);

#define DEV 0x261240u
#define MP 0x262E68u
#define D(i) MEM32(DEV + 4u*(i))
#define M(i) MEM32(MP + 4u*(i))

uint64_t hle_native_clock_ns(void)
{
    LARGE_INTEGER ticks, frequency;
    QueryPerformanceCounter(&ticks);
    QueryPerformanceFrequency(&frequency);
    uint64_t counter = (uint64_t)ticks.QuadPart, hz = (uint64_t)frequency.QuadPart;
    /* PTIMER's shim uses the QPC origin, and the XDK timer unit is ns. */
    return (counter / hz) * 1000000000ull + (counter % hz) * 1000000000ull / hz;
}

static uint32_t alloc(uint32_t bytes,uint32_t flags)
{ uint32_t a[2]={bytes,flags}; return hle_guest_call(0x1609A7u,2,a); }
static void free_memory(uint32_t p,uint32_t flags)
{ if(p){ uint32_t a[2]={p,flags}; hle_guest_call(0x160A47u,2,a); } }
static uint32_t call(uint32_t va,int n,const uint32_t *a,uint32_t self)
{ uint32_t ecx=g_ecx; g_ecx=self; uint32_t v=hle_guest_call(va,n,a); g_ecx=ecx; return v; }
static void routed(uint32_t va,int n,const uint32_t *a)
{ hle_guest_call(va,n,a); }

static void tile(uint32_t index,uint32_t flags,uint32_t data,uint32_t bytes,uint32_t pitch)
{
    uint32_t t=DEV+0x1AC0u+index*24u;
    MEM32(t)=flags; MEM32(t+4)=data; MEM32(t+8)=bytes; MEM32(t+12)=pitch;
    MEM32(t+16)=0; MEM32(t+20)=0;
}

static void teardown_surfaces(void)
{
    if(!call(0x80000001u,0,0,0)){uint32_t a[4]={D(0x70A),9,1,0};call(0x80000002u,4,a,0);}
    for(uint32_t i=0;i<2;i++) {
        uint32_t at=DEV+0x1A04u+4*i,r=MEM32(at);
        if(r) {
            uint32_t parent=MEM32(r+20);
            MEM32((parent?parent:r)+8)=D(11);
            hle_resource_private_release(r); MEM32(at)=0;
        }
    }
    for(uint32_t i=0;i<D(0x684);i++) {
        uint32_t at=DEV+0x1A14u+4*i,r=MEM32(at);
        if(r){ routed(0x257CA0u,1,&r); MEM32(at)=0; }
    }
    if(D(0x688)){ uint32_t r=D(0x688); routed(0x257CA0u,1,&r); D(0x688)=0; }
    tile(0,0,0,0,0); tile(1,0,0,0,0);
    for(uint32_t i=0x6A1;i<=0x6A3;i++){ free_memory(D(i),0xA1800000u); D(i)=0; }
}

static uint32_t surface_pitch(uint32_t r)
{
    uint32_t sz=MEM32(r+16),f=MEM8(r+13),width=1u<<((MEM32(r+12)>>20)&15);
    if(sz)return ((sz>>24)+1)*64;
    return f==12?width*2:f==14||f==15?width*4:width*(MEM8(0x25ED88u+f)&0x3Cu)>>3;
}

static uint32_t surfaces(uint32_t p)
{
    uint32_t back=MEM32(p+12); if(!back)back=1;
    uint32_t color=hle_resource_format(MEM32(p+8)),depth=hle_resource_format(MEM32(p+36));
    uint32_t width=MEM32(p),height=MEM32(p+4),sample=MEM32(p+16),swap=MEM32(p+20);
    uint32_t scaled=(sample&0x3000u)||swap==3;
    MEM32(0x26119Cu)=MEM32(p+48); D(0x65C)=swap; D(0x684)=back+1;
    D(2)=(D(2)&~0x4000u)|(scaled?0x4000u:0); MEM32(0x261208u)=0;
    if(!sample)sample=0x11;
    uint32_t xs=(sample>>4)&15,ys=sample&15;
    D(0x65B)=sample; float xf=(float)xs,yf=(float)ys;
    memcpy((void*)XBOX_PTR(DEV+0x96Cu),&xf,4); memcpy((void*)XBOX_PTR(DEV+0x970u),&yf,4);
    if(sample&0x1000u)MEM32(0x261208u)=(uint8_t)sample==0x21?1:2;
    MEM32(0x261198u)=(sample&0x100u)?4:(sample&0x200u)?5:2;
    if(MEM32(p+52)) {
        for(uint32_t i=0;i<back+1;i++) {
            uint32_t r=DEV+0x1A24u+i*24;
            memcpy((void*)XBOX_PTR(r),(void*)XBOX_PTR(MEM32(p+52+i*4)),24);
            D(0x685+i)=r;
        }
        if(MEM32(p+64)){ memcpy((void*)XBOX_PTR(DEV+0x1A6Cu),(void*)XBOX_PTR(MEM32(p+64)),24); D(0x688)=DEV+0x1A6Cu; }
    } else {
        uint32_t fmt=color;
        switch(MEM32(p+16)&0xF0000u){case 0x10000:fmt=0x1C;break;case 0x20000:fmt=0x11;break;case 0x30000:fmt=0x1E;break;case 0x40000:fmt=0x12;break;}
        uint32_t w=width*xs,h=height*ys,pitch=hle_resource_pitch(w,fmt),fw,sz;
        uint32_t bytes=hle_resource_layout(w,h,1,1,fmt,pitch,0,0,&fw,&sz);
        uint32_t count=scaled?1:back+1,total=((count*bytes+pitch-1)/pitch)*pitch;
        while(total&0x3FFFu)total+=pitch;
        uint32_t data=alloc(total,0xBE800000u); if(!data)return 0x8007000Eu;
        D(0x6A1)=data;
        for(uint32_t i=0;i<count;i++){ uint32_t r=DEV+0x1A24u+i*24; D(0x685+i)=r; hle_resource_surface_header(r,fw,sz,data+i*bytes); }
        if(w!=1920)tile(0,0,data,total,pitch);
        if(scaled) {
            bytes=hle_resource_layout(width,height,1,1,color,0,0,0,&fw,&sz);
            data=alloc(bytes*back,0xBE800000u); if(!data)return 0x8007000Eu;
            D(0x6A2)=data;
            for(uint32_t i=0;i<back;i++){ uint32_t r=DEV+0x1A3Cu+i*24; D(0x686+i)=r; hle_resource_surface_header(r,fw,sz,data+i*bytes); }
        }
        if(MEM32(p+32)) {
            pitch=hle_resource_pitch(w,depth); bytes=hle_resource_layout(w,h,1,1,depth,pitch,0,0,&fw,&sz);
            total=((bytes+pitch-1)/pitch)*pitch; while(total&0x3FFFu)total+=pitch;
            data=alloc(total,0xBE800000u); if(!data)return 0x8007000Eu;
            D(0x6A3)=data; D(0x688)=DEV+0x1A6Cu; hle_resource_surface_header(D(0x688),fw,sz,data);
            if(w!=1920)tile(1,(MEM8(0x25ED88u+depth)&0x3Cu)==32?0x84000001u:0x80000001u,data,total,pitch);
        }
    }
    for(uint32_t i=0;i<D(0x684);i++)hle_resource_addref(D(0x685+i));
    if(D(0x688))hle_resource_addref(D(0x688));
    return 0;
}

static uint32_t display_mode(uint32_t p,uint32_t pitch)
{
    uint32_t encoder=MEM32(0x260D88u);
    if(!encoder){ uint32_t a[4]={0,6,0,0x260D88u}; call(0x80000002u,4,a,0); encoder=MEM32(0x260D88u); }
    uint32_t table=0x25F4E8u,i=0;
    while(i<185 && (MEM32(table)&0xFF00u)!=(encoder&0xFF00u)){ table+=12;i++; }
    while(i<185 && (MEM32(table)&255u) && (MEM32(table)&255u)!=(encoder&255u)){table+=12;i++;}
    uint32_t initial=MEM32(table),flags=MEM32(p+40),refresh=MEM32(p+44),hz=encoder&0xC00000u;
    uint32_t scan=flags&0x60u;if((int8_t)flags<0)scan=flags&0x40u;
    if(refresh==50 && (flags&0x200u) && (encoder&0x400000u))hz=0x400000u;
    else if(refresh==60)hz=(flags&0x200u)&&(encoder&0x800000u)?0x800000u:0x400000u;
    uint32_t mode=0;
    if(encoder&255u) {
        for(;i<185 && (MEM32(table)&255u)==(initial&255u);i++,table+=12) {
            uint32_t v=MEM32(table),dims=MEM32(table+4);
            if((encoder&255u)==4 && (v&0xE0000u) && !(v&encoder&0xE0000u))continue;
            if((dims&65535u)!=MEM32(p) || (dims>>16)!=MEM32(p+4))continue;
            if(((v>>16)^(flags>>4))&1u)continue;
            if(((v>>24)^(flags>>7))&1u)continue;
            if(((v>>25)^(flags>>8))&1u)continue;
            if(!(v&hz))continue;
            if(scan && !((scan&0x20u)&&(v&0x200000u)) && !((scan&0x40u)&&!(v&0x200000u)))continue;
            mode=MEM32(table+8);
            if(!(encoder&hz))mode=0;
            if(((encoder&255u)!=4 || !(v&0x60000u)) && (flags&0x10u) && !(encoder&0x10000u))mode=0;
            break;
        }
        if(!mode)return 0x80004005u;
    }
    M(3)=hle_resource_format(MEM32(p+8)); M(1)=pitch; M(2)=mode;
    M(0x1F7+M(0x1F9))=1; M(0x6E)=1; M(0x6D)=MEM32(table);
    if((refresh==50 && hz==0x400000u && (flags&0x200u)) || (refresh==60 && hz==0x800000u && (flags&0x200u))) {
        uint32_t delta=refresh==50?20000000u:0xFE9156u;
        uint64_t due=hle_native_clock_ns()+delta;
        M(0x208)=delta;M(0x209)=0;M(0x20A)=(uint32_t)due;M(0x20B)=(uint32_t)(due>>32);
        uint32_t slot=M(0x211)==0?1:0;M(0x20C+slot*2)=(uint32_t)due;M(0x20D+slot*2)=(uint32_t)(due>>32);
        uint64_t first=((uint64_t)M(0x20D)<<32)|M(0x20C),second=((uint64_t)M(0x20F)<<32)|M(0x20E);
        M(0x210)=first && (!second || first<second)?0:1;M(0x206)=1;
    }
    return 0;
}

static void defaults(void)
{
    for(uint32_t i=0;i<10;i++)hle_state_transform(i,0x25EEF8u);
    for(uint32_t i=0x39;i<0xA6;i++)if(i!=0x9A&&i!=0x7E&&i!=0x7F)hle_state_render(i,MEM32(0x25F328u+(i-0x39)*4));
    hle_state_render(0x8F,D(0x688)!=0);
    for(uint32_t stage=0;stage<4;stage++) {
        for(uint32_t i=0;i<32;i++)hle_state_texture(stage,i,MEM8(0x25F308u+i));
        hle_state_texture(stage,28,stage);hle_api_set_texture(stage,0);
    }
    hle_state_texture(0,12,4);hle_state_texture(0,16,2);
    uint32_t a[2]={0,0};routed(0x2541D0u,2,a);
}

static uint32_t startup(uint32_t p)
{
    uint32_t mem=alloc(96,0xAC800000u);D(0x922)=mem;if(!mem)return 0x8007000Eu;
    D(12)=mem;D(0x923)=mem+64;D(0x924)=mem+32;memset((void*)XBOX_PTR(mem+32),0,64);
    mem=alloc(MEM32(0x2639ECu),0xBC800000u);D(9)=mem;if(!mem)return 0x8007000Eu;
    D(10)=mem+(MEM32(0x2639ECu)&~3u);D(0)=mem;D(1)=mem-0x204u+(MEM32(0x2639E8u)&~3u);D(11)=5;MEM32(D(12))=3;
    uint32_t n=(MEM32(0x2639ECu)-1)/MEM32(0x2639E8u),shift=0;do{n>>=1;shift++;}while(n);
    D(14)=(1u<<shift)-1;D(18)=alloc((1u<<shift)*8,0x64800000u);if(!D(18))return 0x8007000Eu;
    MEM32(0x260D98u)|=0xFF7F7Fu;D(0x253)=64;
    uint32_t dpc[3]={MP+0x84u,0x259B40u,MP};call(MEM32(0x277154u),3,dpc,0);
    M(0x6C)=MP+0x1ACu;M(0x6B)=MP+0x1ACu;M(0x67)=MP+0x19Cu;M(0x68)=MP+0x19Cu;
    MEM8(MP+0x1A4)=0;MEM8(MP+0x1A6)=4;M(0x6A)=1;MEM8(MP+0x194)=0;MEM8(MP+0x196)=4;M(0x66)=1;
    call(0x259D01u,0,0,MP);call(0x259D2Cu,0,0,MP);
    uint32_t scratch=g_esp+0x20u;
    uint32_t irq[2]={3,scratch};uint32_t vector=call(MEM32(0x277230u),2,irq,0);
    uint32_t interrupt[7]={MP+20,0x259190u,MP,vector,MEM32(scratch),0,1};call(MEM32(0x27723Cu),7,interrupt,0);
    uint32_t interrupt_object=MP+20;call(MEM32(0x277238u),1,&interrupt_object,0);
    M(0x59)=0x2595D0u;M(0x5A)=0;uint32_t shutdown[2]={MP+0x164u,1};call(MEM32(0x2771E4u),2,shutdown,0);
    call(0x259D6Du,0,0,MP);call(0x259DF0u,0,0,MP);
    for(uint32_t ramp=0;ramp<2;ramp++)for(uint32_t i=0;i<0x300;i++)MEM8(MP+0x1DCu+ramp*0x300u+i)=(uint8_t)i;
    M(0x28)=1;
    const uint32_t handles[]={3,5,4,9,10,11,2,7,12,8,6};
    const uint32_t classes[]={0x3D,2,3,0x3D,0x3D,0x3D,3,0x3D,0x3D,0x3D,2};
    for(uint32_t i=0;i<11;i++){
        uint32_t base=i==6?D(0x924):i==7?D(0x923):i==8?0x80000000u:i==9?D(12):0;
        uint32_t limit=i==6||i==7?31:i==8?0x10000000u:i==9?32:0x7FFAFFFu;
        uint32_t a[5]={handles[i],classes[i],base,limit,scratch+i*16};call(0x259EE2u,5,a,MP);
        if(i>=3&&i<=5)D(0x91F+i-3)=MEM32(scratch+i*16+12);
    }
    uint32_t a[5]={0x206E,0,scratch+160,0,scratch+176};call(0x259C6Au,5,a,MP);D(0x708)=MEM32(scratch+176);if(!D(0x708))return 0x8007000Eu;
    for(uint32_t i=0;i<11;i++){uint32_t out=scratch+i*16;call(0x259CCDu,1,&out,MP);}
    const uint32_t gr[][2]={{13,0x97},{14,0x39},{16,0x9F},{17,0x62},{18,0x44},{19,0x57},{20,0x43},{21,0x12},{22,0x72},{24,0x19},{25,0x30}};
    for(uint32_t i=0;i<11;i++){uint32_t b[3]={gr[i][0],gr[i][1],scratch};call(0x259FB1u,3,b,MP);}
    MEM32(0x260D8Cu)=D(0x70A);D(0x24D)=D(0x70A);
    memset((void*)XBOX_PTR(DEV+0x192Cu),0,64);D(0x64C)=3;
    uint32_t zero=0;routed(0x254E70u,1,&zero);
    uint32_t result=surfaces(p);if((int32_t)result<0)return result;
    result=display_mode(p,surface_pitch(D(0x686)));if((int32_t)result<0)return result;
    D(0x1E5)=0x2638D0u;hle_api_set_vertex_shader(4);hle_api_set_vertex_shader(2);
    memset((void*)XBOX_PTR(0x2637B8u),0,0x118);MEM32(0x2637BCu)=16;
    hle_api_set_render_target(D(0x685),D(0x688));defaults();
    uint32_t clear[6]={0,0,3,0,0x3F800000u,0};routed(0x258D70u,6,clear);
    if(XBOX_FS_BASE) {
        uint32_t thread=MEM32(XBOX_FS_BASE+32),tls=thread?MEM32(thread+0x250u):0,debug=tls?MEM32(tls+32):0;
        if(debug){MEM32(debug)=DEV+0x1DE4u;MEM32(debug+4)=DEV+0x2478u;MEM32(debug+8)=D(0x686);MEM32(debug+12)=MP;
            MEM32(debug+20)=DEV+0x8D4u;MEM32(debug+24)=DEV+0x8DCu;MEM32(debug+28)=0x254220u;MEM32(debug+32)=0xABCDEF00u;}
    }
    uint32_t encoder_option[4]={D(0x70A),11,5,0};call(0x80000002u,4,encoder_option,0);
    encoder_option[1]=14;encoder_option[2]=0;call(0x80000002u,4,encoder_option,0);
    MEM32(0x2636F0u)=1;MEM32(0x2636E8u)=5;MEM32(0x2636ECu)=1;MEM32(0x2636E4u)=0;
    return 0;
}

static void cleanup(void)
{
    if(D(0)) { hle_native_drain(); hle_native_wait_idle(); }
    teardown_surfaces();free_memory(D(9),0xA1800000u);free_memory(D(18),0x24800000u);
    uint32_t offsets[]={0x7A0,0xF88,0xF8C,0xF90,0xF94,0xF98,0xF9C,0xFA0,0xFA4};
    for(uint32_t i=0;i<9;i++){uint32_t r=MEM32(DEV+offsets[i]);if(r)hle_resource_private_release(r);}
    for(uint32_t i=0;i<16;i++){uint32_t r=MEM32(0x263700u+i*12);if(r)hle_resource_private_release(r);}
    for(uint32_t i=0;i<64;i++)free_memory(D(0x1F5+i),0xA1800000u);
    free_memory(D(0x922),0xA1800000u);free_memory(D(0x1E9),0x24800000u);free_memory(D(0x708),0xA1800000u);
    M(0x206)=0;uint32_t shutdown[2]={MP+0x164u,0};call(0x8000002Fu,2,shutdown,0);uint32_t interrupt=MP+20;call(0x80000064u,1,&interrupt,0);
    memset((void*)XBOX_PTR(0x2636F8u),0,0xC0);
}

int hle_create_call(int id,const uint32_t *a)
{
    uint32_t result;
    if(id==HLE_Direct3D_CreateDevice){
        extern int hle_device_native(void);
        if(!hle_device_native())return 0;
        if(!MEM32(0x2639ECu))MEM32(0x2639ECu)=0x80000;
        if(!MEM32(0x2639E8u))MEM32(0x2639E8u)=0x8000;
        MEM32(0x261248u)|=a[3]&16u;MEM32(0x261238u)=DEV;MEM32(0x261B78u)=1;
        /* startup() keeps scratch records just above a lowered stack, out of
         * reach of the frames of the helpers it calls. */
        uint32_t esp0=g_esp; g_esp-=0x200u;
        result=startup(a[4]);
        g_esp=esp0;
        if((int32_t)result<0){cleanup();memset((void*)XBOX_PTR(DEV),0,0x24A0);MEM32(0x261238u)=0;if(a[5])MEM32(a[5])=0;}
        else if(a[5])MEM32(a[5])=DEV;
        g_eax=result;g_esp+=28;return 1;
    }
    if(id==HLE_D3DDevice_Reset){
        hle_native_drain(); hle_native_wait_idle();
        teardown_surfaces();result=surfaces(a[0]);
        if((int32_t)result<0)teardown_surfaces();
        else {
            result=display_mode(a[0],surface_pitch(D(0x686)));
            if((int32_t)result>=0){hle_api_set_render_target(D(0x685),D(0x688));uint32_t clear[6]={0,0,3,0,0x3F800000u,0};routed(0x258D70u,6,clear);result=0;}
        }
        g_eax=result;g_esp+=8;return 1;
    }
    return 0;
}
