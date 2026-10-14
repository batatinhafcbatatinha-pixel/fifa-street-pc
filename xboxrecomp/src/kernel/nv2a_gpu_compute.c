/* D3D11 compute performs raster coverage, shading, depth and blending.
 * Never uses WARP: successful dispatch means execution on a physical adapter.
 * nv2a_gpu_draw reads back per call (tests); the resident surface keeps the
 * render target on the GPU and returns it only at the caller's sync points.
 */
#include "nv2a_gpu_compute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define COBJMACROS
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <windows.h>
static ID3D11Device *dev;
static ID3D11DeviceContext *ctx;
static void vertex_cleanup(void);
static void vertex_report(void);
static ID3D11ComputeShader *shader;
static ID3D11Buffer *constants, *outbuf, *readbuf, *texbuf;
static ID3D11Buffer *surfbuf, *surfread, *statsbuf, *statsread, *tilestatsbuf,
    *tilestatsread;
static ID3D11ShaderResourceView *texview;
static ID3D11UnorderedAccessView *outview, *surfview, *statsview, *tilestatsview;
static size_t pixel_capacity, texture_capacity, surf_capacity;
static unsigned long long surface_uploads, surface_downloads;
static unsigned long long batches, batch_tiles, batch_entries;
static int flush_queue(void);
static void queue_cleanup(void);
static Nv2aGpuPixel *host_input, *host_output;
static uint32_t *host_texture, *atlas_snapshot;
static size_t host_input_capacity, host_output_capacity, host_texture_capacity,
    atlas_snapshot_capacity;
static unsigned long long atlas_uploads, atlas_upload_skips, atlas_upload_bytes,
    host_buffer_grows, readback_bytes, full_capacity_bytes;
typedef struct {
  size_t count;
} AtlasKey;
static AtlasKey last_atlas_key;
static int atlas_snapshot_valid;
static int initialized, enabled;
static unsigned long long draws, pixels, failures, fallbacks;
static struct {
  const char *reason;
  unsigned long long count;
} reasons[16];
static unsigned reason_count;
static const char source[] =
    /* One record per queued triangle (Nv2aGpuParams); the field names the
     * code below uses resolve to the triangle the thread is working on. */
    "struct Params{float4 m_pos[3];float4 m_col[3];float4 m_uvw[3];"
    "uint4 m_box;uint4 m_tex;uint4 m_blend;uint4 m_state;uint4 m_depth;uint4 m_alpha;"
    "float4 m_color1[3];float4 m_fog[3];float4 m_stageCoords[3][4];uint4 "
    "m_stageInfo[4];uint4 m_rcmeta;"
    "uint4 m_rcColorIcw[2],m_rcAlphaIcw[2],m_rcColorOcw[2],m_rcAlphaOcw[2];"
    "uint4 m_rcFactor0[2],m_rcFactor1[2];uint4 m_rcFinal,m_rcControl;uint4 m_surf;};\n"
    "cbuffer Batch:register(b0){uint4 batch;}"
    "StructuredBuffer<Params> tris:register(t2);"
    "StructuredBuffer<uint4> tiles:register(t3);"
    "StructuredBuffer<uint> order:register(t4);"
    "static uint g_t;\n"
    "#define pos (tris[g_t].m_pos)\n"
    "#define col (tris[g_t].m_col)\n"
    "#define uvw (tris[g_t].m_uvw)\n"
    "#define box (tris[g_t].m_box)\n"
    "#define tex (tris[g_t].m_tex)\n"
    "#define blend (tris[g_t].m_blend)\n"
    "#define state (tris[g_t].m_state)\n"
    "#define depth (tris[g_t].m_depth)\n"
    "#define alpha (tris[g_t].m_alpha)\n"
    "#define color1 (tris[g_t].m_color1)\n"
    "#define fog (tris[g_t].m_fog)\n"
    "#define stageCoords (tris[g_t].m_stageCoords)\n"
    "#define stageInfo (tris[g_t].m_stageInfo)\n"
    "#define rcmeta (tris[g_t].m_rcmeta)\n"
    "#define rcColorIcw (tris[g_t].m_rcColorIcw)\n"
    "#define rcAlphaIcw (tris[g_t].m_rcAlphaIcw)\n"
    "#define rcColorOcw (tris[g_t].m_rcColorOcw)\n"
    "#define rcAlphaOcw (tris[g_t].m_rcAlphaOcw)\n"
    "#define rcFactor0 (tris[g_t].m_rcFactor0)\n"
    "#define rcFactor1 (tris[g_t].m_rcFactor1)\n"
    "#define rcFinal (tris[g_t].m_rcFinal)\n"
    "#define rcControl (tris[g_t].m_rcControl)\n"
    "#define surf (tris[g_t].m_surf)\n"
    "struct Pixel{uint color;float z;uint covered;uint failed;};"
    "StructuredBuffer<uint> texels:register(t1);"
    "RWStructuredBuffer<Pixel> output:register(u0);"
    "RWStructuredBuffer<uint> stats:register(u1);\n"
    /* In place: surf.x is the row stride in samples, surf.yz the box
     * origin in the bound buffer, surf.w bit 0 counts zpass for reports. */
    "void put(uint i,Pixel o){output[i]=o;if(o.covered){"
    "InterlockedAdd(stats[0],1);if(surf.w&1)InterlockedAdd(stats[2],1);}"
    "if(o.failed)InterlockedAdd(stats[1],1);}"
    "float divideExact(float a,float b){return (float)((double)a/(double)b);}"
    /* One double reciprocal, then double products rounded once to float:
     * the same float as dividing each component, a fraction of the FP64. */
    "float2 divide2(float2 a,float b){double r=1.0/(double)b;return "
    "float2((float)((double)a.x*r),(float)((double)a.y*r));}"
    "float4 divide4(float4 a,float b){double r=1.0/(double)b;return "
    "float4((float)((double)a.x*r),(float)((double)a.y*r),"
    "(float)((double)a.z*r),(float)((double)a.w*r));}"
    /* Exact k/255, as the CPU reference divides; a table read instead of
     * four divisions per colour (FP64 here cost 10x on consumer GPUs). */
    "static const float kUnorm[256]={0,0.00392156886,0.00784313772,"
    "0.0117647061,0.0156862754,0.0196078438,0.0235294122,0.0274509806,"
    "0.0313725509,0.0352941193,0.0392156877,0.0431372561,0.0470588244,"
    "0.0509803928,0.0549019612,0.0588235296,0.0627451017,0.0666666701,"
    "0.0705882385,0.0745098069,0.0784313753,0.0823529437,0.0862745121,"
    "0.0901960805,0.0941176489,0.0980392173,0.101960786,0.105882354,"
    "0.109803922,0.113725491,0.117647059,0.121568628,0.125490203,0.129411772,"
    "0.13333334,0.137254909,0.141176477,0.145098045,0.149019614,0.152941182,"
    "0.156862751,0.160784319,0.164705887,0.168627456,0.172549024,0.176470593,"
    "0.180392161,0.184313729,0.188235298,0.192156866,0.196078435,0.200000003,"
    "0.203921571,0.20784314,0.211764708,0.215686277,0.219607845,0.223529413,"
    "0.227450982,0.23137255,0.235294119,0.239215687,0.243137255,0.247058824,"
    "0.250980407,0.254901975,0.258823544,0.262745112,0.266666681,0.270588249,"
    "0.274509817,0.278431386,0.282352954,0.286274523,0.290196091,0.294117659,"
    "0.298039228,0.301960796,0.305882365,0.309803933,0.313725501,0.31764707,"
    "0.321568638,0.325490206,0.329411775,0.333333343,0.337254912,0.34117648,"
    "0.345098048,0.349019617,0.352941185,0.356862754,0.360784322,0.36470589,"
    "0.368627459,0.372549027,0.376470596,0.380392164,0.384313732,0.388235301,"
    "0.392156869,0.396078438,0.400000006,0.403921574,0.407843143,0.411764711,"
    "0.41568628,0.419607848,0.423529416,0.427450985,0.431372553,0.435294122,"
    "0.43921569,0.443137258,0.447058827,0.450980395,0.454901963,0.458823532,"
    "0.4627451,0.466666669,0.470588237,0.474509805,0.478431374,0.482352942,"
    "0.486274511,0.490196079,0.494117647,0.498039216,0.501960814,0.505882382,"
    "0.509803951,0.513725519,0.517647088,0.521568656,0.525490224,0.529411793,"
    "0.533333361,0.53725493,0.541176498,0.545098066,0.549019635,0.552941203,"
    "0.556862772,0.56078434,0.564705908,0.568627477,0.572549045,0.576470613,"
    "0.580392182,0.58431375,0.588235319,0.592156887,0.596078455,0.600000024,"
    "0.603921592,0.607843161,0.611764729,0.615686297,0.619607866,0.623529434,"
    "0.627451003,0.631372571,0.635294139,0.639215708,0.643137276,0.647058845,"
    "0.650980413,0.654901981,0.65882355,0.662745118,0.666666687,0.670588255,"
    "0.674509823,0.678431392,0.68235296,0.686274529,0.690196097,0.694117665,"
    "0.698039234,0.701960802,0.70588237,0.709803939,0.713725507,0.717647076,"
    "0.721568644,0.725490212,0.729411781,0.733333349,0.737254918,0.741176486,"
    "0.745098054,0.749019623,0.752941191,0.75686276,0.760784328,0.764705896,"
    "0.768627465,0.772549033,0.776470602,0.78039217,0.784313738,0.788235307,"
    "0.792156875,0.796078444,0.800000012,0.80392158,0.807843149,0.811764717,"
    "0.815686285,0.819607854,0.823529422,0.827450991,0.831372559,0.835294127,"
    "0.839215696,0.843137264,0.847058833,0.850980401,0.854901969,0.858823538,"
    "0.862745106,0.866666675,0.870588243,0.874509811,0.87843138,0.882352948,"
    "0.886274517,0.890196085,0.894117653,0.898039222,0.90196079,0.905882359,"
    "0.909803927,0.913725495,0.917647064,0.921568632,0.925490201,0.929411769,"
    "0.933333337,0.937254906,0.941176474,0.945098042,0.949019611,0.952941179,"
    "0.956862748,0.960784316,0.964705884,0.968627453,0.972549021,0.97647059,"
    "0.980392158,0.984313726,0.988235295,0.992156863,0.996078432,1};"
    "float4 unpack(uint c){return float4(kUnorm[(c>>16)&255],"
    "kUnorm[(c>>8)&255],kUnorm[c&255],kUnorm[c>>24]);}"
    "uint pack(float4 c){uint4 "
    "b=(uint4)(saturate(c)*255.0+0.5);return(b.w<<24)|(b.x<<16)|(b.y<<8)|b.z;}"
    "bool compare(float a,float b,uint f){if(f==0x200)return "
    "false;if(f==0x201)return a<b;"
    "if(f==0x202)return a==b;if(f==0x203)return a<=b;if(f==0x204)return a>b;"
    "if(f==0x205)return a!=b;if(f==0x206)return a>=b;return true;}"
    "float4 factor(uint f,float4 s,float4 d){float4 c=unpack(state.y);"
    "if(f==0)return 0;if(f==1)return 1;if(f==0x300)return s;if(f==0x301)return "
    "1-s;"
    "if(f==0x302)return s.w;if(f==0x303)return 1-s.w;if(f==0x304)return d.w;"
    "if(f==0x305)return 1-d.w;if(f==0x306)return d;if(f==0x307)return 1-d;"
    "if(f==0x308)return float4(min(s.w,1-d.w).xxx,1);"
    "if(f==0x8001)return c;if(f==0x8002)return 1-c;if(f==0x8003)return c.w;"
    "if(f==0x8004)return 1-c.w;return 1;}"
    "int address(int c,uint n,uint mode){if(mode==1)return "
    "((c%(int)n)+(int)n)%(int)n;"
    "return clamp(c,0,(int)n-1);}"
#include "nv2a_gpu_combiner.inc"
    "void rasterOne(uint3 id){"
    "uint "
    "sx=max(depth.w,1),sy=max(alpha.w,1);if(id.x>=box.z*sx||id.y>=box.w*sy)"
    "return;"
    "uint i=(surf.z+id.y)*surf.x+surf.y+id.x;Pixel o=output[i];uint2 "
    "logical=uint2(id.x/sx,id.y/sy);"
    "uint sample=(id.y%sy)*sx+(id.x%sx);float2 offset=0.5;"
    "if(state.w){if(sx==2&&sy==1&&sample==1)offset=0;else "
    "if(sy==2)offset=float2((sample%2)?0.75:0.25,(sample/2)?0.75:0.25);}"
    "o.covered=0;o.failed=0;float2 p=float2(box.xy+logical)+offset;"
    "precise float "
    "w0=(pos[1].x-pos[0].x)*(p.y-pos[0].y)-(pos[1].y-pos[0].y)*(p.x-pos[0].x);"
    "precise float "
    "w1=(pos[2].x-pos[1].x)*(p.y-pos[1].y)-(pos[2].y-pos[1].y)*(p.x-pos[1].x);"
    "precise float "
    "w2=(pos[0].x-pos[2].x)*(p.y-pos[2].y)-(pos[0].y-pos[2].y)*(p.x-pos[2].x);"
    "if(!((w0>=0&&w1>=0&&w2>=0)||(w0<=0&&w1<=0&&w2<=0))){put(i,o);return;}"
    "precise float "
    "area=(pos[1].x-pos[0].x)*(pos[2].y-pos[0].y)-(pos[1].y-pos[0].y)*(pos[2]."
    "x-pos[0].x);"
    "precise float invArea=divideExact(1.0,area);precise float3 "
    "l=float3(w1,w2,w0)*invArea;precise float "
    /* Preserve a constant depth plane exactly; barycentric sum rounding
     * otherwise stipples coplanar frontend layers under LEQUAL. */
    "z=(pos[0].z==pos[1].z&&pos[0].z==pos[2].z)?pos[0].z:"
    "l.x*pos[0].z+l.y*pos[1].z+l.z*pos[2].z;"
    "if(depth.x&&!compare(z,o.z,depth.y)){o.failed=1;put(i,o);return;}"
    "precise float4 "
    "c=unpack(state.z);bool shade=(state.x&0x01010101)||alpha.x||!state.w;"
    "if(state.w)c=shade?l.x*col[0]+l.y*col[1]+l.z*col[2]:0;"
    "if(rcmeta.x&&shade){precise float "
    "q=l.x*uvw[0].z+l.y*uvw[1].z+l.z*uvw[2].z;float4 stages[4];"
    "[unroll]for(uint st=0;st<4;st++){precise float4 "
    "tc=divide4(l.x*stageCoords[0][st]+l.y*stageCoords[1][st]+l.z*stageCoords["
    "2][st],q);"
    "if(!stageFetch(st,tc,stages[st])){put(i,o);return;}}"
    "precise float4 d1=l.x*color1[0]+l.y*color1[1]+l.z*color1[2];"
    "precise float4 "
    "fg=float4(fog[0].xyz,l.x*fog[0].w+l.y*fog[1].w+l.z*fog[2].w);c="
    "evalCombiner(c,d1,fg,stages);"
    "}else if(tex.x&&shade&&!rcmeta.x){precise float "
    "q=state.w?(l.x*uvw[0].z+l.y*uvw[1].z+l.z*uvw[2].z):1;"
    "precise float2 uv=divide2(l.x*uvw[0].xy+l.y*uvw[1].xy+l.z*uvw[2].xy,q);"
    "if(!state.w)uv=divide2(w1*uvw[0].xy+w2*uvw[1].xy+w0*uvw[2].xy,area);"
    "int2 t=state.w?(int2)floor(uv):(int2)max(uv,0);"
    "t=int2(address(t.x,tex.x,tex.z),address(t.y,tex.y,tex.w));"
    "precise float4 tc=unpack(texels[t.y*tex.x+t.x]);c=state.w?tc*c:tc;}"
    "uint "
    "argb=pack(c);if(alpha.x&&!compare((argb>>24),alpha.z&255,alpha.y)){put(i,o);return;}"
    "if(depth.z)o.z=z;o.covered=1;"
    "if(blend.x&&!(blend.y==1&&blend.z==0)){float4 "
    "s=unpack(argb),d=unpack(o.color);"
    "precise float4 a=s*factor(blend.y,s,d),b=d*factor(blend.z,s,d),r=a+b;"
    "if(blend.w==0x800a)r=a-b;else if(blend.w==0x800b)r=b-a;"
    "else if(blend.w==0x8007)r=min(s,d);else "
    "if(blend.w==0x8008)r=max(s,d);argb=pack(r);}"
    "uint keep=0;if(!(state.x&0x01000000))keep|=0xff000000;"
    "if(!(state.x&0x00010000))keep|=0xff0000;if(!(state.x&0x00000100))keep|="
    "0xff00;"
    "if(!(state.x&1))keep|=0xff;o.color=(argb&~keep)|(o.color&keep);put(i,o);}"
    /* An 8x8 tile of the target per group: tl.x packs its origin (x | y<<16)
     * in samples, tl.y/tl.z the start and count of its triangles in order[].
     * Each thread visits the tile's triangles in submission order, so the
     * per-pixel order of blending and depth is the order they were drawn. */
    "[numthreads(8,8,1)]void main(uint3 gid:SV_GroupID,"
    "uint3 lt:SV_GroupThreadID){uint4 tl=tiles[batch.x+gid.x];"
    "int gx=(int)(tl.x&0xFFFF)+(int)lt.x,gy=(int)(tl.x>>16)+(int)lt.y;"
    "[loop]for(uint k=0;k<tl.z;k++){g_t=order[tl.y+k];"
    "int2 l=int2(gx,gy)-int2(surf.y,surf.z);"
    "if(l.x>=0&&l.y>=0)rasterOne(uint3(l,0));}}";

#define RELEASE(p, T)                                                          \
  do {                                                                         \
    if (p) {                                                                   \
      T##_Release(p);                                                          \
      p = NULL;                                                                \
    }                                                                          \
  } while (0)
static unsigned long long shader_source_hash(void) {
  unsigned long long hash = 14695981039346656037ULL;
  for (size_t i = 0; i < sizeof(source) - 1; i++) {
    hash ^= (unsigned char)source[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}
#ifndef NV2A_GPU_SHADER_GENERATOR
#include "nv2a_gpu_dxbc.inc"
#endif
#include "nv2a_gpu_profile.inc"
static void cleanup(void) {
  gpu_profile_cleanup();
  vertex_cleanup();
  queue_cleanup();
  if (ctx)
    ID3D11DeviceContext_ClearState(ctx);
  RELEASE(texview, ID3D11ShaderResourceView);
  RELEASE(outview, ID3D11UnorderedAccessView);
  RELEASE(surfview, ID3D11UnorderedAccessView);
  RELEASE(statsview, ID3D11UnorderedAccessView);
  RELEASE(tilestatsview, ID3D11UnorderedAccessView);
  RELEASE(tilestatsbuf, ID3D11Buffer);
  RELEASE(tilestatsread, ID3D11Buffer);
  RELEASE(surfbuf, ID3D11Buffer);
  RELEASE(surfread, ID3D11Buffer);
  RELEASE(statsbuf, ID3D11Buffer);
  RELEASE(statsread, ID3D11Buffer);
  pixel_capacity = texture_capacity = surf_capacity = 0;
  RELEASE(outbuf, ID3D11Buffer);
  RELEASE(readbuf, ID3D11Buffer);
  RELEASE(texbuf, ID3D11Buffer);
  RELEASE(constants, ID3D11Buffer);
  RELEASE(shader, ID3D11ComputeShader);
  RELEASE(ctx, ID3D11DeviceContext);
  RELEASE(dev, ID3D11Device);
  free(host_input);
  host_input = NULL;
  free(host_output);
  host_output = NULL;
  free(host_texture);
  host_texture = NULL;
  free(atlas_snapshot);
  atlas_snapshot = NULL;
  host_input_capacity = host_output_capacity = host_texture_capacity =
      atlas_snapshot_capacity = 0;
  atlas_snapshot_valid = 0;
}
void nv2a_gpu_get_stats(Nv2aGpuStats *stats) {
  if (!stats)
    return;
  stats->draws = draws;
  stats->atlas_uploads = atlas_uploads;
  stats->atlas_upload_skips = atlas_upload_skips;
  stats->atlas_upload_bytes = atlas_upload_bytes;
  stats->host_buffer_grows = host_buffer_grows;
  stats->readback_bytes = readback_bytes;
  stats->full_capacity_bytes = full_capacity_bytes;
}
static void *reserve_host(void *memory, size_t *capacity, size_t count,
                          size_t stride) {
  if (count <= *capacity)
    return memory;
  size_t next = *capacity ? *capacity : 256;
  while (next < count)
    next *= 2;
  void *grown = realloc(memory, next * stride);
  if (!grown)
    return NULL;
  *capacity = next;
  host_buffer_grows++;
  return grown;
}
int nv2a_gpu_host_buffers(size_t count, size_t texels, Nv2aGpuPixel **input,
                          Nv2aGpuPixel **output, uint32_t **texture) {
  if (!input || !output || !texture || !count || count > 4096u * 4096u ||
      texels > 4096u * 4096u || !nv2a_gpu_enabled())
    return 0;
  void *grown = reserve_host(host_input, &host_input_capacity, count,
                             sizeof(*host_input));
  if (!grown)
    goto fail_host;
  host_input = grown;
  grown = reserve_host(host_output, &host_output_capacity, count,
                       sizeof(*host_output));
  if (!grown)
    goto fail_host;
  host_output = grown;
  if (texels) {
    grown = reserve_host(host_texture, &host_texture_capacity, texels,
                         sizeof(*host_texture));
    if (!grown)
      goto fail_host;
    host_texture = grown;
  }
  *input = host_input;
  *output = host_output;
  *texture = texels ? host_texture : NULL;
  return 1;
fail_host:
  nv2a_gpu_fallback("host-allocation");
  return 0;
}
void nv2a_gpu_report(void) {
  vertex_report();
  if (!initialized || !getenv("RECOMP_GPU"))
    return;
  fprintf(stderr,
          "[GPU-HW] dispatches=%llu bounding-pixels=%llu failures=%llu "
          "cpu-fallbacks=%llu\n",
          draws, pixels, failures, fallbacks);
  fprintf(stderr,
          "[GPU-HW] atlas-uploads=%llu skipped=%llu uploaded-bytes=%llu "
          "host-buffer-grows=%llu\n",
          atlas_uploads, atlas_upload_skips, atlas_upload_bytes,
          host_buffer_grows);
  fprintf(stderr, "[GPU-HW] readback-bytes=%llu full-capacity-bytes=%llu\n",
          readback_bytes, full_capacity_bytes);
  fprintf(stderr, "[GPU-HW] resident-surface uploads=%llu downloads=%llu\n",
          surface_uploads, surface_downloads);
  fprintf(stderr, "[GPU-HW] batches=%llu tiles=%llu tile-entries=%llu\n",
          batches, batch_tiles, batch_entries);
  gpu_profile_report();
  for (unsigned i = 0; i < reason_count; i++)
    fprintf(stderr, "[GPU-HW] fallback %s=%llu\n", reasons[i].reason,
            reasons[i].count);
}
void nv2a_gpu_fallback(const char *reason) {
  if (!nv2a_gpu_enabled())
    return;
  fallbacks++;
  for (unsigned i = 0; i < reason_count; i++)
    if (!strcmp(reason, reasons[i].reason)) {
      reasons[i].count++;
      return;
    }
  if (reason_count < 16) {
    reasons[reason_count].reason = reason;
    reasons[reason_count++].count = 1;
  }
}
int nv2a_gpu_enabled(void) {
  HRESULT hr;
  if (initialized)
    return enabled;
  initialized = 1;
  if (!getenv("RECOMP_GPU") || strcmp(getenv("RECOMP_GPU"), "d3d11"))
    return 0;
  atexit(nv2a_gpu_report);
  atexit(cleanup);
  D3D_FEATURE_LEVEL level;
  D3D_FEATURE_LEVEL requested[] = {D3D_FEATURE_LEVEL_11_0};
  hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, requested, 1,
                         D3D11_SDK_VERSION, &dev, &level, &ctx);
  if (FAILED(hr)) {
    fprintf(stderr,
            "[GPU-HW] hardware D3D11 unavailable HRESULT=%08lX; software "
            "fallback\n",
            (unsigned long)hr);
    return 0;
  }
  D3D11_FEATURE_DATA_D3D11_OPTIONS options = {0};
  hr = ID3D11Device_CheckFeatureSupport(dev, D3D11_FEATURE_D3D11_OPTIONS,
                                        &options, sizeof(options));
  if (FAILED(hr) || !options.ExtendedDoublesShaderInstructions) {
    fprintf(stderr, "[GPU-HW] correctly rounded division requires extended "
                    "doubles; software fallback\n");
    cleanup();
    return 0;
  }
  IDXGIDevice *dxdev = NULL;
  IDXGIAdapter *adapter = NULL;
  DXGI_ADAPTER_DESC desc;
  hr = ID3D11Device_QueryInterface(dev, &IID_IDXGIDevice, (void **)&dxdev);
  if (SUCCEEDED(hr))
    hr = IDXGIDevice_GetAdapter(dxdev, &adapter);
  if (SUCCEEDED(hr))
    hr = IDXGIAdapter_GetDesc(adapter, &desc);
  RELEASE(dxdev, IDXGIDevice);
  RELEASE(adapter, IDXGIAdapter);
  if (FAILED(hr) || !desc.VendorId || desc.VendorId == 0x1414) {
    fprintf(
        stderr,
        "[GPU-HW] physical adapter identity unavailable; refusing dispatch\n");
    cleanup();
    return 0;
  }
  char name[256];
  WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name, sizeof(name),
                      NULL, NULL);
  fprintf(stderr,
          "[GPU-HW] adapter=%s vendor=%04X device=%04X dedicated-MiB=%llu "
          "feature=11_0 WARP=disabled\n",
          name, desc.VendorId, desc.DeviceId,
          (unsigned long long)(desc.DedicatedVideoMemory >> 20));
  hr = E_FAIL;
#ifndef NV2A_GPU_SHADER_GENERATOR
  if (!getenv("RECOMP_GPU_SHADER_SOURCE") &&
      shader_source_hash() == s_gpu_dxbc_source_hash) {
    hr = ID3D11Device_CreateComputeShader(dev, s_gpu_dxbc, sizeof(s_gpu_dxbc),
                                          NULL, &shader);
    if (SUCCEEDED(hr))
      fprintf(stderr,
              "[GPU-HW] shader=embedded-DXBC bytes=%zu source-FNV1a=%016llX\n",
              sizeof(s_gpu_dxbc), s_gpu_dxbc_source_hash);
    else
      fprintf(stderr,
              "[GPU-HW] embedded DXBC rejected HRESULT=%08lX; trying source\n",
              (unsigned long)hr);
  } else if (!getenv("RECOMP_GPU_SHADER_SOURCE")) {
    fprintf(stderr, "[GPU-HW] embedded shader source hash differs; "
                    "regenerating at runtime\n");
  }
#endif
  if (FAILED(hr)) {
    ID3DBlob *code = NULL, *error = NULL;
    fprintf(stderr, "[GPU-HW] shader=runtime-source (slow fallback)\n");
    hr = D3DCompile(source, sizeof(source) - 1, "nv2a_gpu_compute", NULL, NULL,
                    "main", "cs_5_0",
                    D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS,
                    0, &code, &error);
    if (FAILED(hr))
      fprintf(stderr, "[GPU-HW] shader compile failed: %s\n",
              error ? (char *)ID3D10Blob_GetBufferPointer(error)
                    : "no diagnostic");
    RELEASE(error, ID3D10Blob);
    if (SUCCEEDED(hr))
      hr = ID3D11Device_CreateComputeShader(
          dev, ID3D10Blob_GetBufferPointer(code),
          ID3D10Blob_GetBufferSize(code), NULL, &shader);
    RELEASE(code, ID3D10Blob);
  }
  D3D11_BUFFER_DESC bd = {0};
  bd.ByteWidth = sizeof(Nv2aGpuParams);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  if (SUCCEEDED(hr))
    hr = ID3D11Device_CreateBuffer(dev, &bd, NULL, &constants);
  if (FAILED(hr)) {
    fprintf(stderr, "[GPU-HW] initialization failed HRESULT=%08lX\n",
            (unsigned long)hr);
    cleanup();
    return 0;
  }
  enabled = 1;
  return 1;
}
static int create_uav_buffer(size_t count, size_t stride, ID3D11Buffer **buf,
                             ID3D11UnorderedAccessView **view,
                             ID3D11Buffer **staging) {
  D3D11_BUFFER_DESC d = {0};
  d.ByteWidth = (UINT)(count * stride);
  d.Usage = D3D11_USAGE_DEFAULT;
  d.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
  d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  d.StructureByteStride = (UINT)stride;
  HRESULT hr = ID3D11Device_CreateBuffer(dev, &d, NULL, buf);
  d.Usage = D3D11_USAGE_STAGING;
  d.BindFlags = 0;
  d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  d.MiscFlags = 0;
  d.StructureByteStride = 0;
  if (SUCCEEDED(hr))
    hr = ID3D11Device_CreateBuffer(dev, &d, NULL, staging);
  D3D11_UNORDERED_ACCESS_VIEW_DESC u = {0};
  u.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
  u.Buffer.NumElements = (UINT)count;
  if (SUCCEEDED(hr))
    hr = ID3D11Device_CreateUnorderedAccessView(dev, (ID3D11Resource *)*buf,
                                                &u, view);
  return SUCCEEDED(hr);
}
static int resize_pixels(size_t count) {
  if (count <= pixel_capacity)
    return 1;
  flush_queue_reason(PF_PIXELS_GROW);
  RELEASE(outview, ID3D11UnorderedAccessView);
  RELEASE(outbuf, ID3D11Buffer);
  RELEASE(readbuf, ID3D11Buffer);
  pixel_capacity = 0;
  if (!create_uav_buffer(count, sizeof(Nv2aGpuPixel), &outbuf, &outview,
                         &readbuf))
    return 0;
  pixel_capacity = count;
  return 1;
}
static int resize_surface(size_t count) {
  if (count <= surf_capacity)
    return 1;
  flush_queue_reason(PF_SURFACE_GROW);
  RELEASE(surfview, ID3D11UnorderedAccessView);
  RELEASE(surfbuf, ID3D11Buffer);
  RELEASE(surfread, ID3D11Buffer);
  surf_capacity = 0;
  if (!create_uav_buffer(count, sizeof(Nv2aGpuPixel), &surfbuf, &surfview,
                         &surfread))
    return 0;
  surf_capacity = count;
  return 1;
}
/* Separate counters: per-call draws report through their pixels and must
 * not add to the resident surface's pending statistics. */
static int ensure_stats(void) {
  if (statsview && tilestatsview)
    return 1;
  return (statsview ||
          create_uav_buffer(4, 4, &statsbuf, &statsview, &statsread)) &&
         (tilestatsview || create_uav_buffer(4, 4, &tilestatsbuf,
                                             &tilestatsview, &tilestatsread));
}
static void zero_stats(void) {
  static const uint32_t zero[4];
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)statsbuf, 0,
                                        NULL, zero, 0, 0);
}
static int resize_texture(size_t count) {
  if (count <= texture_capacity)
    return 1;
  flush_queue_reason(PF_TEXTURE_GROW);
  RELEASE(texview, ID3D11ShaderResourceView);
  RELEASE(texbuf, ID3D11Buffer);
  texture_capacity = 0;
  atlas_snapshot_valid = 0;
  D3D11_BUFFER_DESC d = {0};
  d.ByteWidth = (UINT)(count * 4);
  d.Usage = D3D11_USAGE_DEFAULT;
  d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  d.StructureByteStride = 4;
  HRESULT hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &texbuf);
  D3D11_SHADER_RESOURCE_VIEW_DESC s = {0};
  s.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  s.Buffer.NumElements = (UINT)count;
  if (SUCCEEDED(hr))
    hr = ID3D11Device_CreateShaderResourceView(dev, (ID3D11Resource *)texbuf,
                                               &s, &texview);
  if (FAILED(hr))
    return 0;
  texture_capacity = count;
  return 1;
}
/* Shared argument checks; count is samples in the box, texcount atlas texels. */
static int validate(const Nv2aGpuParams *p, size_t *count, size_t *texcount) {
  unsigned sx = p->depth[3] ? p->depth[3] : 1,
           sy = p->alpha[3] ? p->alpha[3] : 1;
  if ((sx != 1 && sx != 2) || (sy != 1 && sy != 2) || !p->box[2] ||
      !p->box[3] || p->box[2] > 4096 || p->box[3] > 4096 || p->box[0] > 65535 ||
      p->box[1] > 65535 || (uint64_t)p->box[0] + p->box[2] > 65536 ||
      (uint64_t)p->box[1] + p->box[3] > 65536 || p->tex[0] > 4096 ||
      p->tex[1] > 4096) {
    nv2a_gpu_fallback("invalid-dimensions");
    return 0;
  }
  *count = (size_t)p->box[2] * p->box[3] * sx * sy;
  *texcount = p->rcmeta[0] ? p->rcmeta[1] : (size_t)p->tex[0] * p->tex[1];
  if (p->rcmeta[0])
    for (unsigned st = 0; st < 4; st++) {
      uint64_t size = (uint64_t)p->stage_info[st][0] * p->stage_info[st][1];
      if (p->stage_info[st][0] > 4096 || p->stage_info[st][1] > 4096 ||
          p->stage_info[st][2] + size > *texcount) {
        nv2a_gpu_fallback("invalid-atlas");
        return 0;
      }
    }
  return *count && *count <= 4096 * 4096 && *texcount <= 4096 * 4096;
}
static void upload_atlas(const Nv2aGpuParams *p, const uint32_t *texture,
                         size_t texcount) {
  AtlasKey key = {0};
  key.count = texcount;
  (void)p;
  /* The resource contains decoded ARGB texels only. Dimensions, offsets,
   * address/filter modes and combiner interpretation live in each triangle's
   * Params, so changing those does not invalidate identical resource bytes.
   * Keeping them in the resource key needlessly flushed otherwise valid batches. */
  if (atlas_snapshot_valid && key.count == last_atlas_key.count &&
      !memcmp(texture, atlas_snapshot, texcount * 4)) {
    atlas_upload_skips++;
    return;
  }
  flush_queue_reason(PF_ATLAS); /* queued triangles still read the current atlas */
  D3D11_BOX box = {0};
  box.right = (UINT)(texcount * 4);
  box.bottom = box.back = 1;
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)texbuf, 0, &box,
                                        texture, 0, 0);
  atlas_uploads++;
  atlas_upload_bytes += texcount * 4;
  void *grown =
      reserve_host(atlas_snapshot, &atlas_snapshot_capacity, texcount, 4);
  if (grown) {
    atlas_snapshot = grown;
    memcpy(atlas_snapshot, texture, texcount * 4);
    last_atlas_key = key;
    atlas_snapshot_valid = 1;
  } else
    atlas_snapshot_valid = 0; /* Allocation failure merely disables caching. */
}
/* Triangle batches.
 *
 * A match frame is tens of thousands of triangles; one dispatch each cost
 * about 11 us of GPU/driver time apiece, so the frame was bounded by
 * dispatch count rather than pixels. Triangles now queue on the host and are
 * binned into 8x8 tiles of the target; one dispatch runs a group per touched
 * tile, and every thread visits that tile's triangles in submission order.
 * Per-pixel order -- what blending and depth depend on -- is therefore the
 * order the triangles were drawn, exactly as with one dispatch each.
 *
 * The queue is flushed before anything that could observe or change what the
 * queued triangles read or write: a different target, the atlas changing
 * (or its buffer growing), the resident surface being replaced or read back,
 * a per-call draw, or the queue filling. */
#define QUEUE_MAX 4096
static Nv2aGpuParams *q_tris;
static size_t q_count;
static ID3D11UnorderedAccessView *q_target, *q_counters;
static uint32_t q_stride, q_rows;
static ID3D11Buffer *tribuf, *tilebuf, *orderbuf, *batchcb;
static ID3D11ShaderResourceView *triview, *tileview, *orderview;
static size_t tri_cap, tile_cap, order_cap;
static uint32_t *bin_count, *bin_cursor, *h_tiles, *h_order;
static size_t bin_cap, cursor_cap, h_tiles_cap, h_order_cap;

static void queue_cleanup(void) {
  RELEASE(triview, ID3D11ShaderResourceView);
  RELEASE(tileview, ID3D11ShaderResourceView);
  RELEASE(orderview, ID3D11ShaderResourceView);
  RELEASE(tribuf, ID3D11Buffer);
  RELEASE(tilebuf, ID3D11Buffer);
  RELEASE(orderbuf, ID3D11Buffer);
  RELEASE(batchcb, ID3D11Buffer);
  free(q_tris);
  free(bin_count);
  free(bin_cursor);
  free(h_tiles);
  free(h_order);
  q_tris = NULL;
  bin_count = bin_cursor = h_tiles = h_order = NULL;
  q_count = tri_cap = tile_cap = order_cap = 0;
  bin_cap = cursor_cap = h_tiles_cap = h_order_cap = 0;
  q_target = q_counters = NULL;
}
static int dyn_buffer(ID3D11Buffer **buf, ID3D11ShaderResourceView **view,
                      size_t *cap, size_t count, size_t stride) {
  if (count <= *cap)
    return 1;
  size_t next = *cap ? *cap : 256;
  while (next < count)
    next *= 2;
  RELEASE(*view, ID3D11ShaderResourceView);
  RELEASE(*buf, ID3D11Buffer);
  *cap = 0;
  D3D11_BUFFER_DESC d = {0};
  d.ByteWidth = (UINT)(next * stride);
  d.Usage = D3D11_USAGE_DYNAMIC;
  d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  d.StructureByteStride = (UINT)stride;
  HRESULT hr = ID3D11Device_CreateBuffer(dev, &d, NULL, buf);
  D3D11_SHADER_RESOURCE_VIEW_DESC s = {0};
  s.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  s.Buffer.NumElements = (UINT)next;
  if (SUCCEEDED(hr))
    hr = ID3D11Device_CreateShaderResourceView(dev, (ID3D11Resource *)*buf, &s,
                                               view);
  if (FAILED(hr))
    return 0;
  *cap = next;
  return 1;
}
static int dyn_upload(ID3D11Buffer *buf, const void *data, size_t bytes) {
  D3D11_MAPPED_SUBRESOURCE m;
  if (FAILED(ID3D11DeviceContext_Map(ctx, (ID3D11Resource *)buf, 0,
                                     D3D11_MAP_WRITE_DISCARD, 0, &m)))
    return 0;
  memcpy(m.pData, data, bytes);
  ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)buf, 0);
  return 1;
}
static void *grow_u32(uint32_t *p, size_t *cap, size_t count) {
  if (count <= *cap)
    return p;
  size_t next = *cap ? *cap : 1024;
  while (next < count)
    next *= 2;
  void *g = realloc(p, next * sizeof(uint32_t));
  if (g)
    *cap = next;
  return g;
}
static void unbind(void) {
  ID3D11UnorderedAccessView *nullu[] = {NULL, NULL};
  ID3D11ShaderResourceView *nulls[] = {NULL, NULL, NULL, NULL, NULL};
  ID3D11DeviceContext_CSSetUnorderedAccessViews(ctx, 0, 2, nullu, NULL);
  ID3D11DeviceContext_CSSetShaderResources(ctx, 0, 5, nulls);
}
/* Sample-space rectangle a queued triangle may write, clipped to target. */
static int tri_rect(const Nv2aGpuParams *p, uint32_t r[4]) {
  unsigned sx = p->depth[3] ? p->depth[3] : 1,
           sy = p->alpha[3] ? p->alpha[3] : 1;
  uint64_t x1 = (uint64_t)p->surf[1] + (uint64_t)p->box[2] * sx,
           y1 = (uint64_t)p->surf[2] + (uint64_t)p->box[3] * sy;
  r[0] = p->surf[1];
  r[1] = p->surf[2];
  r[2] = (uint32_t)(x1 < q_stride ? x1 : q_stride);
  r[3] = (uint32_t)(y1 < q_rows ? y1 : q_rows);
  return r[0] < r[2] && r[1] < r[3];
}
static int flush_queue(void) {
  if (!q_count)
    return 1;
  size_t n = q_count;
  uint64_t profile_start = gpu_profile_ticks(), profile_mark;
  if (gpu_profile_enabled()) {
    profile_flush[profile_reason]++;
    profile_hist[n==1?0:n<=8?1:n<=32?2:n<=128?3:n<=512?4:5]++;
  }
  q_count = 0;
  uint32_t tx = (q_stride + 7) / 8, ty = (q_rows + 7) / 8;
  size_t tiles = (size_t)tx * ty, total = 0, used = 0;
  uint32_t *c = grow_u32(bin_count, &bin_cap, tiles);
  if (!c)
    goto fail;
  bin_count = c;
  c = grow_u32(bin_cursor, &cursor_cap, tiles);
  if (!c)
    goto fail;
  bin_cursor = c;
  memset(bin_count, 0, tiles * sizeof(uint32_t));
  for (size_t i = 0; i < n; i++) {
    uint32_t r[4];
    if (!tri_rect(&q_tris[i], r))
      continue;
    for (uint32_t y = r[1] / 8; y <= (r[3] - 1) / 8; y++)
      for (uint32_t x = r[0] / 8; x <= (r[2] - 1) / 8; x++)
        bin_count[(size_t)y * tx + x]++;
  }
  for (size_t t = 0; t < tiles; t++)
    if (bin_count[t]) {
      bin_cursor[t] = (uint32_t)total;
      total += bin_count[t];
      used++;
    }
  if (!used)
    return 1;
  uint32_t *ht = grow_u32(h_tiles, &h_tiles_cap, used * 4);
  if (!ht)
    goto fail;
  h_tiles = ht;
  uint32_t *ho = grow_u32(h_order, &h_order_cap, total);
  if (!ho)
    goto fail;
  h_order = ho;
  for (size_t t = 0, k = 0; t < tiles; t++)
    if (bin_count[t]) {
      uint32_t x = (uint32_t)(t % tx) * 8, y = (uint32_t)(t / tx) * 8;
      h_tiles[k * 4 + 0] = x | (y << 16);
      h_tiles[k * 4 + 1] = bin_cursor[t];
      h_tiles[k * 4 + 2] = bin_count[t];
      h_tiles[k * 4 + 3] = 0;
      k++;
    }
  for (size_t i = 0; i < n; i++) {
    uint32_t r[4];
    if (!tri_rect(&q_tris[i], r))
      continue;
    for (uint32_t y = r[1] / 8; y <= (r[3] - 1) / 8; y++)
      for (uint32_t x = r[0] / 8; x <= (r[2] - 1) / 8; x++)
        h_order[bin_cursor[(size_t)y * tx + x]++] = (uint32_t)i;
  }
  profile_mark = gpu_profile_ticks();
  profile_bin += profile_mark - profile_start;
  if (!dyn_buffer(&tribuf, &triview, &tri_cap, n, sizeof(Nv2aGpuParams)) ||
      !dyn_buffer(&tilebuf, &tileview, &tile_cap, used, 16) ||
      !dyn_buffer(&orderbuf, &orderview, &order_cap, total, 4) ||
      !dyn_upload(tribuf, q_tris, n * sizeof(Nv2aGpuParams)) ||
      !dyn_upload(tilebuf, h_tiles, used * 16) ||
      !dyn_upload(orderbuf, h_order, total * 4))
    goto fail;
  if (!batchcb) {
    D3D11_BUFFER_DESC d = {0};
    d.ByteWidth = 16;
    d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(ID3D11Device_CreateBuffer(dev, &d, NULL, &batchcb)))
      goto fail;
  }
  profile_start = gpu_profile_ticks();
  profile_upload += profile_start - profile_mark;
  GpuProfileQuery *profile_query = gpu_profile_begin();
  ID3D11ShaderResourceView *views[] = {NULL, texview, triview, tileview,
                                       orderview};
  ID3D11UnorderedAccessView *uavs[] = {q_target, q_counters};
  ID3D11DeviceContext_CSSetShader(ctx, shader, NULL, 0);
  ID3D11DeviceContext_CSSetShaderResources(ctx, 0, 5, views);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(ctx, 0, 2, uavs, NULL);
  for (size_t base = 0; base < used; base += 65535) {
    uint32_t cb[4] = {(uint32_t)base, 0, 0, 0};
    size_t groups = used - base < 65535 ? used - base : 65535;
    ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)batchcb, 0,
                                          NULL, cb, 0, 0);
    ID3D11DeviceContext_CSSetConstantBuffers(ctx, 0, 1, &batchcb);
    ID3D11DeviceContext_Dispatch(ctx, (UINT)groups, 1, 1);
  }
  gpu_profile_end(profile_query);
  unbind();
  profile_submit += gpu_profile_ticks() - profile_start;
  batches++;
  batch_tiles += used;
  batch_entries += total;
  return 1;
fail:
  fprintf(stderr, "[GPU-HW] batch of %zu triangles failed\n", n);
  failures++;
  return 0;
}
static int enqueue(const Nv2aGpuParams *p, ID3D11UnorderedAccessView *target,
                   ID3D11UnorderedAccessView *counters, uint32_t stride,
                   uint32_t rows, uint32_t ox, uint32_t oy, int zpass) {
  if (q_count && (target != q_target || counters != q_counters ||
                  stride != q_stride || rows != q_rows))
    if (!flush_queue_reason(PF_TARGET))
      return 0;
  if (q_count == QUEUE_MAX && !flush_queue_reason(PF_FULL))
    return 0;
  if (!q_tris) {
    q_tris = malloc(QUEUE_MAX * sizeof(*q_tris));
    if (!q_tris)
      return 0;
  }
  Nv2aGpuParams *q = &q_tris[q_count++];
  *q = *p;
  q->surf[0] = stride;
  q->surf[1] = ox;
  q->surf[2] = oy;
  q->surf[3] = zpass ? 1u : 0u;
  q_target = target;
  q_counters = counters;
  q_stride = stride;
  q_rows = rows;
  return 1;
}
static int read_buffer(ID3D11Buffer *src, ID3D11Buffer *staging, void *out,
                       size_t bytes) {
  uint64_t profile_start = gpu_profile_ticks();
  D3D11_BOX readbox = {0};
  readbox.right = (UINT)bytes;
  readbox.bottom = readbox.back = 1;
  ID3D11DeviceContext_CopySubresourceRegion(ctx, (ID3D11Resource *)staging, 0,
                                            0, 0, 0, (ID3D11Resource *)src, 0,
                                            &readbox);
  D3D11_MAPPED_SUBRESOURCE mapped;
  HRESULT hr = ID3D11DeviceContext_Map(ctx, (ID3D11Resource *)staging, 0,
                                       D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr))
    return 0;
  memcpy(out, mapped.pData, bytes);
  ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)staging, 0);
  readback_bytes += bytes;
  profile_readback += gpu_profile_ticks() - profile_start;
  return 1;
}
static void device_failure(void) {
  failures++;
  nv2a_gpu_fallback("dispatch-failure");
  if (FAILED(ID3D11Device_GetDeviceRemovedReason(dev))) {
    fprintf(stderr, "[GPU-HW] device removed; disabling hardware backend\n");
    enabled = 0;
    cleanup();
  }
}
static void count_draw(size_t count) {
  draws++;
  pixels += count;
  if (draws == 1 || draws % 10000 == 0)
    nv2a_gpu_report();
}
int nv2a_gpu_draw(const Nv2aGpuParams *p, const Nv2aGpuPixel *input,
                  const uint32_t *texture, Nv2aGpuPixel *output) {
  size_t count, texcount;
  if (!p || !nv2a_gpu_enabled() || !validate(p, &count, &texcount))
    return 0;
  unsigned sx = p->depth[3] ? p->depth[3] : 1,
           sy = p->alpha[3] ? p->alpha[3] : 1;
  if (!input || !output || (texcount && !texture))
    return 0;
  if (!flush_queue_reason(PF_PER_CALL))
    goto fail;
  if (!resize_pixels(count) || !resize_texture(texcount ? texcount : 1) ||
      !ensure_stats())
    goto fail;
  /* The tile is its own surface: stride is the box width, origin zero. */
  D3D11_BOX box = {0};
  box.right = (UINT)(count * sizeof(*input));
  box.bottom = box.back = 1;
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)outbuf, 0, &box,
                                        input, 0, 0);
  if (texcount)
    upload_atlas(p, texture, texcount);
  if (!enqueue(p, outview, tilestatsview, p->box[2] * sx, p->box[3] * sy, 0,
               0, 0) ||
      !flush_queue_reason(PF_PER_CALL_END))
    goto fail;
  if (!read_buffer(outbuf, readbuf, output, count * sizeof(*output)))
    goto fail;
  full_capacity_bytes += pixel_capacity * sizeof(*output);
  count_draw(count);
  return 1;
fail:
  device_failure();
  return 0;
}
static size_t surf_count;
int nv2a_gpu_surface_upload(const Nv2aGpuPixel *pixels, size_t count) {
  if (!pixels || !count || count > 8192u * 8192u || !nv2a_gpu_enabled())
    return 0;
  if (!flush_queue_reason(PF_SURFACE_UPLOAD))
    return 0;
  if (!resize_surface(count) || !ensure_stats()) {
    device_failure();
    return 0;
  }
  D3D11_BOX box = {0};
  box.right = (UINT)(count * sizeof(*pixels));
  box.bottom = box.back = 1;
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)surfbuf, 0, &box,
                                        pixels, 0, 0);
  zero_stats();
  surf_count = count;
  surface_uploads++;
  return 1;
}
int nv2a_gpu_surface_draw(const Nv2aGpuParams *p, const uint32_t *texture,
                          uint32_t stride, int zpass) {
  size_t count, texcount;
  if (!p || !nv2a_gpu_enabled() || !surf_count || !validate(p, &count, &texcount))
    return 0;
  unsigned sx = p->depth[3] ? p->depth[3] : 1,
           sy = p->alpha[3] ? p->alpha[3] : 1;
  uint64_t ox = (uint64_t)p->box[0] * sx, oy = (uint64_t)p->box[1] * sy,
           w = (uint64_t)p->box[2] * sx, h = (uint64_t)p->box[3] * sy;
  if (!stride || ox + w > stride || (oy + h) * stride > surf_count) {
    nv2a_gpu_fallback("surface-bounds");
    return 0;
  }
  if (texcount) {
    if (texture) {
      if (!resize_texture(texcount)) {
        device_failure();
        return 0;
      }
      upload_atlas(p, texture, texcount);
    } else if (texcount > texture_capacity || !atlas_snapshot_valid) {
      nv2a_gpu_fallback("atlas-reuse");
      return 0;
    } else
      atlas_upload_skips++;
  } else if (!resize_texture(1)) {
    device_failure();
    return 0;
  }
  if (!enqueue(p, surfview, statsview, stride, (uint32_t)(surf_count / stride),
               (uint32_t)ox, (uint32_t)oy, zpass)) {
    device_failure();
    return 0;
  }
  count_draw(count);
  return 1;
}
int nv2a_gpu_surface_download(Nv2aGpuPixel *pixels, size_t count,
                              uint64_t stats[3]) {
  uint32_t s[4];
  if (!pixels || !nv2a_gpu_enabled() || !count || count > surf_count)
    return 0;
  if (!flush_queue_reason(PF_READBACK))
    return 0;
  if (!read_buffer(surfbuf, surfread, pixels, count * sizeof(*pixels)) ||
      !read_buffer(statsbuf, statsread, s, sizeof(s))) {
    device_failure();
    return 0;
  }
  zero_stats();
  if (stats) {
    stats[0] = s[0];
    stats[1] = s[1];
    stats[2] = s[2];
  }
  surface_downloads++;
  return 1;
}
#include "nv2a_gpu_vertex.inc"
#else
int nv2a_gpu_transform(Nv2aVshState *s, const Nv2aVshInput *i,
                       Nv2aVshOutput *o, size_t n) {
  (void)s; (void)i; (void)o; (void)n; return 0;
}
int nv2a_gpu_enabled(void) { return 0; }
int nv2a_gpu_host_buffers(size_t n, size_t t, Nv2aGpuPixel **i,
                          Nv2aGpuPixel **o, uint32_t **x) {
  (void)n;
  (void)t;
  (void)i;
  (void)o;
  (void)x;
  return 0;
}
void nv2a_gpu_get_stats(Nv2aGpuStats *s) {
  if (s)
    memset(s, 0, sizeof(*s));
}
int nv2a_gpu_draw(const Nv2aGpuParams *p, const Nv2aGpuPixel *i,
                  const uint32_t *t, Nv2aGpuPixel *o) {
  (void)p;
  (void)i;
  (void)t;
  (void)o;
  return 0;
}
int nv2a_gpu_surface_upload(const Nv2aGpuPixel *p, size_t n) {
  (void)p; (void)n; return 0;
}
int nv2a_gpu_surface_draw(const Nv2aGpuParams *p, const uint32_t *t,
                          uint32_t s, int z) {
  (void)p; (void)t; (void)s; (void)z; return 0;
}
int nv2a_gpu_surface_download(Nv2aGpuPixel *p, size_t n, uint64_t s[3]) {
  (void)p; (void)n; (void)s; return 0;
}
void nv2a_gpu_fallback(const char *reason) { (void)reason; }
void nv2a_gpu_report(void) {}
#endif
