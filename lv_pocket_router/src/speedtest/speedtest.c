#include "speedtest.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lv_pocket_router/src/util/convert_string.h>
#include "lv_pocket_router/src/util/debug_log.h"
#include "../../res/values/styles.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/util.h"

#define SERVER_IP_FILE "/data/misc/speed_test/speed_test.txt"
#ifdef FEATURE_ROUTER
#define DNSMASQ_HOST_FILE "/var/volatile/tmp/data/dnsmasq_host.txt"
#else
#define DNSMASQ_HOST_FILE "Data_Store/dnsmasq_host.txt"
#endif

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

#ifdef OOKLA_SPEED_TEST
static pthread_t dump_ookla_thread = NULL;
static const int EXEC_DELAY = 3000; //ookla took about 3 secs to start download test
static const int RUN_INTERVAL = 170;
static const int RUN_TIMES = 100;
#else
static const int EXEC_DELAY = 0;
static const int RUN_INTERVAL = 500;
static const int RUN_TIMES = 10;
#endif

static const int TASK_PING = 0;
static const int TASK_TRANSFER_DOWNLOAD = 1;
static const int TASK_TRANSFER_UPLOAD = 2;

static lv_task_t * lv_task;
static lv_task_t * exec_task;
static lv_obj_t * lmeter;
static lv_obj_t * lmeter_label;
static lv_obj_t * mbps_label;
static lv_obj_t * start_btn;
static lv_obj_t * pin;
static lv_obj_t * download;
static lv_obj_t * upload;

static int task_id = 0;
static int run_times = 0;
static float total_value = 0.0;

static const float MAX_PING_VALUE = 1.50;
#ifdef OOKLA_SPEED_TEST
static const float MAX_DOWNLOAD_VALUE = 500.0;
static const float MAX_UPLOAD_VALUE = 500.0;
#else
static const float MAX_DOWNLOAD_VALUE = 150.0;
static const float MAX_UPLOAD_VALUE = 150.0;
#endif

static const float MIN_PING_VALUE = 0.00;
static const float MIN_DOWNLOAD_VALUE = 50.0;
static const float MIN_UPLOAD_VALUE = 50.0;

static const char* EMPTY = "--";
static const char* IPERF_START_TAG = "MBytes";
static const char* IPERF_END_TAG = " MBytes";
static const char* PING_START_TAG = "time=";
static const char* PING_END_TAG = " ms";

static const float demo_ping_array[10] =
    {0.17, 0.18, 0.19, 0.17, 0.20, 0.18, 0.17, 0.16, 0.18, 0.20};
static const float demo_download_array[10] =
    {720, 770, 800, 850, 870,900, 860, 830, 870, 900};
static const float demo_upload_array[10] =
    {740, 790, 810, 840, 880,900, 880, 860, 820, 850};

char server_ip[20];
bool demo_mode_enable;

//for progress bar animation
lv_task_t * anim_arc_task;
int prev_st_pb;
int tar_st_pb;
int st_pb_cnt;


const void * arc_map[]={&icon_chart_22, &icon_chart_21, &icon_chart_20
        , &icon_chart_19, &icon_chart_18, &icon_chart_17, &icon_chart_16
        , &icon_chart_15, &icon_chart_14, &icon_chart_13, &icon_chart_12
        , &icon_chart_11, &icon_chart_10, &icon_chart_9, &icon_chart_8
        , &icon_chart_7, &icon_chart_6, &icon_chart_5, &icon_chart_4
        , &icon_chart_3, &icon_chart_2, &icon_chart_1};

static int ARC_MAP_LENGTH = sizeof(arc_map) / sizeof(void *);

//show progress bar arc animation
void anim_arc() {
    int len;

    if (tar_st_pb > prev_st_pb) {
        len = tar_st_pb - prev_st_pb;
        if (st_pb_cnt < len) {//progress bar increase
            int arc = prev_st_pb + st_pb_cnt;
            arc = (arc >= ARC_MAP_LENGTH) ? ARC_MAP_LENGTH - 1:arc;
            arc = (arc < 0) ? 0:arc;
            lv_img_set_src(lmeter, arc_map[arc]);
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
            lv_img_set_src(lmeter, arc_map[arc]);
            st_pb_cnt++;
        } else {
            prev_st_pb = tar_st_pb - 1;
            st_pb_cnt = 0;
        }
    }
}

void nextTask() {
    total_value = 0;
    run_times = 0;
    task_id++;
    if (task_id > TASK_TRANSFER_UPLOAD) {
        if (demo_mode_enable) {
            task_id = 0;
        } else {
            lv_task_set_period(lv_task, LV_TASK_PRIO_OFF);
            lv_task_del(lv_task);
            lv_task = NULL;
            if (anim_arc_task != NULL) {
                lv_task_set_period(anim_arc_task, LV_TASK_PRIO_OFF);
                lv_task_del(anim_arc_task);
                anim_arc_task = NULL;
            }
            lv_btn_set_state(start_btn, LV_BTN_STATE_REL);

            /*if (server_ip != NULL) {
                free(server_ip);
            }*/
            lv_label_set_text(lmeter_label, EMPTY);
            lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
            lv_img_set_src(lmeter, arc_map[0]);
        }
    } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
        lv_label_set_text(lmeter_label, EMPTY);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(mbps_label, get_string(ID_MBPS));
    } else if (task_id == TASK_TRANSFER_UPLOAD) {
        lv_label_set_text(lmeter_label, EMPTY);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(mbps_label, get_string(ID_MBPS));
    }
    lv_obj_align(mbps_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
}

int getPercentage(float value, float max, float min) {
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

#ifdef OOKLA_SPEED_TEST
void update_final_value() {
    FILE *fp = NULL;
    char command[128];
    if (task_id == TASK_TRANSFER_DOWNLOAD) {
#ifdef FEATURE_ROUTER
        sprintf(command, "grep \"download: \" /data/misc/pocketrouter/ookla_result.txt");
#else
        sprintf(command, "grep \"download: \" Data_Store/ookla_result.txt");
#endif
    } else if (task_id == TASK_TRANSFER_UPLOAD) {
#ifdef FEATURE_ROUTER
        sprintf(command, "grep \"upload: \" /data/misc/pocketrouter/ookla_result.txt");
#else
        sprintf(command, "grep \"upload: \" Data_Store/ookla_result.txt");
#endif
    }
    fp = popen(command, "r");

    if (fp == NULL) {
        return ;
    }

    float value = 0.0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char* ptr = strrchr(buffer, ':');
        if (ptr != NULL) {
            value = atof(ptr + 1);
            value = value / 1000; // to Mbytes
        }

        char str_value[16];
        memset(str_value, '\0', sizeof(str_value));

        if (task_id == TASK_TRANSFER_DOWNLOAD) {
            snprintf(str_value, sizeof(str_value), "%.1f", value);
            lv_label_set_text(download, str_value);
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            snprintf(str_value, sizeof(str_value), "%.1f", value);
            lv_label_set_text(upload, str_value);
        }

        lv_label_set_text(lmeter_label, str_value);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

        break;
    }

    pclose(fp);
}

void test_terminate_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    if (strcmp(txt, get_string(ID_OK)) == 0) {
        close_popup();
    }
}

void run_task(void *p) {
    static int retry_cnt = 0; // retry count of percentage grep
    static int timeout_cnt = 0; // count of how many fail times of retry cnt

    reset_screen_timeout();

    FILE *fp = NULL;
    char command[128];
    if (task_id == TASK_TRANSFER_DOWNLOAD) {
#ifdef FEATURE_ROUTER
        sprintf(command, "grep \"Download:%d%\" /data/misc/pocketrouter/ookla_result.txt", run_times + 1);
#else
        sprintf(command, "grep \"Download:%d%\" Data_Store/ookla_result.txt", run_times + 1);
#endif
    } else if (task_id == TASK_TRANSFER_UPLOAD) {
#ifdef FEATURE_ROUTER
        sprintf(command, "grep \"Upload:%d%\" /data/misc/pocketrouter/ookla_result.txt", run_times + 1);
#else
        sprintf(command, "grep \"Upload:%d%\" Data_Store/ookla_result.txt", run_times + 1);
#endif
    }

    fp = popen(command, "r");

    if (fp == NULL) {
        nextTask();
        return ;
    }

    float value = 0.0;
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        run_times++;
        retry_cnt = 0;
        timeout_cnt = 0;
        char* ptr = strrchr(buffer, ':');
        if (ptr != NULL) {
            value = atof(ptr + 1);
            value = value / 1000.0; // to Mbytes
            if (value < 0) value = 0; // do not show negative values
        }

        total_value = value;

        char str_value[16];
        memset(str_value, '\0', sizeof(str_value));

        if (task_id == TASK_TRANSFER_DOWNLOAD) {
            snprintf(str_value, sizeof(str_value), "%.1f", value);
            lv_label_set_text(download, str_value);
            tar_st_pb = getPercentage(value, MAX_DOWNLOAD_VALUE, MIN_DOWNLOAD_VALUE);
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            snprintf(str_value, sizeof(str_value), "%.1f", value);
            lv_label_set_text(upload, str_value);
            tar_st_pb = getPercentage(value, MAX_UPLOAD_VALUE, MIN_UPLOAD_VALUE);
        }

        lv_label_set_text(lmeter_label, str_value);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
    } else {
        retry_cnt++;
        if (retry_cnt > 40) {
           // skip to next percent in case ookla dump did not include result at particular percentage
           run_times++;
           retry_cnt = 0;
           timeout_cnt++;
        }
    }

    if (run_times == RUN_TIMES) {
        update_final_value(); // final value don't seem to be avg of running values, so show final value again
        nextTask();
    }

    if (timeout_cnt >= 3) {
        // stop this test
        timeout_cnt = 0;
        task_id = TASK_TRANSFER_UPLOAD;
        nextTask();

        static const char *btns[2];
        btns[0] = get_string(ID_OK);
        btns[1] = "";
        popup_anim_not_create(get_string(ID_SPEED_TEST_ERROR), btns, test_terminate_action, NULL);
    }

    pclose(fp);
}
#else
void run_task(void *p) {
    reset_screen_timeout();

    if (demo_mode_enable) {
        float value = 0.0;
        if (task_id == TASK_PING) {
            value = demo_ping_array[run_times];
        } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
            value = demo_download_array[run_times];
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            value = demo_upload_array[run_times];
        }

        total_value += value;

        char str_value[16];
        char str_avg[16];
        memset(str_avg, '\0', sizeof(str_avg));
        memset(str_value, '\0', sizeof(str_value));

        if (task_id == TASK_PING) {
            snprintf(str_value, sizeof(str_value), "%.2f", value);
            snprintf(str_avg, sizeof(str_avg), "%.2f", (total_value/(float)run_times));
            lv_label_set_text(pin, str_avg);
            tar_st_pb = getPercentage(value, MAX_PING_VALUE, MIN_PING_VALUE);
        } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
            snprintf(str_value, sizeof(str_value), "%d", (int)value);
            snprintf(str_avg, sizeof(str_avg), "%.1f", (total_value/(float)run_times));
            lv_label_set_text(download, str_avg);
            tar_st_pb = getPercentage(value, MAX_DOWNLOAD_VALUE, MIN_DOWNLOAD_VALUE);
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            snprintf(str_value, sizeof(str_value), "%d", (int)value);
            snprintf(str_avg, sizeof(str_avg), "%.1f", (total_value/(float)run_times));
            lv_label_set_text(upload, str_avg);
            tar_st_pb = getPercentage(value, MAX_UPLOAD_VALUE, MIN_UPLOAD_VALUE);
        }

        lv_label_set_text(lmeter_label, str_value);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

        if (++run_times == RUN_TIMES) {
            nextTask();
        }
        return ;
    }

    FILE *fp = NULL;
    char command[128];
    if (task_id == TASK_PING) {
        sprintf(command, "ping -c 1 -i 1 %s", server_ip);
    } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
        sprintf(command, "iperf3 -c %s -f M -t 1 -R", server_ip);
    } else if (task_id == TASK_TRANSFER_UPLOAD) {
        sprintf(command, "iperf3 -c %s -f M -t 1", server_ip);
    }
    fp = popen(command, "r");

    if (fp == NULL) {
        nextTask();
        return ;
    }

    run_times++;

    float value = 0.0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (task_id == TASK_PING) {
            value = string_to_float_by_string(buffer, PING_START_TAG, PING_END_TAG);
        } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
            value = string_to_int_by_string(buffer, IPERF_START_TAG, IPERF_END_TAG);
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            value = string_to_int_by_string(buffer, IPERF_START_TAG, IPERF_END_TAG);
        }

        if (value == 0.0) {
            continue;
        }

        if (task_id != TASK_PING) {
            value = value * 8; // to Mbytes
        }

        total_value += value;;

        //log_d("buffer:%s", buffer);
        //log_d("value:%.1f", value);

        char str_value[16];
        char str_avg[16];
        memset(str_avg, '\0', sizeof(str_avg));
        memset(str_value, '\0', sizeof(str_value));

        if (task_id == TASK_PING) {
            snprintf(str_value, sizeof(str_value), "%.2f", value);
            snprintf(str_avg, sizeof(str_avg), "%.2f", (total_value/(float)run_times));
            lv_label_set_text(pin, str_avg);
            //set target progress bar value, should be between 0~22
            tar_st_pb = getPercentage(value, MAX_PING_VALUE, MIN_PING_VALUE);
        } else if (task_id == TASK_TRANSFER_DOWNLOAD) {
            snprintf(str_value, sizeof(str_value), "%d", (int)value);
            snprintf(str_avg, sizeof(str_avg), "%.1f", (total_value/(float)run_times));
            lv_label_set_text(download, str_avg);
            tar_st_pb = getPercentage(value, MAX_DOWNLOAD_VALUE, MIN_DOWNLOAD_VALUE);
        } else if (task_id == TASK_TRANSFER_UPLOAD) {
            snprintf(str_value, sizeof(str_value), "%d", (int)value);
            snprintf(str_avg, sizeof(str_avg), "%.1f", (total_value/(float)run_times));
            lv_label_set_text(upload, str_avg);
            tar_st_pb = getPercentage(value, MAX_UPLOAD_VALUE, MIN_UPLOAD_VALUE);
        }

        lv_label_set_text(lmeter_label, str_value);
        lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

        break;
    }

    if (run_times == RUN_TIMES) {
        nextTask();
    }
    pclose(fp);
}
#endif

void load_server_ip() {
    bool dnsmasq = false;

    memset(server_ip, 0, sizeof(server_ip));
    strcpy(server_ip, "127.0.0.1");

    FILE *fp = fopen(SERVER_IP_FILE, "r+");
    if (fp == NULL) {
        log_e("Not found %s", SERVER_IP_FILE);
        fp = fopen(DNSMASQ_HOST_FILE, "r+");
        dnsmasq = true;
        if (fp == NULL) {
            log_e("Not found %s", DNSMASQ_HOST_FILE);
            return ;
        }
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (dnsmasq) {
            char* ipaddr = strstr(buffer, " ");
            strcpy(server_ip, ipaddr);
            server_ip[strcspn(server_ip, "\n")] = 0;
        } else {
            strcpy(server_ip, buffer);
        }
        break;
    }
    fclose(fp);
}

#if defined(OOKLA_SPEED_TEST) & defined(FEATURE_ROUTER)
int ookla_dump_thread() {
    int status;
    systemCmd("rm /data/misc/pocketrouter/ookla_result.txt");
    status = systemCmd("/sbin/ookla --configurl=http://www.speedtest.net/api/embed/trial/config -p -f legacy > /data/misc/pocketrouter/ookla_result.txt");
    log_d("ookla Dump ookla result status: %d", status);
    dump_ookla_thread = NULL;
    pthread_exit(NULL);
    return 0;
}

void ookla_dump() {
    if (dump_ookla_thread == NULL) {
        // create thread to dump ookla download/upload results
        memset(&dump_ookla_thread, 0, sizeof(pthread_t));
        int res = pthread_create(&dump_ookla_thread, NULL, ookla_dump_thread, NULL);
        if (res) log_e("create ookla dump thread fail");
    }
}
#endif

void exec_speed_task() {
    lv_task_del(exec_task);
    exec_task = NULL;

    load_server_ip();
    log_d("server_ip:%s", server_ip);

#ifdef OOKLA_SPEED_TEST
    task_id = TASK_TRANSFER_DOWNLOAD;
#else
    task_id = TASK_PING;
#endif
    run_times = 0;
    total_value = 0;
    prev_st_pb = 0;
    tar_st_pb = 0;

#ifndef OOKLA_SPEED_TEST
    lv_label_set_text(pin, EMPTY);
#endif
    lv_label_set_text(download, EMPTY);
    lv_label_set_text(upload, EMPTY);
    lv_label_set_text(lmeter_label, EMPTY);
    lv_obj_set_hidden(mbps_label, 0);
#ifdef OOKLA_SPEED_TEST
    lv_label_set_text(mbps_label, get_string(ID_MBPS));
#else
    lv_label_set_text(mbps_label, get_string(ID_MS));
#endif
    lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(mbps_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
    lv_btn_set_state(start_btn, LV_BTN_STATE_INA);
    lv_task = lv_task_create(run_task, RUN_INTERVAL, LV_TASK_PRIO_LOW, NULL);
    anim_arc_task = lv_task_create(anim_arc, 50, LV_TASK_PRIO_LOW, NULL);
    int status = systemCmd("iperf3 -s -D");
    log_d("Start iperf3 daemon status: %d", status);
}

void popup_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    if (strcmp(txt, get_string(ID_OK)) == 0) { //ok
        if (lv_task != NULL) {
            return;
        }

#if defined(OOKLA_SPEED_TEST) & defined(FEATURE_ROUTER)
        ookla_dump();
#endif

        exec_task = lv_task_create(exec_speed_task, EXEC_DELAY, LV_TASK_PRIO_LOW, NULL);
        lv_task_once(exec_task);
#ifndef OOKLA_SPEED_TEST
        lv_label_set_text(pin, EMPTY);
#endif
        lv_label_set_text(download, EMPTY);
        lv_label_set_text(upload, EMPTY);
        lv_label_set_text(lmeter_label, EMPTY);
        lv_obj_set_hidden(mbps_label, true);
        lv_btn_set_state(start_btn, LV_BTN_STATE_INA);
    }
    close_popup();
}

void start_btn_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    if (lv_btn_get_state(btn) == LV_BTN_STATE_INA) return;

    static const char *btns[3];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = get_string(ID_OK);
    btns[2] = "";

    popup_anim_que_create(get_string(ID_SPEED_TEST_WARNING), btns, popup_action, NULL);
}

void st_close_cb(lv_obj_t * btn, lv_event_t event) {
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
    if (exec_task != NULL) {
        lv_task_del(exec_task);
        exec_task = NULL;
    }
    //close window
    //TODO need to fix home btn crash issue
    //lv_obj_t * win = lv_win_get_from_btn(btn);
    //lv_obj_del(win);

}

//Add draft version for speed test
void speedtest_create(void) {
    int tmp_len;
    int ms_len = strlen(get_string(ID_MS));
    int mbps_len = strlen(get_string(ID_MBPS));
    if(ms_len > mbps_len){
        tmp_len = ms_len + 1;
    }else{
        tmp_len = mbps_len + 1;
    }
    char tmp[tmp_len];

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
#if defined(HIGH_RESOLUTION)
    style_font_color.text.font = get_font(font_w_bold, font_h_30);
#else
    //all language use the same font on test result to avoid UI issue
    style_font_color.text.font = get_locale_font(EN, font_w_bold, font_h_22);
    style_font_color.text.letter_space = 0;
#endif

    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_SILVER;

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_SPEED_TEST),
	    st_close_cb);
    lv_obj_set_size(win, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    //remove scroll bar
    lv_win_set_sb_mode(win, LV_SB_MODE_OFF);

    lmeter = lv_img_create(win, NULL);
    lv_obj_set_size(lmeter, 170 * LV_RES_OFFSET, 128 * LV_RES_OFFSET);
    lv_img_set_src(lmeter, arc_map[0]);
    lv_obj_align(lmeter, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    //Add a label to show current value
    lmeter_label = lv_label_create(lmeter, NULL);
    lv_label_set_text(lmeter_label, EMPTY);
    lv_label_set_style(lmeter_label, LV_LABEL_STYLE_MAIN, &style_font_bold);
    lv_obj_align(lmeter_label, NULL, LV_ALIGN_CENTER, 0, 0);

    //Add a label to mbps
    mbps_label = lv_label_create(lmeter, NULL);
#ifdef OOKLA_SPEED_TEST
    lv_label_set_text(mbps_label, get_string(ID_MBPS));
#else
    lv_label_set_text(mbps_label, get_string(ID_MS));
#endif
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
    lv_obj_set_event_cb(start_btn, start_btn_action);
    lv_obj_set_size(start_btn, LV_HOR_RES_MAX * LV_RES_OFFSET, 50 * LV_RES_OFFSET);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_REL, &style_s_btn_rel);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_PR, &style_s_btn_pr);
    lv_btn_set_style(start_btn, LV_BTN_STYLE_INA, &style_btn_ina);
    lv_obj_align(start_btn, win, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t * btn_label = lv_label_create(start_btn, NULL);
    lv_label_set_text(btn_label, get_string(ID_SPEED_TEST_START));

    lv_obj_t * ps_btn = NULL;
    lv_obj_t * pinspeed_ic = NULL;
#ifndef OOKLA_SPEED_TEST
    //draw pin speed part
    ps_btn = lv_btn_create(win, NULL);
    lv_obj_set_size(ps_btn, 110, 34);
    lv_btn_set_state(ps_btn, LV_BTN_STATE_INA);
    lv_btn_set_style(ps_btn, LV_BTN_STYLE_INA, &style_btn_rel);
    lv_btn_set_layout(ps_btn, LV_LAYOUT_OFF);
    lv_obj_align(ps_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 13);

    pinspeed_ic = lv_img_create(ps_btn, NULL);
    lv_obj_set_size(pinspeed_ic, 24, 34);
    lv_img_set_src(pinspeed_ic, &ic_speedtest_pinspeed_wb);
    lv_obj_align(pinspeed_ic, ps_btn, LV_ALIGN_IN_LEFT_MID, 0, 0);
#endif

    //draw download part
    lv_obj_t * d_btn = lv_btn_create(win, ps_btn);
#ifdef OOKLA_SPEED_TEST
    lv_obj_set_size(d_btn, 110, 34);
    lv_btn_set_state(d_btn, LV_BTN_STATE_INA);
    lv_btn_set_style(d_btn, LV_BTN_STYLE_INA, &style_btn_rel);
    lv_btn_set_layout(d_btn, LV_LAYOUT_OFF);
    lv_obj_align(d_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 13);
#else
    lv_obj_align(d_btn, ps_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
#endif
    lv_obj_t *  dowload_ic = lv_img_create(d_btn, pinspeed_ic);

    lv_img_set_src(dowload_ic, &ic_speedtest_download_wb);
    lv_obj_align(dowload_ic, d_btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

    //draw upload part
    lv_obj_t * u_btn = lv_btn_create(win, d_btn);
    lv_obj_align(u_btn, d_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lv_obj_t *  upload_ic = lv_img_create(u_btn, dowload_ic);
    lv_img_set_src(upload_ic, &ic_speedtest_upload_wb);
    lv_obj_align(upload_ic, u_btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

#ifndef OOKLA_SPEED_TEST
    //Add a label for pin
    lv_obj_t * pin_label;
    pin_label = lv_label_create(ps_btn, NULL);
    lv_label_set_text(pin_label, get_string(ID_PING));
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(pin_label, &style_font_bold);
#else
    lv_label_set_style(pin_label, LV_LABEL_STYLE_MAIN, &style_font_small_u);
#endif
    lv_obj_align(pin_label, ps_btn, LV_ALIGN_IN_TOP_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add pin value
    pin = lv_label_create(ps_btn, NULL);
    lv_label_set_text(pin, EMPTY);
    lv_label_set_style(pin, LV_LABEL_STYLE_MAIN, &style_font_color);
    lv_obj_align(pin, ps_btn, LV_ALIGN_IN_BOTTOM_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add ms
    lv_obj_t * ms;
    ms = lv_label_create(ps_btn, NULL);
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "/%s", get_string(ID_MS));
    lv_label_set_text(ms, tmp);
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(ms, &style_font_bold);
#else
    lv_label_set_style(ms, LV_LABEL_STYLE_MAIN, &style_font_small_d);
#endif
    lv_obj_align(ms, ps_btn, LV_ALIGN_IN_BOTTOM_RIGHT, -5 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);
#endif

    //Add a label for Download
    lv_obj_t * d_label;
    d_label = lv_label_create(d_btn, NULL);
    lv_label_set_text(d_label, get_string(ID_DOWNLOAD));
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(d_label, &style_font_bold);
#else
    lv_label_set_style(d_label, LV_LABEL_STYLE_MAIN, &style_font_small_u);
#endif
    lv_obj_align(d_label, d_btn, LV_ALIGN_IN_TOP_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add Download value
    download = lv_label_create(d_btn, NULL);
    lv_label_set_text(download, EMPTY);
    lv_label_set_style(download, LV_LABEL_STYLE_MAIN, &style_font_color);
    lv_obj_align(download, d_btn, LV_ALIGN_IN_BOTTOM_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add Mbps
    lv_obj_t * mbps;
    mbps = lv_label_create(d_btn, NULL);
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "/%s", get_string(ID_MBPS));
    lv_label_set_text(mbps, tmp);
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(mbps, &style_font_bold);
#else
    lv_label_set_style(mbps, LV_LABEL_STYLE_MAIN, &style_font_small_d);
#endif
    lv_obj_align(mbps, d_btn, LV_ALIGN_IN_BOTTOM_RIGHT, -5 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add a label for Upload
    lv_obj_t * u_label;
    u_label = lv_label_create(u_btn, NULL);
    lv_label_set_text(u_label, get_string(ID_UPLOAD));
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(u_label, LV_LABEL_STYLE_MAIN, &style_font_bold);
#else
    lv_label_set_style(u_label, LV_LABEL_STYLE_MAIN, &style_font_small_u);
#endif
    lv_obj_align(u_label, u_btn, LV_ALIGN_IN_TOP_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add Upload value
    upload = lv_label_create(u_btn, NULL);
    lv_label_set_text(upload, EMPTY);
    lv_label_set_style(upload, LV_LABEL_STYLE_MAIN, &style_font_color);
    lv_obj_align(upload, u_btn, LV_ALIGN_IN_BOTTOM_LEFT, 32 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //Add Mbps
    lv_obj_t * ms1;
    ms1 = lv_label_create(u_btn, NULL);
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "/%s", get_string(ID_MBPS));
    lv_label_set_text(ms1, tmp);
#if defined(HIGH_RESOLUTION)
    lv_label_set_style(ms1, &style_font_bold);
#else
    lv_label_set_style(ms1, LV_LABEL_STYLE_MAIN, &style_font_small_d);
#endif
    lv_obj_align(ms1, u_btn, LV_ALIGN_IN_BOTTOM_RIGHT, -5 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    //for demo
    demo_mode_enable = ds_get_bool(DS_KEY_DEMO_MODE);
    if (demo_mode_enable) {
        exec_speed_task();

        //remove start key and adjust UI again
        lv_obj_set_hidden(start_btn, 1);
        lv_obj_t * page = lv_win_get_content(win);
        lv_obj_align(page, win, LV_ALIGN_CENTER, 0, 50);
    }
    //show page to page exit part
    page_anim_exit();
}
