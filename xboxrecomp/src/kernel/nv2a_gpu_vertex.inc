/* Included by nv2a_gpu_compute.c: same physical adapter and executor thread,
 * but a D3D11 device of its own. Every batch reads its vertices back before
 * rasterising them; on the raster device that readback waited behind every
 * triangle already queued, so CPU and GPU took turns. A separate device is a
 * separate queue: the readback waits only for this batch's transform. Each
 * cached shader is straight-line generated HLSL, compiled once; neither
 * shader nor per-vertex host path dispatches NV2A opcodes. */
#include "nv2a_vsh_translate.h"
static ID3D11Device *vdev;
static ID3D11DeviceContext *vctx;
static int vertex_device(void) {
  if (vctx)
    return 1;
  if (!dev)
    return 0;
  IDXGIDevice *dxdev = NULL;
  IDXGIAdapter *adapter = NULL;
  D3D_FEATURE_LEVEL level, requested[] = {D3D_FEATURE_LEVEL_11_0};
  HRESULT hr = ID3D11Device_QueryInterface(dev, &IID_IDXGIDevice, (void **)&dxdev);
  if (SUCCEEDED(hr))
    hr = IDXGIDevice_GetAdapter(dxdev, &adapter);
  if (SUCCEEDED(hr))
    hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, requested,
                           1, D3D11_SDK_VERSION, &vdev, &level, &vctx);
  RELEASE(dxdev, IDXGIDevice);
  RELEASE(adapter, IDXGIAdapter);
  if (FAILED(hr)) {
    fprintf(stderr, "[GPU-VSH] vertex device unavailable HRESULT=%08lX\n",
            (unsigned long)hr);
    return 0;
  }
  return 1;
}
#define dev vdev
#define ctx vctx
#define VSH_CACHE_SIZE 16
typedef struct {
  uint32_t program[NV2A_VSH_SLOTS][4], start, cxt;
  ID3D11ComputeShader *shader;
  int occupied;
} VertexProgram;
static VertexProgram v_programs[VSH_CACHE_SIZE];
static unsigned v_replacement;
static ID3D11Buffer *v_input, *v_output, *v_readback, *v_constants;
static ID3D11ShaderResourceView *v_input_view;
static ID3D11UnorderedAccessView *v_output_view;
static ID3D11Buffer *v_context, *v_context_readback;
static ID3D11UnorderedAccessView *v_context_view;
static size_t v_capacity;
static unsigned long long v_compiled, v_dispatches, v_vertices,
    v_rejections, v_failures, v_fallbacks, v_cache_hits;
#include "nv2a_gpu_vertex_profile.inc"
static void vertex_cleanup(void) {
  for (unsigned j = 0; j < VSH_CACHE_SIZE; j++) {
    RELEASE(v_programs[j].shader, ID3D11ComputeShader);
    v_programs[j].occupied = 0;
  }
  RELEASE(v_input_view, ID3D11ShaderResourceView);
  RELEASE(v_output_view, ID3D11UnorderedAccessView);
  RELEASE(v_input, ID3D11Buffer);
  RELEASE(v_output, ID3D11Buffer);
  RELEASE(v_readback, ID3D11Buffer);
  RELEASE(v_constants, ID3D11Buffer);
  RELEASE(v_context_view, ID3D11UnorderedAccessView);
  RELEASE(v_context, ID3D11Buffer);
  RELEASE(v_context_readback, ID3D11Buffer);
  v_capacity = 0;
  if (vctx)
    ID3D11DeviceContext_ClearState(vctx);
  RELEASE(vctx, ID3D11DeviceContext);
  RELEASE(vdev, ID3D11Device);
}
static void vertex_report(void) {
  if (v_compiled || v_rejections || v_failures)
    fprintf(stderr, "[GPU-VSH] compiled=%llu dispatches=%llu vertices=%llu "
            "cache-hits=%llu rejected-programs=%llu failures=%llu "
            "interpreter-fallback-batches=%llu\n", v_compiled, v_dispatches,
            v_vertices, v_cache_hits, v_rejections, v_failures, v_fallbacks);
  vertex_profile_report();
}
static ID3D11ComputeShader *vertex_program(const Nv2aVshState *state) {
  for (unsigned j = 0; j < VSH_CACHE_SIZE; j++) {
    VertexProgram *p = &v_programs[j];
    if (p->occupied && p->start == state->start && p->cxt == state->cxt_write &&
        !memcmp(p->program, state->program, sizeof p->program)) {
      v_cache_hits++; return p->shader;
    }
  }
  VertexProgram *p = &v_programs[v_replacement++ % VSH_CACHE_SIZE];
  RELEASE(p->shader, ID3D11ComputeShader);
  memcpy(p->program, state->program, sizeof p->program);
  p->start = state->start; p->cxt = state->cxt_write; p->occupied = 1;
  char *text = malloc(256 * 1024), diagnostic[256] = {0};
  if (!text) { v_failures++; return NULL; }
  size_t length = nv2a_vsh_translate(state->program, state->start,
      state->cxt_write != 0, text, 256 * 1024, diagnostic, sizeof diagnostic);
  if (!length) {
    fprintf(stderr, "[GPU-VSH] rejected start=%u: %s\n", state->start, diagnostic);
    v_rejections++; free(text); return NULL;
  }
  ID3DBlob *code = NULL, *error = NULL;
  HRESULT hr = D3DCompile(text, length, "nv2a_native_vertex", NULL, NULL,
      "main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS,
      0, &code, &error);
  free(text);
  if (FAILED(hr)) {
    fprintf(stderr, "[GPU-VSH] compile failed start=%u: %s\n", state->start,
        error ? (char *)ID3D10Blob_GetBufferPointer(error) : "no diagnostic");
  } else {
    hr = ID3D11Device_CreateComputeShader(dev, ID3D10Blob_GetBufferPointer(code),
        ID3D10Blob_GetBufferSize(code), NULL, &p->shader);
  }
  RELEASE(error, ID3D10Blob); RELEASE(code, ID3D10Blob);
  if (FAILED(hr)) v_failures++;
  else {
    v_compiled++;
    fprintf(stderr, "[GPU-VSH] compiled straight-line program start=%u source-bytes=%zu\n",
        state->start, length);
  }
  return p->shader;
}
static int vertex_buffers(size_t count) {
  HRESULT hr = S_OK;
  if (!v_constants) {
    D3D11_BUFFER_DESC d = {0};
    d.ByteWidth = NV2A_VSH_CONSTANTS * 16 + 16;
    d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_constants);
    if (FAILED(hr)) return 0;
  }
  if (!v_context) {
    D3D11_BUFFER_DESC d = {0};
    d.ByteWidth = NV2A_VSH_CONSTANTS * 16;
    d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; d.StructureByteStride = 16;
    hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_context);
    D3D11_UNORDERED_ACCESS_VIEW_DESC u = {0};
    u.Format = DXGI_FORMAT_UNKNOWN; u.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    u.Buffer.NumElements = NV2A_VSH_CONSTANTS;
    if (SUCCEEDED(hr)) hr = ID3D11Device_CreateUnorderedAccessView(dev,
        (ID3D11Resource *)v_context, &u, &v_context_view);
    d.Usage = D3D11_USAGE_STAGING; d.BindFlags = 0;
    d.CPUAccessFlags = D3D11_CPU_ACCESS_READ; d.MiscFlags = 0; d.StructureByteStride = 0;
    if (SUCCEEDED(hr)) hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_context_readback);
    if (FAILED(hr)) {
      RELEASE(v_context_view, ID3D11UnorderedAccessView);
      RELEASE(v_context, ID3D11Buffer); RELEASE(v_context_readback, ID3D11Buffer);
      return 0;
    }
  }
  if (count <= v_capacity) return 1;
  RELEASE(v_input_view, ID3D11ShaderResourceView);
  RELEASE(v_output_view, ID3D11UnorderedAccessView);
  RELEASE(v_input, ID3D11Buffer); RELEASE(v_output, ID3D11Buffer);
  RELEASE(v_readback, ID3D11Buffer); v_capacity = 0;
  size_t capacity = 64;
  while (capacity < count) capacity *= 2;
  D3D11_BUFFER_DESC d = {0};
  d.ByteWidth = (UINT)(capacity * sizeof(Nv2aVshInput));
  d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  d.StructureByteStride = sizeof(Nv2aVshInput);
  hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_input);
  d.ByteWidth = (UINT)(capacity * sizeof(Nv2aVshOutput));
  d.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
  d.StructureByteStride = sizeof(Nv2aVshOutput);
  if (SUCCEEDED(hr)) hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_output);
  d.Usage = D3D11_USAGE_STAGING; d.BindFlags = 0;
  d.CPUAccessFlags = D3D11_CPU_ACCESS_READ; d.MiscFlags = 0; d.StructureByteStride = 0;
  if (SUCCEEDED(hr)) hr = ID3D11Device_CreateBuffer(dev, &d, NULL, &v_readback);
  D3D11_SHADER_RESOURCE_VIEW_DESC s = {0};
  s.Format = DXGI_FORMAT_UNKNOWN; s.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  s.Buffer.NumElements = (UINT)capacity;
  if (SUCCEEDED(hr)) hr = ID3D11Device_CreateShaderResourceView(dev,
      (ID3D11Resource *)v_input, &s, &v_input_view);
  D3D11_UNORDERED_ACCESS_VIEW_DESC u = {0};
  u.Format = DXGI_FORMAT_UNKNOWN; u.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
  u.Buffer.NumElements = (UINT)capacity;
  if (SUCCEEDED(hr)) hr = ID3D11Device_CreateUnorderedAccessView(dev,
      (ID3D11Resource *)v_output, &u, &v_output_view);
  if (FAILED(hr)) return 0;
  v_capacity = capacity; return 1;
}
int nv2a_gpu_transform(Nv2aVshState *state, const Nv2aVshInput *input,
                       Nv2aVshOutput *output, size_t count) {
  if (!state || !input || !output || !count || count > 4096 ||
      !nv2a_gpu_enabled() || !vertex_device()) return 0;
  uint64_t profile_whole=gpu_profile_ticks(), profile_mark=profile_whole;
  int serial = nv2a_vsh_context_writes(state->program,state->start,state->cxt_write != 0);
  ID3D11ComputeShader *program = vertex_program(state);
  uint64_t profile_now=gpu_profile_ticks();
  vp_program+=profile_now-profile_mark; profile_mark=profile_now;
  if (!program) { v_fallbacks++; return 0; }
  if (!vertex_buffers(count)) goto fail;
  profile_now=gpu_profile_ticks();
  vp_buffers+=profile_now-profile_mark; profile_mark=profile_now;
  struct { float c[NV2A_VSH_CONSTANTS][4]; uint32_t meta[4]; } params;
  memcpy(params.c, state->constants, sizeof params.c);
  params.meta[0] = (uint32_t)count;
  params.meta[1] = params.meta[2] = params.meta[3] = 0;
  D3D11_BOX box = {0};
  box.right = (UINT)(count * sizeof(*input)); box.bottom = box.back = 1;
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)v_input,
      0, &box, input, 0, 0);
  ID3D11DeviceContext_UpdateSubresource(ctx, (ID3D11Resource *)v_constants,
      0, NULL, &params, 0, 0);
  profile_now=gpu_profile_ticks();
  vp_upload+=profile_now-profile_mark; profile_mark=profile_now;
  ID3D11DeviceContext_CSSetShader(ctx, program, NULL, 0);
  ID3D11DeviceContext_CSSetConstantBuffers(ctx, 0, 1, &v_constants);
  ID3D11DeviceContext_CSSetShaderResources(ctx, 0, 1, &v_input_view);
  ID3D11UnorderedAccessView *views[2] = {v_output_view,serial?v_context_view:NULL};
  ID3D11DeviceContext_CSSetUnorderedAccessViews(ctx, 0, 2, views, NULL);
  ID3D11DeviceContext_Dispatch(ctx, serial?1:(UINT)((count + 63) / 64), 1, 1);
  ID3D11ShaderResourceView *nullsrv = NULL;
  ID3D11UnorderedAccessView *nulluav[2] = {NULL,NULL};
  ID3D11DeviceContext_CSSetShaderResources(ctx, 0, 1, &nullsrv);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(ctx, 0, 2, nulluav, NULL);
  profile_now=gpu_profile_ticks();
  vp_submit+=profile_now-profile_mark; profile_mark=profile_now;
  box.right = (UINT)(count * sizeof(*output));
  ID3D11DeviceContext_CopySubresourceRegion(ctx, (ID3D11Resource *)v_readback,
      0, 0, 0, 0, (ID3D11Resource *)v_output, 0, &box);
  D3D11_MAPPED_SUBRESOURCE mapped;
  uint64_t profile_map=gpu_profile_ticks();
  HRESULT hr = ID3D11DeviceContext_Map(ctx, (ID3D11Resource *)v_readback,
      0, D3D11_MAP_READ, 0, &mapped);
  vp_map+=gpu_profile_ticks()-profile_map;
  if (FAILED(hr)) goto fail;
  float final_constants[NV2A_VSH_CONSTANTS][4];
  if (serial) {
    ID3D11DeviceContext_CopyResource(ctx, (ID3D11Resource *)v_context_readback,
        (ID3D11Resource *)v_context);
    D3D11_MAPPED_SUBRESOURCE context_map;
    profile_map=gpu_profile_ticks();
    hr = ID3D11DeviceContext_Map(ctx, (ID3D11Resource *)v_context_readback,
        0, D3D11_MAP_READ, 0, &context_map);
    vp_map+=gpu_profile_ticks()-profile_map;
    if (FAILED(hr)) {
      ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)v_readback, 0);
      goto fail;
    }
    memcpy(final_constants,context_map.pData,sizeof final_constants);
    ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)v_context_readback, 0);
  }
  /* Publish neither outputs nor mutated state before every readback succeeds.
   * A failed batch can then safely use the interpreter's original state. */
  memcpy(output, mapped.pData, count * sizeof(*output));
  if (serial) memcpy(state->constants,final_constants,sizeof final_constants);
  ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)v_readback, 0);
  uint64_t profile_read=gpu_profile_ticks()-profile_mark;
  vp_readback+=profile_read;
  vertex_profile_finish(profile_whole,profile_read,count,serial);
  v_dispatches++; v_vertices += count;
  if (v_dispatches == 1) vertex_report();
  return 1;
fail:
  v_failures++; v_fallbacks++;
  fprintf(stderr, "[GPU-VSH] buffer/dispatch failed; retaining interpreter\n");
  return 0;
}
#undef dev
#undef ctx
