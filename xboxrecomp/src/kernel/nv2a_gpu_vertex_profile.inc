/* Same opt-in flag as raster profiling. Totals include every batch; readback
 * percentiles sample every 32nd successful batch, retaining the latest 4096.
 * QPC only: no GPU wait, flush, query or extra memory transfer is introduced. */
static uint64_t vp_program, vp_buffers, vp_upload, vp_submit, vp_readback,
    vp_map, vp_whole, vp_batches, vp_serial, vp_hist[6], vp_serial_hist[6];
static uint64_t vp_size_whole[6], vp_size_readback[6], vp_serial_whole, vp_serial_readback;
static uint64_t vp_read_samples[4096], vp_read_seen;
static unsigned vp_read_count;
static unsigned vp_bucket(size_t n) {
  return n==1?0:n<=8?1:n<=32?2:n<=128?3:n<=512?4:5;
}
static void vertex_profile_finish(uint64_t whole, uint64_t readback,
                                   size_t count, int serial) {
  if (!gpu_profile_enabled()) return;
  uint64_t duration=gpu_profile_ticks()-whole;
  unsigned bucket=vp_bucket(count);
  vp_whole += duration;
  vp_size_whole[bucket]+=duration; vp_size_readback[bucket]+=readback;
  vp_batches++;
  vp_hist[vp_bucket(count)]++;
  if (serial) {vp_serial++;vp_serial_hist[bucket]++;vp_serial_whole+=duration;vp_serial_readback+=readback;}
  if ((vp_batches & 31u)==0) {
    vp_read_samples[vp_read_seen++ % 4096] = readback;
    if (vp_read_count < 4096) vp_read_count++;
  }
}
static int vertex_profile_order(const void *a,const void *b) {
  uint64_t x=*(const uint64_t *)a,y=*(const uint64_t *)b;
  return (x>y)-(x<y);
}
static void vertex_profile_report(void) {
  if (!gpu_profile_enabled()) return;
  LARGE_INTEGER f;QueryPerformanceFrequency(&f);
  double ms=1000.0/(double)f.QuadPart;
  uint64_t samples[4096], p95=0;
  if (vp_read_count) {
    memcpy(samples,vp_read_samples,vp_read_count*sizeof(*samples));
    qsort(samples,vp_read_count,sizeof(*samples),vertex_profile_order);
    p95=samples[(vp_read_count*95u+99u)/100u-1u];
  }
  fprintf(stderr,"[GPU-VSH-PROFILE] batches=%llu parallel=%llu serial=%llu cpu-ms program=%.3f buffers=%.3f upload=%.3f submit=%.3f readback=%.3f map=%.3f whole=%.3f readback-p95-us=%.3f samples=%u cadence=32 window=4096\n",
          (unsigned long long)vp_batches,(unsigned long long)(vp_batches-vp_serial),(unsigned long long)vp_serial,
          vp_program*ms,vp_buffers*ms,vp_upload*ms,vp_submit*ms,vp_readback*ms,vp_map*ms,vp_whole*ms,p95*ms*1000,vp_read_count);
  fprintf(stderr,"[GPU-VSH-PROFILE] batch-size 1=%llu 2-8=%llu 9-32=%llu 33-128=%llu 129-512=%llu 513+=%llu serial-size=%llu,%llu,%llu,%llu,%llu,%llu\n",
          (unsigned long long)vp_hist[0],(unsigned long long)vp_hist[1],(unsigned long long)vp_hist[2],
          (unsigned long long)vp_hist[3],(unsigned long long)vp_hist[4],(unsigned long long)vp_hist[5],
          (unsigned long long)vp_serial_hist[0],(unsigned long long)vp_serial_hist[1],(unsigned long long)vp_serial_hist[2],
          (unsigned long long)vp_serial_hist[3],(unsigned long long)vp_serial_hist[4],(unsigned long long)vp_serial_hist[5]);
  fprintf(stderr,"[GPU-VSH-PROFILE] whole-ms-size=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f readback-ms-size=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f serial-whole-ms=%.3f serial-readback-ms=%.3f\n",
          vp_size_whole[0]*ms,vp_size_whole[1]*ms,vp_size_whole[2]*ms,vp_size_whole[3]*ms,vp_size_whole[4]*ms,vp_size_whole[5]*ms,
          vp_size_readback[0]*ms,vp_size_readback[1]*ms,vp_size_readback[2]*ms,vp_size_readback[3]*ms,vp_size_readback[4]*ms,vp_size_readback[5]*ms,
          vp_serial_whole*ms,vp_serial_readback*ms);
}
