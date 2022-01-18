#include <assert.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "util.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/util/power_menu.h"

static bool silent_reboot = false;
static bool storage_full_prompted = false;
static bool task_debug_enabled = false;
static bool charge_mode_enabled = false;
static bool reboot_screen_on_enable = false;

// flag to keep a record if shown fota upgrade prompt to user yet in this boot up
// only show once per boot up
static bool upgrade_notified = false;

#ifndef COREDUMP_DEBUG
lv_font_t * font_regular[LOCALE_COUNT][font_h_count] = {
    // EN
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // JP
#ifdef JP_AR_FONT
    { NULL, &MPLUS1p_Medium_14, NULL, &MPLUS1p_Medium_26, NULL, &MPLUS1p_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#else
    { NULL, &NotoSansCJKjp_Medium_14, NULL, &NotoSansCJKjp_Medium_26, NULL, &NotoSansCJKjp_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#endif
    // FR
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // DE
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // AR
    { NULL, &NotoSansArabic_ExtraCondensedMedium_14, NULL, &NotoSansArabic_ExtraCondensedMedium_26, NULL, &NotoSansArabic_ExtraCondensedMedium_26, NULL, NULL, NULL, NULL, NULL, NULL },
    // PT
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // IT
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // ES
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // ZH_CN
#ifdef JP_AR_FONT
    { NULL, &MPLUS1p_Medium_14, NULL, &MPLUS1p_Medium_26, NULL, &MPLUS1p_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#else
    { NULL, &NotoSansCJKjp_Medium_14, NULL, &NotoSansCJKjp_Medium_26, NULL, &NotoSansCJKjp_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#endif
    // ZH_TW
#ifdef JP_AR_FONT
    { NULL, &MPLUS1p_Medium_14, NULL, &MPLUS1p_Medium_26, NULL, &MPLUS1p_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#else
    { NULL, &NotoSansCJKjp_Medium_14, NULL, &NotoSansCJKjp_Medium_26, NULL, &NotoSansCJKjp_Medium_26, NULL, NULL, NULL, NULL, NULL, NULL },
#endif
    // SL
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // NL
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // RU
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // PL
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
    // HU
    { NULL, &Oswald_Medium_14, NULL, &Oswald_Medium_22, NULL, &Oswald_Medium_22, NULL, NULL, NULL, NULL, NULL, NULL },
};

lv_font_t * font_bold[LOCALE_COUNT][font_h_count] = {
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
#ifdef JP_AR_FONT
    { &MPLUS1p_Bold_16, NULL, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24 },
#else
    { &NotoSansCJKjp_Bold_16, NULL, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24 },
#endif
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &NotoSansArabic_ExtraCondensedBold_18, NULL, &NotoSansArabic_ExtraCondensedBold_20, &NotoSansArabic_ExtraCondensedBold_22,
      &NotoSansArabic_ExtraCondensedBold_24, &NotoSansArabic_ExtraCondensedBold_26, &NotoSansArabic_ExtraCondensedBold_30, &NotoSansArabic_ExtraCondensedBold_32,
      &NotoSansArabic_ExtraCondensedBold_32, &NotoSansArabic_ExtraCondensedBold_40, &NotoSansArabic_ExtraCondensedBold_40, &NotoSansArabic_ExtraCondensedBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
#ifdef JP_AR_FONT
    { &MPLUS1p_Bold_16, NULL, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24 },
    { &MPLUS1p_Bold_16, NULL, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24,
      &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24, &MPLUS1p_Bold_24 },
#else
    { &NotoSansCJKjp_Bold_16, NULL, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24 },
    { &NotoSansCJKjp_Bold_16, NULL, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24,
      &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24, &NotoSansCJKjp_Bold_24 },
#endif
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    // PL
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
    // HU
    { &Oswald_SemiBold_12, NULL, &Oswald_SemiBold_16, &Oswald_SemiBold_18,
      &Oswald_SemiBold_20, &Oswald_SemiBold_22, &Oswald_SemiBold_24, &Oswald_SemiBold_26,
      &Oswald_SemiBold_30, &Oswald_SemiBold_32, &Oswald_SemiBold_40, &Oswald_SemiBold_50 },
};
#endif

lv_font_t * get_font_impl(int locale, font_weights_t w, font_heights_t h) {
    lv_font_t * font;
#ifndef COREDUMP_DEBUG
    if (w == font_w_regular) {
        font = font_regular[locale][h];
    } else {
        font = font_bold[locale][h];
    }
#else
    // for coredump debug, remove font we used since it will cause coredump parse error
    font = &lv_font_roboto_12;
#endif
    assert(font != NULL);
    return font;
}

lv_font_t * get_font(font_weights_t w, font_heights_t h) {
    return get_font_impl(get_device_locale(), w, h);
}

lv_font_t * get_locale_font(int locale, font_weights_t w, font_heights_t h) {
    return get_font_impl(locale, w, h);
}

//[ZX53] JP font were missing Latin Extended-A/B which were needed for SL and NL locale
lv_font_t * get_locale_font_cust(font_weights_t w, font_heights_t h) {
#ifndef JP_AR_FONT
    int curr_locale = get_device_locale();
    if(curr_locale == NL || curr_locale == SL){
        return get_font_impl(curr_locale, w, h);
    }
#endif
    return get_font_impl(JP, w, h);
}

bool is_ltr() {
    return isltr();
}

//[ZX53] check if txt include AR text
bool is_letter_rtl(uint32_t letter)
{
    if((letter >= 0x0600 && letter <= 0x06FF) ||
            (letter >= 0x0750  && letter <= 0x077F)  ||
            (letter >= 0x08A0  && letter <= 0x08FF)  ||
            (letter >= 0xFB50  && letter <= 0xFDFF)  ||
            (letter >= 0xFE70  && letter <= 0xFEFF)  ||
            (letter >= 0x10E60 && letter <= 0x10E7F) ||
            (letter >= 0x1EC70 && letter <= 0x1ECBF) ||
            (letter >= 0x1ED00 && letter <= 0x1ED4F) ||
            (letter >= 0x1EE00 && letter <= 0x1EEFF)) {
        return true;
    }
    return false;
}

bool is_txt_line_rtl(const char * txt, uint32_t line_start, uint32_t line_end)
{
    uint32_t i = line_start;
    uint32_t letter;
    while(i < line_end) {
        letter = lv_txt_encoded_next(txt, &i);
        if(is_letter_rtl(letter)) return true;
    }
    return false;
}

bool is_txt_rtl(const char * txt)
{
    uint32_t i = 0;
    uint32_t letter;
    while(txt[i] != '\0') {
        letter = lv_txt_encoded_next(txt, &i);
        if(is_letter_rtl(letter)) return true;
    }
    return false;
}

//[ZX53] check if txt include CJK text, below include Chinese, Japanese characters
bool is_txt_CJK(const char * txt){
    uint32_t i = 0;
    uint32_t letter;
    while(txt[i] != '\0') {
        letter = lv_txt_encoded_next(txt, &i);
        if((letter >= 0x3400 && letter <= 0x4DB5)        ||
                (letter >= 0x4E00  && letter <= 0x9FA5)  ||
                (letter >= 0x9FA6  && letter <= 0x9FBB)  ||
                (letter >= 0xF900  && letter <= 0xFA2D)  ||
                (letter >= 0xFA30  && letter <= 0xFA6A)  ||
                (letter >= 0xFA70  && letter <= 0xFAD9)  ||
                (letter >= 0x20000 && letter <= 0x2A6D6) ||
                (letter >= 0x2F800 && letter <= 0x2FA1D) ||
                (letter >= 0xFF00  && letter <= 0xFFEF)  ||
                (letter >= 0x2E80  && letter <= 0x2EFF)  ||
                (letter >= 0x3000  && letter <= 0x303F)  ||
                (letter >= 0x31C0  && letter <= 0x31EF)  ||
                (letter >= 0x3040  && letter <= 0x309F)  || //JP
                (letter >= 0x3000  && letter <= 0x30FF)  || //JP
                (letter >= 0x31F0  && letter <= 0x31FF)) {  //JP
            return true;
        }
    }
    return false;
}

int systemCmd(const char *command) {
    int ret = system(command);
    if (ret != 0) {
        log_e("Shell command: %s, failed %d, errno %d %s", command, ret, errno, strerror(errno));
    }
    return ret;
}

int read_node_value(char* path, char* value, int size) {
    static int buf_size = 100;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        log_e("open node error: %s", path);
        return ENODEV;
    }

    char buf[buf_size];
    memset(buf, 0, sizeof(buf));
    int len = read(fd, buf, sizeof(buf));
    if (len < 0) {
        log_e("read value errno %d %s: %s", errno, strerror(errno), path);
        close(fd);
        return ENODEV;
    } else if (size < len) {
        log_i("allocated size not enought to read complete value from %s", path);
    }

    close(fd);

    memset(value, 0, sizeof(value));
    memcpy(value, buf, size - 1);

    return 0;
}

void ui_cleanup() {
    close_sms(false);
    //close power menu if exist
    close_power_menu();
    close_popup();
    close_kb();
    close_all_lists(0);
    close_all_pages();
    launcher_destroy();
    db_cleanup();
}

void launch_home_behaviour() {
#ifdef CUST_DLINK
    ui_cleanup();
    dashboard_create();
#else
    close_all_lists(0);
    close_all_pages();
#endif
}

bool is_silent_reboot() {
    static bool silent_flag_init = false;

    silent_flag_init = true; //power cut feature disable

    if (!silent_flag_init) {
        int fd = open("/sys/devices/platform/c440000.qcom,spmi/spmi-0/spmi0-08/c440000.qcom,spmi:qcom,pmxprairie@8:qcom,power-on@800/pon_reason", O_RDONLY);
        if (fd < 0) {
            log_e("open pon_reason node error");
            return false;
        }
        char buf[6];
        int len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            log_e("read pon_reason node error");
            close(fd);
            return false;
        }
        close(fd);
        if (strncmp(buf, "0x20", strlen("0x20")) == 0) {
            silent_reboot = true;
            log_d("Going to boot up as silent reboot");
        }
        silent_flag_init = true;
    }
    return silent_reboot;
}

void reset_silent_reboot() {
    silent_reboot = false;
}

bool is_upgrade_notified() {
    return upgrade_notified;
}

void set_upgrade_notified() {
    upgrade_notified = true;
}

void start_sleep() {
    systemCmd("echo mem > /sys/power/autosleep");
}

void stop_sleep() {
    systemCmd("echo off > /sys/power/autosleep");
}

//return txt x size
lv_coord_t get_txt_x_size(lv_obj_t * obj, const char * txt){
    lv_point_t size;
    const lv_style_t * style = lv_obj_get_style(obj);
    lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
    if(lv_label_get_align(obj) == LV_LABEL_ALIGN_CENTER) flag |= LV_TXT_FLAG_CENTER;
    if(lv_label_get_align(obj) == LV_LABEL_ALIGN_RIGHT) flag |= LV_TXT_FLAG_RIGHT;

    lv_txt_get_size(&size, txt, style->text.font, style->text.letter_space, style->text.line_space,
                    LV_COORD_MAX, flag);

    return size.x;
}

bool fs_ready() {
    static bool ready = false;
    if (!ready) {
        struct stat buffer;
        int exist = stat(DEFAULT_DATA_STORE_FILE, &buffer);
        if (exist == 0 && buffer.st_size > 0) {
            ready = true;
        }
    }
    return ready;
}

void set_storage_full_promp_flag() {
    storage_full_prompted = true;
}

bool storage_full_check() {
#ifdef FEATURE_ROUTER
    if (!storage_full_prompted) {
        struct statvfs s;
        const char * path = "/data/";

        if (statvfs(path, &s) < 0) {
            log_e("errno: %d. Failed to get free disk space on %s", errno, path);
        } else {
            if ((int)(s.f_bsize * s.f_bfree) <= 1048576) { // less than 1MB
                return true;
            }
        }
    }
#endif
    return false;
}

bool is_task_debug_enabled() {
    return task_debug_enabled;
}

/**
 *  Check if need refresh enable/disable state of debug log in lv_task
 */
void task_debug_refresh() {
    char buf[2] = {0};

#ifdef FEATURE_ROUTER
    FILE *fp = fopen("/data/misc/pocketrouter/task_enable.flag", "r");
#else
    FILE *fp = fopen("Data_Store/task_enable.flag", "r");
#endif
    if (fp != NULL) {
        fread(buf, 1, 1, fp);
        fclose(fp);
        fp = NULL;
    } else {
        task_debug_enabled = false;
    }

    if(strncmp(buf, "1", 1) == 0 && !task_debug_enabled) {
        task_debug_enabled = true;
        lv_task_debug_enable(true);
    } else if (strncmp(buf, "0", 1) == 0 && task_debug_enabled) {
        task_debug_enabled = false;
        lv_task_debug_enable(false);
    }
}

bool charge_mode() {
#if !USE_FBDEV && !USE_ANDROID_FBDEV
    // no LCM product, no need to enter charging only mode
    return false;
#elif defined(NO_BATTERY)
    // no battery product, no need to enter charging only mode
    return false;
#else
    static bool charge_mode_init = false;

    if (!charge_mode_init) {
        int fd = open("/sys/devices/platform/c440000.qcom,spmi/spmi-0/spmi0-08/c440000.qcom,spmi:qcom,pmxprairie@8:qcom,power-on@800/pon_reason", O_RDONLY);
        if (fd < 0) {
            log_e("open charge_mode pon_reason node error");
            return false;
        }
        char buf[6];
        int len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            log_e("read charge_mode pon_reason node error");
            close(fd);
            return false;
        }
        close(fd);
        if (strncmp(buf, "0x40", strlen("0x40")) == 0) {
            get_battery_info();
            dump_usb_state();
            if(is_charging()){
                if (get_battery_present()) {
                    log_d("Going to boot up as charge_mode");
                    charge_mode_enabled = true;
                }
            } else {
                log_d("Not in charge mode, power off");
                charging_only_power_off();
            }
        }
        charge_mode_init = true;
    }
    return charge_mode_enabled;
#endif
}

void set_reboot_screen_on(bool enable){
    reboot_screen_on_enable = enable;
}

bool get_reboot_screen_on(){
    return reboot_screen_on_enable;
}

// workaround to fix mobileap_cfg.xml unsync after upgrade from C2 to CS9
void convert_mobileapcfg() {
    FILE *fp = popen("grep \"EthIfaceName>eth0\" /systemrw/data/mobileap_cfg.xml", "r");
    if (fp == NULL) {
        log_e("grep mobileap_cfg.xml failed");
    }

    char buffer[120];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) > 0) {
        log_d("No need to convert mobileap cfg");
    } else {
        log_d("Need to convert mobileap cfg");

        int res;
        struct stat buffer;
        lstat("/systemrw/data/mobileap_cfg.xml", &buffer);
        if (S_ISLNK(buffer.st_mode)) {
            // for v124 to v179 workaround
            systemCmd("unlink /systemrw/data/mobileap_cfg.xml");
            res = systemCmd("cp -p /oem/data/factory_mobileap_cfg.xml /systemrw/data/mobileap_cfg.xml");
            log_d("cp /oem/data/factory_mobileap_cfg.xml cmd ret:%d", res);
        } else {
            // for v150 to v179 workaround
            res = systemCmd("sh /usr/bin/cei_conv_qcmap_c2_conf_to_cs9.sh");
            log_d("cei_conv_qcmap_c2_conf_to_cs9 cmd ret:%d", res);
        }
        if (res == 0) {
            // set wifi config flag to false for reboot to perform after wlan init finishes
            ds_set_bool(DS_KEY_WIFI_REBOOT_FLAG, false);
        }
    }
    pclose(fp);
}

#define BT_BUF_SIZE 30
void trace_dump() {
    int nptrs;
    void *buffer[BT_BUF_SIZE];
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    //log_e("backtrace() returned %d addresses", nptrs);
    if (nptrs) {
        log_e("Backtrace:");
    } else {
        log_e("No Backtrace");
    }

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        log_e("backtrace_symbols error: %s", strerror(errno));
        return;
    }

    for (int j = 0; j < nptrs; j++)
        log_e("  %s", strings[j]);

    free(strings);
}

#ifdef CUST_LUXSHARE
void luxshare_init() {
    char cmd[80];
    bool enabled = ds_get_bool(DS_KEY_CHARGE_ENABLE);
    sprintf(cmd, "echo %s > /sys/module/smb138x_charger/parameters/batt_temp_monitor_disabled", (enabled) ? "0" : "1");
    systemCmd(cmd);
    sprintf(cmd, "echo %s > /sys/class/power_supply/battery/input_suspend", (enabled) ? "0" : "1");
    systemCmd(cmd);
}
#endif
