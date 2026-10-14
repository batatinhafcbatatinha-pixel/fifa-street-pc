/* Optional profiling: timestamp queries are polled with DONOTFLUSH and never
 * waited for. A full ring skips samples instead of stalling the renderer. */
static int gpu_profile_enabled(void) {
  static int value = -1;
  if (value < 0) value = getenv("RECOMP_GPU_PROFILE") != NULL;
  return value;
}
static uint64_t gpu_profile_ticks(void) {
  LARGE_INTEGER t;
  if (!gpu_profile_enabled()) return 0;
  QueryPerformanceCounter(&t);
  return (uint64_t)t.QuadPart;
}
static uint64_t profile_bin, profile_upload, profile_submit, profile_readback;
static uint64_t profile_gpu_ns, profile_gpu_samples, profile_gpu_skips;
static uint64_t profile_hist[6], profile_flush[11];
enum { PF_DEFAULT, PF_PIXELS_GROW, PF_SURFACE_GROW, PF_TEXTURE_GROW,
       PF_ATLAS, PF_TARGET, PF_FULL, PF_PER_CALL, PF_PER_CALL_END,
       PF_SURFACE_UPLOAD, PF_READBACK };
static unsigned profile_reason;
typedef struct { ID3D11Query *start, *end, *disjoint; int pending; } GpuProfileQuery;
static GpuProfileQuery profile_queries[64];
static unsigned profile_cursor;
static void gpu_profile_collect(void) {
  if (!gpu_profile_enabled()) return;
  for (unsigned k = 0; k < 64; ++k) {
    GpuProfileQuery *q = &profile_queries[k];
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT d;
    UINT64 a, b;
    if (!q->pending || ID3D11DeviceContext_GetData(ctx, (ID3D11Asynchronous *)q->disjoint,
           &d, sizeof(d), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) continue;
    if (ID3D11DeviceContext_GetData(ctx, (ID3D11Asynchronous *)q->start, &a, sizeof(a), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK ||
        ID3D11DeviceContext_GetData(ctx, (ID3D11Asynchronous *)q->end, &b, sizeof(b), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) continue;
    q->pending = 0;
    if (!d.Disjoint && d.Frequency && b >= a) {
      profile_gpu_ns += (uint64_t)((double)(b-a)*1e9/(double)d.Frequency);
      profile_gpu_samples++;
    }
  }
}
static GpuProfileQuery *gpu_profile_begin(void) {
  if (!gpu_profile_enabled()) return NULL;
  /* Poll only the slot about to be reused: constant overhead per batch. */
  GpuProfileQuery *q = &profile_queries[profile_cursor++ % 64];
  if (q->pending) { gpu_profile_collect(); if (q->pending) { profile_gpu_skips++; return NULL; } }
  D3D11_QUERY_DESC d = {D3D11_QUERY_TIMESTAMP, 0};
  if (!q->start && FAILED(ID3D11Device_CreateQuery(dev, &d, &q->start))) return NULL;
  if (!q->end && FAILED(ID3D11Device_CreateQuery(dev, &d, &q->end))) return NULL;
  d.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
  if (!q->disjoint && FAILED(ID3D11Device_CreateQuery(dev, &d, &q->disjoint))) return NULL;
  ID3D11DeviceContext_Begin(ctx, (ID3D11Asynchronous *)q->disjoint);
  ID3D11DeviceContext_End(ctx, (ID3D11Asynchronous *)q->start);
  return q;
}
static void gpu_profile_end(GpuProfileQuery *q) {
  if (!q) return;
  ID3D11DeviceContext_End(ctx, (ID3D11Asynchronous *)q->end);
  ID3D11DeviceContext_End(ctx, (ID3D11Asynchronous *)q->disjoint);
  q->pending = 1;
}
static void gpu_profile_cleanup(void) {
  for (unsigned k=0;k<64;k++) {
    RELEASE(profile_queries[k].start, ID3D11Query);
    RELEASE(profile_queries[k].end, ID3D11Query);
    RELEASE(profile_queries[k].disjoint, ID3D11Query);
    profile_queries[k].pending = 0;
  }
}
static void gpu_profile_report(void) {
  if (!gpu_profile_enabled()) return;
  LARGE_INTEGER f; QueryPerformanceFrequency(&f);
  double ms=1000.0/(double)f.QuadPart;
  gpu_profile_collect();
  fprintf(stderr,"[GPU-PROFILE] cpu-ms bin=%.3f upload=%.3f submit=%.3f readback=%.3f gpu-ms=%.3f sampled-batches=%llu skipped=%llu\n",
          profile_bin*ms,profile_upload*ms,profile_submit*ms,profile_readback*ms,(double)profile_gpu_ns/1e6,
          (unsigned long long)profile_gpu_samples,(unsigned long long)profile_gpu_skips);
  fprintf(stderr,"[GPU-PROFILE] batch-size 1=%llu 2-8=%llu 9-32=%llu 33-128=%llu 129-512=%llu 513+=%llu\n",
          (unsigned long long)profile_hist[0],(unsigned long long)profile_hist[1],(unsigned long long)profile_hist[2],
          (unsigned long long)profile_hist[3],(unsigned long long)profile_hist[4],(unsigned long long)profile_hist[5]);
  static const char *names[]={"other","pixels-grow","surface-grow","texture-grow","atlas","target","full","per-call","per-call-end","surface-upload","readback"};
  fprintf(stderr,"[GPU-PROFILE] flush");
  for(unsigned k=0;k<11;k++)fprintf(stderr," %s=%llu",names[k],(unsigned long long)profile_flush[k]);
  fputc('\n',stderr);
}
static int flush_queue_reason(unsigned reason) {
  unsigned previous=profile_reason; profile_reason=reason;
  int ok=flush_queue(); profile_reason=previous;
  return ok;
}
