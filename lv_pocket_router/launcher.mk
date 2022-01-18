CSRCS += launcher.c

DEPPATH += --dep-path lv_pocket_router
VPATH += :lv_pocket_router

CFLAGS += "-I$(LVGL_DIR)/lv_pocket_router"
