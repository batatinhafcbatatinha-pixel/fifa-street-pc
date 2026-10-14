/* Port of nv2a_combiner.c: register encoding and simultaneous read/write
 * semantics match the portable CPU reference. Included at build time as C
 * string literals in nv2a_gpu_combiner.inc; no runtime source-file dependency. */
float mapIn(float x,uint b){float p=max(x,0);switch((b>>5)&7){
case 0:return p;case 1:return 1-saturate(x);case 2:return 2*p-1;
case 3:return -2*p+1;case 4:return p-0.5;case 5:return -p+0.5;
case 6:return x;default:return -x;}}
float3 readRgb(float4 r[16],uint b){float4 v=r[b&15];float3 x=(b&16)?v.www:v.xyz;
return float3(mapIn(x.x,b),mapIn(x.y,b),mapIn(x.z,b));}
float readAlpha(float4 r[16],uint b){float4 v=r[b&15];return mapIn((b&16)?v.w:v.z,b);}
float4 mapOut(float4 x,uint m){switch(m){case 8:return x-0.5;case 16:return x*2;
case 24:return (x-0.5)*2;case 32:return x*4;case 48:return x*0.5;default:return x;}}
uint word(uint4 a[2],uint s){return a[s/4][s%4];}
float4 evalCombiner(float4 v0,float4 v1,float4 fogIn,float4 t[4]){
float4 r[16];[unroll]for(uint k=0;k<16;k++)r[k]=0;
r[3]=fogIn;r[4]=v0;r[5]=v1;[unroll]for(uint k=0;k<4;k++)r[8+k]=t[k];
r[12].w=(rcControl.y&31)?t[0].w:1;
uint flags=rcControl.x>>8,n=min(rcControl.x&255,8);
[loop]for(uint s=0;s<n;s++){
uint icw=word(rcColorIcw,s),ocw=word(rcColorOcw,s);
uint aicw=word(rcAlphaIcw,s),aocw=word(rcAlphaOcw,s),fl=ocw>>12,afl=aocw>>12;
r[1]=unpack(word(rcFactor0,(flags&16)?s:0));r[2]=unpack(word(rcFactor1,(flags&256)?s:0));
bool mux=(flags&1)?r[12].w>=0.5:(((int)(r[12].w*255)&1)!=0);
float3 A=readRgb(r,icw>>24),B=readRgb(r,icw>>16),C=readRgb(r,icw>>8),D=readRgb(r,icw);
float aa=readAlpha(r,aicw>>24),ab=readAlpha(r,aicw>>16),ac=readAlpha(r,aicw>>8),ad=readAlpha(r,aicw);
precise float3 pab=(fl&2)?dot(A,B).xxx:A*B,pcd=(fl&1)?dot(C,D).xxx:C*D;
float3 sum=(fl&4)?(mux?pcd:pab):pab+pcd;
float3 rab=clamp(mapOut(float4(pab,0),fl&56).xyz,-1,1);
float3 rcd=clamp(mapOut(float4(pcd,0),fl&56).xyz,-1,1);
float3 rms=clamp(mapOut(float4(sum,0),fl&56).xyz,-1,1);
precise float apab=aa*ab,apcd=ac*ad;
float asum=(afl&4)?(mux?apcd:apab):apab+apcd;
float aab=clamp(mapOut(apab.xxxx,afl&56).x,-1,1);
float acd=clamp(mapOut(apcd.xxxx,afl&56).x,-1,1);
float ams=clamp(mapOut(asum.xxxx,afl&56).x,-1,1);
uint dst=(ocw>>4)&15;if(dst){r[dst].xyz=rab;if(fl&128)r[dst].w=rab.z;}
dst=ocw&15;if(dst){r[dst].xyz=rcd;if(fl&64)r[dst].w=rcd.z;}
dst=(ocw>>8)&15;if(dst)r[dst].xyz=rms;
dst=(aocw>>4)&15;if(dst)r[dst].w=aab;dst=aocw&15;if(dst)r[dst].w=acd;
dst=(aocw>>8)&15;if(dst)r[dst].w=ams;r[0]=0;
}
if(rcFinal.x||rcFinal.y){uint f0=rcFinal.x,f1=rcFinal.y,ff=f1&255;
r[1]=unpack(rcFinal.z);r[2]=unpack(rcFinal.w);
float3 a=(ff&64)?1-r[5].xyz:r[5].xyz,b=(ff&32)?1-r[12].xyz:r[12].xyz;
r[14]=float4((ff&128)?saturate(a+b):a+b,0);
float3 E=readRgb(r,f1>>24),F=readRgb(r,f1>>16);r[15]=float4(E*F,0);
float3 A=readRgb(r,f0>>24),B=readRgb(r,f0>>16),C=readRgb(r,f0>>8),D=readRgb(r,f0);
return saturate(float4(D+C*(1-A)+B*A,readAlpha(r,f1>>8)));}
return saturate(r[12]);}
float4 atlasTexel(uint st,int2 p){uint4 inf=stageInfo[st];
if(!inf.x)return 1;
p=int2(address(p.x,inf.x,(inf.w&16)?1:0),address(p.y,inf.y,(inf.w&32)?1:0));
return unpack(texels[inf.z+p.y*inf.x+p.x]);}
float4 stageTexture(uint st,float2 uv){uint4 inf=stageInfo[st];
if(!inf.x)return 1;
if(inf.w&4)uv*=float2(inf.xy);
if(inf.w&8){float2 f=uv-0.5;int2 p=(int2)floor(f);float2 w=f-p;
return (atlasTexel(st,p)*(1-w.x)+atlasTexel(st,p+int2(1,0))*w.x)*(1-w.y)
+(atlasTexel(st,p+int2(0,1))*(1-w.x)+atlasTexel(st,p+1)*w.x)*w.y;}
return atlasTexel(st,(int2)floor(uv));}
bool stageFetch(uint st,float4 c,out float4 o){uint mode=(rcControl.y>>(st*5))&31;
float q=c.w!=0?c.w:1;
if(!mode){o=float4(0,0,0,1);return true;}
if(mode==1||mode==2){o=stageTexture(st,divide2(c.xy,q));return true;}
if(mode==3){o=stageTexture(st,c.xy);return true;}
if(mode==4){o=saturate(c);return true;}
o=0;if(mode==5){[unroll]for(uint j=0;j<4;j++){
bool ge=((rcmeta.z>>(st*4+j))&1)!=0;if(ge?c[j]>=0:c[j]<0)return false;}}
return true;}
