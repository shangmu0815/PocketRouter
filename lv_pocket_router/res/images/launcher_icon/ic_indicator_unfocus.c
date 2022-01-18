#include "lvgl/lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN uint8_t ic_indicator_unfocus_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
  /*Pixel format: Alpha 8 bit, Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0x1c, 0xff, 0xa7, 0xff, 0xe7, 0xff, 0xe7, 0xff, 0xa7, 0xff, 0x1c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0x1c, 0xff, 0xec, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec, 0xff, 0x1c, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xa7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa7, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xa7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa7, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0x1c, 0xff, 0xec, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec, 0xff, 0x1c, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0x1c, 0xff, 0xa7, 0xff, 0xe7, 0xff, 0xe7, 0xff, 0xa7, 0xff, 0x1c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb, 0xde, 0x1c, 0xdb, 0xde, 0xa7, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xa7, 0xfb, 0xde, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xfb, 0xde, 0x1c, 0xdb, 0xde, 0xec, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xec, 0xfb, 0xde, 0x1c, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xdb, 0xde, 0xa7, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xa7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xe7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xe7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xdb, 0xde, 0xa7, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xa7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xfb, 0xde, 0x1c, 0xdb, 0xde, 0xec, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xff, 0xdb, 0xde, 0xec, 0xfb, 0xde, 0x1c, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb, 0xde, 0x1c, 0xdb, 0xde, 0xa7, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xe7, 0xdb, 0xde, 0xa7, 0xfb, 0xde, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit  BUT the 2  color bytes are swapped*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xfb, 0x1c, 0xde, 0xdb, 0xa7, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xa7, 0xde, 0xfb, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xfb, 0x1c, 0xde, 0xdb, 0xec, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xec, 0xde, 0xfb, 0x1c, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xdb, 0xa7, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xa7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xe7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xe7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xdb, 0xa7, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xa7, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xde, 0xfb, 0x1c, 0xde, 0xdb, 0xec, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xff, 0xde, 0xdb, 0xec, 0xde, 0xfb, 0x1c, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xfb, 0x1c, 0xde, 0xdb, 0xa7, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xe7, 0xde, 0xdb, 0xa7, 0xde, 0xfb, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 32
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0xdb, 0x1c, 0xd9, 0xd9, 0xd9, 0xa7, 0xd8, 0xd8, 0xd8, 0xe7, 0xd8, 0xd8, 0xd8, 0xe7, 0xd9, 0xd9, 0xd9, 0xa7, 0xdb, 0xdb, 0xdb, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0xdb, 0x1c, 0xd8, 0xd8, 0xd8, 0xec, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xec, 0xdb, 0xdb, 0xdb, 0x1c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xd9, 0xd9, 0xd9, 0xa7, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd9, 0xd9, 0xd9, 0xa7, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xd8, 0xd8, 0xd8, 0xe7, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xe7, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xd8, 0xd8, 0xd8, 0xe7, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xe7, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xd9, 0xd9, 0xd9, 0xa7, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd9, 0xd9, 0xd9, 0xa7, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0xdb, 0x1c, 0xd8, 0xd8, 0xd8, 0xec, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xff, 0xd8, 0xd8, 0xd8, 0xec, 0xdb, 0xdb, 0xdb, 0x1c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xdb, 0xdb, 0x1c, 0xd9, 0xd9, 0xd9, 0xa7, 0xd8, 0xd8, 0xd8, 0xe7, 0xd8, 0xd8, 0xd8, 0xe7, 0xd9, 0xd9, 0xd9, 0xa7, 0xdb, 0xdb, 0xdb, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
};

lv_img_dsc_t ic_indicator_unfocus = {
  .header.always_zero = 0,
  .header.w = 10,
  .header.h = 10,
  .data_size = 100 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = ic_indicator_unfocus_map,
};
