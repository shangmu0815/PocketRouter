#include "lvgl/lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN uint8_t ic_status_signal_4gp_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
  /*Pixel format: Alpha 8 bit, Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x1b, 0x49, 0xb3, 0x49, 0x1b, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x1b, 0x49, 0x80, 0x49, 0xff, 0x49, 0x80, 0x49, 0x1b, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0x14, 0x6d, 0x03, 0x00, 0x00, 0x49, 0xb3, 0x49, 0xff, 0x49, 0xff, 0x49, 0xff, 0x49, 0xb3, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x47, 0x49, 0xf4, 0x49, 0xf4, 0x49, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x49, 0x17, 0x49, 0xaf, 0x49, 0xf4, 0x49, 0xff, 0x49, 0xfb, 0x49, 0xc4, 0x49, 0x47, 0x49, 0x80, 0x49, 0xff, 0x49, 0x80, 0x49, 0x1b, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0xb3, 0x49, 0xff, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x49, 0xc8, 0x49, 0xff, 0x49, 0xfc, 0x49, 0xd0, 0x49, 0xf7, 0x49, 0xff, 0x49, 0xe7, 0x49, 0x24, 0x49, 0xb3, 0x49, 0x1b, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x20, 0x49, 0xfc, 0x49, 0xf7, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x34, 0x49, 0xff, 0x49, 0xff, 0x49, 0x98, 0x00, 0x00, 0x49, 0x68, 0x49, 0xff, 0x49, 0xff, 0x49, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x8b, 0x49, 0xff, 0x49, 0xab, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x5c, 0x49, 0xff, 0x49, 0xff, 0x49, 0x6c, 0x00, 0x00, 0x49, 0x40, 0x49, 0xff, 0x49, 0xff, 0x49, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x6d, 0x08, 0x49, 0xec, 0x49, 0xc8, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x74, 0x49, 0xff, 0x49, 0xff, 0x49, 0x64, 0x00, 0x00, 0x6d, 0x18, 0x49, 0x6c, 0x49, 0x6c, 0x49, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x49, 0x63, 0x49, 0xff, 0x49, 0x6c, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x77, 0x49, 0xff, 0x49, 0xff, 0x49, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x49, 0xcf, 0x49, 0xfb, 0x6d, 0x14, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x77, 0x49, 0xff, 0x49, 0xff, 0x49, 0x64, 0x49, 0x38, 0x49, 0xa7, 0x49, 0xa7, 0x49, 0xa7, 0x49, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x49, 0x3b, 0x49, 0xff, 0x49, 0xb4, 0x00, 0x00, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x77, 0x49, 0xff, 0x49, 0xff, 0x49, 0x64, 0x49, 0x50, 0x49, 0xf0, 0x49, 0xff, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x49, 0x74, 0x49, 0xff, 0x49, 0xf4, 0x49, 0xe8, 0x49, 0xf4, 0x49, 0xff, 0x49, 0xfb, 0x49, 0xe8, 0x49, 0x7f, 0x49, 0xff, 0x49, 0xff, 0x49, 0x64, 0x00, 0x00, 0x00, 0x00, 0x49, 0xff, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x49, 0x70, 0x49, 0xf4, 0x49, 0xf4, 0x49, 0xf4, 0x49, 0xfb, 0x49, 0xff, 0x49, 0xfc, 0x49, 0xf4, 0x49, 0x6b, 0x49, 0xff, 0x49, 0xff, 0x49, 0x6c, 0x00, 0x00, 0x6d, 0x0b, 0x49, 0xff, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x49, 0x3c, 0x49, 0xff, 0x49, 0xff, 0x49, 0x9f, 0x00, 0x00, 0x49, 0x43, 0x49, 0xff, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0xff, 0x00, 0x49, 0xd7, 0x49, 0xff, 0x49, 0xff, 0x49, 0xc4, 0x49, 0xec, 0x49, 0xfc, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x84, 0x49, 0xff, 0x49, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x49, 0x28, 0x49, 0xcf, 0x49, 0xff, 0x49, 0xff, 0x49, 0xd4, 0x49, 0x77, 0x49, 0xff, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x10, 0x49, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x1b, 0x49, 0x4a, 0xb3, 0x6a, 0x52, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x1b, 0x6a, 0x52, 0x80, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x80, 0x6a, 0x52, 0x1b, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0x5a, 0x14, 0xab, 0x5a, 0x03, 0x00, 0x00, 0x00, 0x49, 0x4a, 0xb3, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xb3, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x47, 0x69, 0x4a, 0xf4, 0x69, 0x4a, 0xf4, 0x49, 0x4a, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x17, 0x69, 0x4a, 0xaf, 0x49, 0x4a, 0xf4, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xfb, 0x49, 0x4a, 0xc4, 0x6a, 0x52, 0x47, 0x6a, 0x52, 0x80, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x80, 0x6a, 0x52, 0x1b, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x4a, 0xb3, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0xc8, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xfc, 0x69, 0x4a, 0xd0, 0x69, 0x4a, 0xf7, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xe7, 0x6a, 0x52, 0x24, 0x49, 0x4a, 0xb3, 0x6a, 0x52, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x20, 0x69, 0x4a, 0xfc, 0x49, 0x4a, 0xf7, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x34, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x98, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x68, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x8b, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xab, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x5c, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x6c, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x40, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0x5a, 0x08, 0x49, 0x4a, 0xec, 0x69, 0x4a, 0xc8, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x74, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x64, 0x00, 0x00, 0x00, 0x8a, 0x52, 0x18, 0x69, 0x4a, 0x6c, 0x69, 0x4a, 0x6c, 0x6a, 0x52, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x63, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x6c, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x77, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x4a, 0xcf, 0x49, 0x4a, 0xfb, 0xab, 0x5a, 0x14, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x77, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x64, 0x6a, 0x52, 0x38, 0x69, 0x4a, 0xa7, 0x69, 0x4a, 0xa7, 0x69, 0x4a, 0xa7, 0x6a, 0x52, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x6a, 0x52, 0x3b, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xb4, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x77, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x64, 0x6a, 0x52, 0x50, 0x49, 0x4a, 0xf0, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x49, 0x4a, 0x74, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xf4, 0x49, 0x4a, 0xe8, 0x69, 0x4a, 0xf4, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xfb, 0x49, 0x4a, 0xe8, 0x49, 0x4a, 0x7f, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x6a, 0x52, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x69, 0x4a, 0x70, 0x69, 0x4a, 0xf4, 0x69, 0x4a, 0xf4, 0x69, 0x4a, 0xf4, 0x49, 0x4a, 0xfb, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xfc, 0x69, 0x4a, 0xf4, 0x69, 0x4a, 0x6b, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x6c, 0x00, 0x00, 0x00, 0xec, 0x62, 0x0b, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x3c, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0x9f, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x43, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x69, 0x4a, 0xd7, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xc4, 0x49, 0x4a, 0xec, 0x69, 0x4a, 0xfc, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4a, 0x84, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0x28, 0x69, 0x4a, 0xcf, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0xff, 0x69, 0x4a, 0xd4, 0x69, 0x4a, 0x77, 0x49, 0x4a, 0xff, 0x49, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x52, 0x10, 0x6a, 0x52, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit  BUT the 2  color bytes are swapped*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x1b, 0x4a, 0x49, 0xb3, 0x52, 0x6a, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x1b, 0x52, 0x6a, 0x80, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x80, 0x52, 0x6a, 0x1b, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0xab, 0x14, 0x5a, 0xab, 0x03, 0x00, 0x00, 0x00, 0x4a, 0x49, 0xb3, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xb3, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x47, 0x4a, 0x69, 0xf4, 0x4a, 0x69, 0xf4, 0x4a, 0x49, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x17, 0x4a, 0x69, 0xaf, 0x4a, 0x49, 0xf4, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xfb, 0x4a, 0x49, 0xc4, 0x52, 0x6a, 0x47, 0x52, 0x6a, 0x80, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x80, 0x52, 0x6a, 0x1b, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x49, 0xb3, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0xc8, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xfc, 0x4a, 0x69, 0xd0, 0x4a, 0x69, 0xf7, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xe7, 0x52, 0x6a, 0x24, 0x4a, 0x49, 0xb3, 0x52, 0x6a, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x20, 0x4a, 0x69, 0xfc, 0x4a, 0x49, 0xf7, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x34, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x98, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x68, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x8b, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xab, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x5c, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x6c, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x40, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0xab, 0x08, 0x4a, 0x49, 0xec, 0x4a, 0x69, 0xc8, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x74, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x64, 0x00, 0x00, 0x00, 0x52, 0x8a, 0x18, 0x4a, 0x69, 0x6c, 0x4a, 0x69, 0x6c, 0x52, 0x6a, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x63, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x6c, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x77, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x49, 0xcf, 0x4a, 0x49, 0xfb, 0x5a, 0xab, 0x14, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x77, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x64, 0x52, 0x6a, 0x38, 0x4a, 0x69, 0xa7, 0x4a, 0x69, 0xa7, 0x4a, 0x69, 0xa7, 0x52, 0x6a, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x52, 0x6a, 0x3b, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xb4, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x77, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x64, 0x52, 0x6a, 0x50, 0x4a, 0x49, 0xf0, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x4a, 0x49, 0x74, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xf4, 0x4a, 0x49, 0xe8, 0x4a, 0x69, 0xf4, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xfb, 0x4a, 0x49, 0xe8, 0x4a, 0x49, 0x7f, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x52, 0x6a, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x4a, 0x69, 0x70, 0x4a, 0x69, 0xf4, 0x4a, 0x69, 0xf4, 0x4a, 0x69, 0xf4, 0x4a, 0x49, 0xfb, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xfc, 0x4a, 0x69, 0xf4, 0x4a, 0x69, 0x6b, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x6c, 0x00, 0x00, 0x00, 0x62, 0xec, 0x0b, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x3c, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0x9f, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x43, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x4a, 0x69, 0xd7, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xc4, 0x4a, 0x49, 0xec, 0x4a, 0x69, 0xfc, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x69, 0x84, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0x28, 0x4a, 0x69, 0xcf, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0xff, 0x4a, 0x69, 0xd4, 0x4a, 0x69, 0x77, 0x4a, 0x49, 0xff, 0x4a, 0x49, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x8a, 0x10, 0x52, 0x6a, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 32
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x4e, 0x4e, 0x1b, 0x4a, 0x4a, 0x4a, 0xb3, 0x4e, 0x4e, 0x4e, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x4e, 0x4e, 0x1b, 0x4d, 0x4d, 0x4d, 0x80, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x80, 0x4e, 0x4e, 0x4e, 0x1b, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x14, 0x55, 0x55, 0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x4a, 0x4a, 0xb3, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xb3, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x47, 0x4b, 0x4b, 0x4b, 0xf4, 0x4b, 0x4b, 0x4b, 0xf4, 0x4a, 0x4a, 0x4a, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x4e, 0x4e, 0x17, 0x4b, 0x4b, 0x4b, 0xaf, 0x4a, 0x4a, 0x4a, 0xf4, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xfb, 0x4a, 0x4a, 0x4a, 0xc4, 0x4d, 0x4d, 0x4d, 0x47, 0x4d, 0x4d, 0x4d, 0x80, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x80, 0x4e, 0x4e, 0x4e, 0x1b, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x4a, 0x4a, 0xb3, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0xc8, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xfc, 0x4b, 0x4b, 0x4b, 0xd0, 0x4b, 0x4b, 0x4b, 0xf7, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xe7, 0x4e, 0x4e, 0x4e, 0x24, 0x4a, 0x4a, 0x4a, 0xb3, 0x4e, 0x4e, 0x4e, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x20, 0x4b, 0x4b, 0x4b, 0xfc, 0x4a, 0x4a, 0x4a, 0xf7, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x34, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x98, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x68, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x8b, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xab, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x5c, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4c, 0x4c, 0x4c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x4c, 0x4c, 0x40, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x08, 0x4a, 0x4a, 0x4a, 0xec, 0x4b, 0x4b, 0x4b, 0xc8, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x74, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x64, 0x00, 0x00, 0x00, 0x00, 0x52, 0x52, 0x52, 0x18, 0x4c, 0x4c, 0x4c, 0x6c, 0x4c, 0x4c, 0x4c, 0x6c, 0x4e, 0x4e, 0x4e, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x63, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x6c, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x4c, 0x4c, 0x77, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x4a, 0x4a, 0xcf, 0x4a, 0x4a, 0x4a, 0xfb, 0x55, 0x55, 0x55, 0x14, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x4c, 0x4c, 0x77, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x64, 0x4d, 0x4d, 0x4d, 0x38, 0x4b, 0x4b, 0x4b, 0xa7, 0x4b, 0x4b, 0x4b, 0xa7, 0x4b, 0x4b, 0x4b, 0xa7, 0x4d, 0x4d, 0x4d, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x4e, 0x4e, 0x4e, 0x3b, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x4c, 0x4c, 0x77, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x64, 0x4d, 0x4d, 0x4d, 0x50, 0x4a, 0x4a, 0x4a, 0xf0, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x4a, 0x4a, 0x4a, 0x74, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xf4, 0x4a, 0x4a, 0x4a, 0xe8, 0x4b, 0x4b, 0x4b, 0xf4, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xfb, 0x4a, 0x4a, 0x4a, 0xe8, 0x4a, 0x4a, 0x4a, 0x7f, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4d, 0x4d, 0x4d, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x70, 0x4b, 0x4b, 0x4b, 0xf4, 0x4b, 0x4b, 0x4b, 0xf4, 0x4b, 0x4b, 0x4b, 0xf4, 0x4a, 0x4a, 0x4a, 0xfb, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xfc, 0x4b, 0x4b, 0x4b, 0xf4, 0x4c, 0x4c, 0x4c, 0x6b, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x5d, 0x5d, 0x5d, 0x0b, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x3c, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x43, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x4b, 0x4b, 0x4b, 0xd7, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xc4, 0x4a, 0x4a, 0x4a, 0xec, 0x4b, 0x4b, 0x4b, 0xfc, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x4b, 0x84, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x4d, 0x4d, 0x28, 0x4b, 0x4b, 0x4b, 0xcf, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0xff, 0x4b, 0x4b, 0x4b, 0xd4, 0x4c, 0x4c, 0x4c, 0x77, 0x4a, 0x4a, 0x4a, 0xff, 0x4a, 0x4a, 0x4a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0x50, 0x10, 0x4d, 0x4d, 0x4d, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
};

lv_img_dsc_t ic_status_signal_4gp = {
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 20,
  .data_size = 440 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = ic_status_signal_4gp_map,
};
