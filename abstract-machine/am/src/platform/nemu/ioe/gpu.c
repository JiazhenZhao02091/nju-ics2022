#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  /*
  int i;
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  uint32_t *fb = (uint32_t *)(uintptr_t) FB_ADDR;
  for(i = 0; i  < w * h; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
  */
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // uint32_t tmp = inl(VGACTL_ADDR);
  // uint16_t h = (uint16_t)(tmp & 0x0000ffff);
  // uint16_t w = (uint16_t)(tmp >> 16);
  // *cfg = (AM_GPU_CONFIG_T) {
  //   .present = true, .has_accel = false,
  //   .width = w, .height = h,
  //   .vmemsz = 0
  // };

  uint32_t screen_wh = inl(VGACTL_ADDR);
  uint32_t h = screen_wh & 0xffff;
  uint32_t w = screen_wh >> 16;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0)) return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y+h; i++) {
    for (int j = x; j < x+w; j++) {
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  // int w = io_read(AM_GPU_CONFIG).width;
  // uint32_t *dst = (uint32_t *)(uintptr_t) FB_ADDR;
  // uint32_t *src = (uint32_t *)(uintptr_t) ctl -> pixels;
  // // 对应位置像素点的信息
  // for(int i = 0; i < ctl -> h; i++) {
  //   for(int j = 0; j < ctl -> w; j++) {
  //     dst[(ctl -> y) * w + i * w + ctl -> x + j] = src[i * (ctl -> w) + j];
  //   }
  // }
  // if (ctl->sync) {
  //   outl(SYNC_ADDR, 1);
  // }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
