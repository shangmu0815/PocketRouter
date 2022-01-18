#include <stdio.h>

#include "downloadtest.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/convert_string.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/util.h"

#define DEBUG_DT        1

static const uint32_t INDEX_START_BTN = 0;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;
static lv_style_t style_btn_ina;
static lv_style_t style_font_small_u;
static lv_style_t style_font_small_d;
static lv_style_t style_font_med;
static lv_style_t style_font_bold_small;
static lv_style_t style_font_bold;
static lv_style_t style_line;
static lv_style_t style_font_color;
static lv_style_t style_s_btn_rel;
static lv_style_t style_s_btn_pr;

static lv_task_t * lv_task;
static lv_obj_t * lmeter;
static lv_obj_t * lmeter_label;
static lv_obj_t * mbps_label;
static lv_obj_t * start_btn;
static lv_obj_t * download;

static int run_times = 0;
static float total_value = 0.0;
float allbytes = 0.0;

static const float MAX_DOWNLOAD_VALUE = 150.0;
static const float MIN_DOWNLOAD_VALUE = 0.0;

static const char* EMPTY = "--";
static const char* END_TAG = " (";

//for progress bar animation
lv_task_t * anim_arc_task;
int prev_st_pb;
int tar_st_pb;
int st_pb_cnt;

static const float rxtest_array[11] =
    {24920, 24920, 24920, 25073, 25226, 25286, 25305, 25398, 25432, 25496, 25503};
static const float txtest_array[11] =
    {157835, 157835, 158011, 158011, 158011, 158065, 158106, 158195, 158206, 158263, 158299};


const void * arc_map_dt[]={&icon_chart_22, &icon_chart_21, &icon_chart_20
        , &icon_chart_19, &icon_chart_18, &icon_chart_17, &icon_chart_16
        , &icon_chart_15, &icon_chart_14, &icon_chart_13, &icon_chart_12
        , &icon_chart_11, &icon_chart_10, &icon_chart_9, &icon_chart_8
        , &icon_chart_7, &icon_chart_6, &icon_chart_5, &icon_chart_4
        , &icon_chart_3, &icon_chart_2, &icon_chart_1};

static int ARC_MAP_LENGTH = sizeof(arc_map_dt) / sizeof(void *);

//show progress bar arc animation
void anim_arc_dt() {
    int len;

    if (tar_st_pb > prev_st_pb) {
        len = tar_st_pb - prev_st_pb;
        if (st_pb_cnt < len) {//progress bar increase
            int arc = prev_st_pb + st_pb_cnt;
            arc = (arc >= ARC_MAP_LENGTH) ? ARC_MAP_LENGTH - 1:arc;
            arc = (arc < 0) ? 0:arc;
            lv_img_set_src(lmeter, arc_map_dt[arc]);
            st_pb_cnt++;
        } else {
            //reset
            prev_st_pb = tar_st_pb - 1;
            st_pb_cnt = 0;
        }
    } else {
        len = prev_st_pb - tar_st_pb;
        if (st_pb_cnt < len) {//progress bar decrease
            int arc = prev_st_pb - st_pb_cnt;
            arc = (arc >= ARC_MAP_LENGTH) ? ARC_MAP_LENGTH - 1:arc;
            arc = (arc < 0) ? 0:arc;
            lv_img_set_src(lmeter, arc_map_dt[arc]);
            st_pb_cnt++;
        } else {
            prev_st_pb = tar_st_pb - 1;
            st_pb_cnt = 0;
        }
    }
}

void nextTask_dt() {
    total_value = 0;
    run_times = 0;
    lv_task_set_period(lv_task, LV_TASK_PRIO_OFF);
    lv_task_del(lv_task);
    lv_task = NULL;
    if (anim_arc_task != NULL) {
        lv_task_set_period(anim_arc_task, LV_TASK_PRIO_OFF);
        lv_task_del(anim_arc_task);
        anim_arc_task = NULL;
    }
    lv_btn_set_state(start_btn, LV_BTN_STATE_REL);
}

int getPercentage_dt(float value, float max, float min) {
    if (value > max) {
        return 100;
    } else if (value < min) {
        return 1;
    }

    // (value-min)/(max -min)
    int percentage = (int)(((value-min)/(max -min)) * 100);
    log_d("Percentage: %d", percentage);

    //set target progress bar value, res should be between 0~22
    int res = (int)(percentage * ARC_MAP_LENGTH / 100);
    return res;
}

float readBytes() {
    float bytes = 0.0;

    FILE *fp = NULL;
    static const char readCmd[] = "ifconfig wlan0 | grep \"RX bytes\"";

#ifdef FEATURE_ROUTER
    fp = popen(readCmd, "r");
    if (fp == NULL) {
        return bytes;
    }
#endif

    char buffer[1024];
#ifdef FEATURE_ROUTER
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (DEBUG_DT) log_d("buffer: %s", buffer);
        bytes = string_to_int_by_string(buffer, "RX bytes:", END_TAG);
        if (DEBUG_DT) log_d("rxbytes: %f", bytes);
        bytes = bytes + string_to_int_by_string(buffer, "TX bytes:", END_TAG);
#else
        bytes = rxtest_array[run_times] + txtest_array[run_times];
#endif
        if (DEBUG_DT) log_d("bytes: %f", bytes);
#ifdef FEATURE_ROUTER
    }
#endif

#ifdef FEATURE_ROUTER
    pclose(fp);
#endif
    return bytes;
}

void run_task_dt(void *p) {
    run_times++;
    log_d("run_times: %d", run_times);

    float value = 0.0;
    float new_rxbytes = readBytes();
    value = (new_rxbytes - allbytes);
    if (DEBUG_DT) log_d("value: %f", value);
    allbytes = new_rxbytes;

    total_value += value;;

    char str_value[16];
    char str_avg[16];
    memset(str_avg, '\0', sizeof(str_avg));
    memset(str_value, '\0', sizeof(str_value));
    snprintf(str_value, sizeof(str_value), "%d", (int)value);
    snprintf(str_avg, sizeof(str_avg), "%.1f", (total_value/(float)run_times));
    lv_label_set_text(download, str_avg);
    tar_st_pb = getPercentage_dt(value, MAX_DOWNLOAD_VALUE, MIN_DOWNLOAD_VALUE);

    lv_label_set_text(lmeter_label, str_value);
    lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

    if (run_times == 10) {
        nextTask_dt();
    }
}

void exec_speed_task_dt() {
    run_times = 0;
    total_value = 0;
    prev_st_pb = 0;
    tar_st_pb = 0;

    //init rxbytes
    allbytes = readBytes();

    lv_label_set_text(download, EMPTY);
    lv_label_set_text(lmeter_label, EMPTY);
    lv_obj_set_hidden(mbps_label, 0);
    lv_label_set_text(mbps_label, get_string(ID_BYTES));
    lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(mbps_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
    lv_btn_set_state(start_btn, LV_BTN_STATE_INA);
    lv_task = lv_task_create(run_task_dt, 1000, LV_TASK_PRIO_LOW, NULL);
    anim_arc_task = lv_task_create(anim_arc_dt, 50, LV_TASK_PRIO_LOW, NULL);
}

void start_btn_action_dt(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    exec_speed_task_dt();
}

void dt_close_cb(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    if (lv_task != NULL) {
        lv_task_set_period(lv_task, LV_TASK_PRIO_OFF);
        lv_task_del(lv_task);
        lv_task = NULL;
    }
    if (anim_arc_task != NULL) {
        lv_task_set_period(anim_arc_task, LV_TASK_PRIO_OFF);
        lv_task_del(anim_arc_task);
        anim_arc_task = NULL;
    }

    //close window
    //TODO need to fix home btn crash issue
    //lv_obj_t * win = lv_win_get_from_btn(btn);
    //lv_obj_del(win);
}

void downloadtest_create(void) {

    lv_style_copy(&style_btn_rel, &lv_style_plain);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_50;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_20);
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.body.border.opa = LV_OPA_COVER;
    style_btn_pr.text.color = LV_COLOR_BASE;

    lv_style_copy(&style_s_btn_rel, &style_btn_rel);
    style_s_btn_rel.body.border.part = LV_BORDER_TOP;

    lv_style_copy(&style_s_btn_pr, &style_s_btn_rel);
    style_s_btn_pr.body.border.color = LV_COLOR_BASE;
    style_s_btn_pr.body.border.width = 3;
    style_s_btn_pr.body.border.opa = LV_OPA_COVER;
    style_s_btn_pr.text.color = LV_COLOR_BASE;

    lv_style_copy(&style_btn_ina, &style_btn_rel);
    style_btn_ina.body.border.color = LV_COLOR_SILVER;
    style_btn_ina.text.color = LV_COLOR_SILVER;

    lv_style_copy(&style_font_bold, &lv_style_plain);
    style_font_bold.text.color = LV_COLOR_NIGHT_RIDER;
    style_font_bold.text.font = get_font(font_w_bold, font_h_40);

    lv_style_copy(&style_font_small_u, &lv_style_plain);
    style_font_small_u.text.color = LV_COLOR_GREYISH_BROWN;
    style_font_small_u.text.font = get_font(font_w_regular, font_h_14);
    style_font_small_u.text.letter_space = 0;

    lv_style_copy(&style_font_small_d, &style_font_small_u);
    style_font_small_d.text.font = get_font(font_w_bold, font_h_12);
    style_font_small_d.text.letter_space = 0;

    lv_style_copy(&style_font_bold_small, &lv_style_plain);
    style_font_bold_small.text.color = LV_COLOR_NIGHT_RIDER;
    style_font_bold_small.text.font = get_font(font_w_bold, font_h_12);
    style_font_bold_small.text.letter_space = 0;

    lv_style_copy(&style_font_med, &lv_style_plain);
    style_font_med.text.color = LV_COLOR_NIGHT_RIDER;
    style_font_med.text.font = get_font(font_w_bold, font_h_16);

    lv_style_copy(&style_font_color, &lv_style_plain);
    style_font_color.text.color = LV_COLOR_BASE;
    style_font_color.text.font = get_font(font_w_regular, font_h_22);
    style_font_color.text.letter_space = 0;

    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_SILVER;

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_LAUNCHER_DOWNLOADTEST),
    dt_close_cb);
    lv_obj_set_size(win, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    //remove scroll bar
    lv_win_set_sb_mode(win, LV_SB_MODE_OFF);

    lmeter = lv_img_create(win, NULL);
    lv_obj_set_size(lmeter, 170 * LV_RES_OFFSET, 128 * LV_RES_OFFSET);
    lv_img_set_src(lmeter, arc_map_dt[0]);
    lv_obj_align(lmeter, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    //Add a label to show current value
    lmeter_label = lv_label_create(lmeter, NULL);
    lv_label_set_text(lmeter_label, EMPTY);
    lv_label_set_style(lmeter_label, LV_LABEL_STYLE_MAIN, &style_font_bold);
    lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

    //Add a label to mbps
    mbps_label = lv_label_create(lmeter, NULL);
    lv_label_set_text(mbps_label, get_string(ID_BYTES));
    lv_label_set_style(mbps_label, LV_LABEL_STYLE_MAIN, &style_font_med);
    lv_obj_align(mbps_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
    lv_obj_set_hidden(mbps_label, 1);

    //draw slow
    lv_obj_t * slow = lv_label_create(lmeter, NULL);
    lv_label_set_text(slow, get_string(ID_SPEED_TEST_SLOW));
    lv_label_set_style(slow, LV_LABEL_STYLE_MAIN, &style_font_bold_small);
    lv_obj_align(slow, lmeter, LV_ALIGN_IN_BOTTOM_LEFT, 10 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //draw fast
    lv_obj_t * fast = lv_label_create(lmeter, slow);
    lv_label_set_text(fast, get_string(ID_SPEED_TEST_FAST));
    lv_obj_align(fast, lmeter, LV_ALIGN_IN_BOTTOM_RIGHT, -10 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //draw start button
    start_btn = lv_btn_create(win, NULL);
    lv_obj_set_user_data(start_btn, INDEX_START_BTN);
    lv_obj_set_event_cb(start_btn, start_btn_action_dt);
    lv_obj_set_size(start_btn, LV_HOR_RES_MAX * LV_RES_OFFSET, 50 * LV_RES_OFFSET);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_REL, &style_s_btn_rel);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_PR, &style_s_btn_pr);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_INA, &style_btn_ina);
    lv_obj_align(start_btn, win, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t * btn_label = lv_label_create(start_btn, NULL);
    lv_label_set_text(btn_label, get_string(ID_SPEED_TEST_START));

    //draw download part
    lv_obj_t * d_btn = lv_btn_create(win, NULL);
    lv_obj_set_size(d_btn, 110, 34);
    lv_btn_set_state(d_btn, LV_BTN_STATE_INA);
    lv_btn_set_style(d_btn, LV_BTN_STYLE_INA, &style_btn_rel);
    lv_btn_set_layout(d_btn, LV_LAYOUT_OFF);
    lv_obj_align(d_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 60);

    lv_obj_t *  dowload_ic = lv_img_create(d_btn, NULL);
    lv_obj_set_size(dowload_ic, 24, 34);
    lv_img_set_src(dowload_ic, &ic_speedtest_download_wb);
    lv_obj_align(dowload_ic, d_btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

    //Add a label for Download
    lv_obj_t * d_label;
    d_label = lv_label_create(d_btn, NULL);
    lv_label_set_text(d_label, get_string(ID_DOWNLOAD));

    lv_label_set_style(d_label, LV_LABEL_STYLE_MAIN, &style_font_small_u);
    lv_obj_align(d_label, d_btn, LV_ALIGN_IN_TOP_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add Download value
    download = lv_label_create(d_btn, NULL);
    lv_label_set_text(download, EMPTY);
    lv_label_set_style(download, LV_LABEL_STYLE_MAIN, &style_font_color);
    lv_obj_align(download, d_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0 * LV_RES_OFFSET, -2 * LV_RES_OFFSET);

    //Add Mbps
    lv_obj_t * mbps;
    mbps = lv_label_create(d_btn, NULL);
    char tmp[20];
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "/%s", get_string(ID_BYTES));
    lv_label_set_text(mbps, tmp);
    lv_label_set_style(mbps, LV_LABEL_STYLE_MAIN, &style_font_small_d);
    lv_obj_align(mbps, d_btn, LV_ALIGN_IN_BOTTOM_RIGHT, -5 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //show page to page exit part
    page_anim_exit();
}
