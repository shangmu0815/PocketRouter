
#ifndef LV_POCKET_ROUTER_SRC_UTIL_PAGE_ANIM_H_
#define LV_POCKET_ROUTER_SRC_UTIL_PAGE_ANIM_H_

#include "../../../lvgl/lvgl.h"

void page_anim_enter();
void page_anim_exit();
void page_animate_impl(lv_obj_t * obj, int type, uint16_t time, uint16_t delay, bool is_out);

#define ANIM_TIME ((uint32_t)275)
#define ANIM_DELAY_TIME ((uint32_t)250)

enum ANIM_FLOAT_TYPE{
    PAGE_ANIM_FLOAT_LEFT,
    PAGE_ANIM_FLOAT_RIGHT,
    PAGE_ANIM_FLOAT_BOTTOM,
    PAGE_ANIM_FLOAT_TOP,
};

#endif /* LV_POCKET_ROUTER_SRC_UTIL_PAGE_ANIM_H_ */
