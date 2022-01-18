LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := lvgl_demo
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DUSE_ANDROID_FBDEV=1 -DUSE_EVDEV=1 -DHIGH_RESOLUTION=1 -DANDROID_LOG=1 -std=gnu99 -DANDROID_PROPERTY=1 -DDEBUG_MEM=1 -DANDROID_BUILD
LOCAL_CFLAGS += -DFEATURE_ROUTER
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/lv_drivers \
                     $(LOCAL_PATH)/lvgl \
                     $(LOCAL_PATH)/lv_pocket_router/src
#LOCAL_SHARED_LIBRARIES := liblvgldev liblvgl
LOCAL_STATIC_LIBRARIES := liblvgldev liblvgl
LOCAL_SHARED_LIBRARIES += libc libcutils liblog libxml2

LOCAL_SRC_FILES := main.c \
                   lv_examples/lv_apps/demo/demo.c \
                   lv_examples/lv_apps/demo/img_bubble_pattern.c

LOCAL_SRC_FILES += $(call all-c-files-under,lv_pocket_router)

#LOCAL_FORCE_STATIC_EXECUTABLE := true
#LOCAL_VENDOR_MODULE := true
include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))
