#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "battery_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/battery/battery_info.h"

#if FEATURE_ROUTER
#define LOG_FILE                        "/data/misc/battery_log.txt"
#define LOG_BACKUP_FILE                 "/data/misc/battery_log_bak.txt"
#else
#define LOG_FILE                        "Data_Store/battery_log.txt"
#define LOG_BACKUP_FILE                 "Data_Store/battery_log_bak.txt"
#endif

#define LEVEL_OFFSET                    5

#define LEVEL_HEADER                    "Battery:"
#define PLUGIN_HEADER                   "Plugin:"
#define PLUGOUT_HEADER                  "Plugout:"
#define TEMP_HEADER                     "Temperature:"
#define CPU_TEMP_HEADER                 "CPU temperature:"
#define CURRENT_HEADER                  "Current:"
#define VOLTAGE_HEADER                  "Voltage:"
#define DRIVER_CURRENT_HEADER           "Driver workqueue current:"
#define REGISTER1340_HEADER             "Register 1340[0]:"
#define DATETIME_LENGTH                 17
#define DUMP_CHARGE_INFO_INTERVAL       300000

static bool init_done = false;
static int battery_level = 0;
static int plugin_count = 0;

void dump_time_stamp(char* timestamp, int stamp_len) {
    if (stamp_len > DATETIME_LENGTH) return;

    time_t t = time(NULL);
    struct tm *now = localtime(&t);

    memset(timestamp, 0, DATETIME_LENGTH);
    strftime(timestamp, DATETIME_LENGTH, "%Y-%m-%d %H:%M", now);
}

void dump_level() {
#if (0) // replaced with dump_charge_info()
    char datetime[DATETIME_LENGTH];
    char cmd[150];
    memset(cmd, 0, sizeof(cmd));

    dump_time_stamp(&datetime, sizeof(datetime));
    sprintf(cmd, "echo %s %s %d%%, %s %d, %s %dmA >> %s", datetime, LEVEL_HEADER, battery_level,
            TEMP_HEADER, get_battery_temperature(), CURRENT_HEADER, get_charging_current(), LOG_FILE);
    systemCmd(cmd);
#endif
}

void dump_charge_info() {
    char datetime[DATETIME_LENGTH];
    char cmd[270];
    memset(cmd, 0, sizeof(cmd));

    dump_time_stamp(&datetime, sizeof(datetime));
    sprintf(cmd, "echo %s %s %d%%, %s %d, %s %d, %s %dmA, %s %d, %s %d, %s %d >> %s", datetime,
            LEVEL_HEADER, get_battery_info(), TEMP_HEADER, get_battery_temperature(),
            CPU_TEMP_HEADER, get_cpu_temperature(),
            CURRENT_HEADER, get_charging_current(), VOLTAGE_HEADER, get_battery_voltage(),
            DRIVER_CURRENT_HEADER, get_driver_workqueue_current(), REGISTER1340_HEADER,
            get_register_1340(), LOG_FILE);
    systemCmd(cmd);
}

void dump_plugin(int plug_type) {
    char cmd[100];
    char header[9];
    char datetime[DATETIME_LENGTH];

    dump_time_stamp(&datetime, sizeof(datetime));
    strcpy(header, (plug_type == PLUG_IN) ? PLUGIN_HEADER : PLUGOUT_HEADER);
    plugin_count++;
    sprintf(cmd, "echo %s %s%d %dmAh >> %s", datetime, header, plugin_count, get_battery_capacity(), LOG_FILE);
    systemCmd(cmd);
}

int parse_from_dump(char * header, char * footer) {
    int count = 0;

    FILE *fp = NULL;

    char cmd[60];
    sprintf(cmd, "grep \"%s\" %s | tail -n 1", header, LOG_FILE);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("error parsing battery log info!");
        return count;
    }

    char buffer[50];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    char *head = NULL;
    head = strstr(buffer, header);
    if (head != NULL) {
        head += (strlen(header));
        int len = strstr(head, footer) - head;
        char found[len + 1];
        memset(found, 0, sizeof(found));
        strncpy(found, head, len);
        count = atoi(found);
    }
    pclose(fp);
    return count;
}

int get_level_count() {
    return parse_from_dump(LEVEL_HEADER, "%");
}

int get_plugin_count() {
    int in_count = parse_from_dump(PLUGIN_HEADER, " ");
    int out_count = parse_from_dump(PLUGOUT_HEADER, " ");
    return (in_count > out_count) ? in_count : out_count;
}

void log_file_backup() {
    struct stat buffer;
    int exist = stat(LOG_FILE, &buffer);

    if (buffer.st_size > 512000) {
        char cmd[70];
        sprintf(cmd, "mv %s %s", LOG_FILE, LOG_BACKUP_FILE);
        systemCmd(cmd);
        log_d("mv battery log to backup file");
        exist = stat(LOG_FILE, &buffer);
    }

    if (exist != 0 || buffer.st_size == 0) {
        log_d("init battery log file");
        dump_level();
        dump_charge_info();
    }
}

void init_battery_log() {
    if (init_done) return;

    log_file_backup();
    battery_level = get_level_count();
    plugin_count = get_plugin_count();

    init_done = true;
}

void battery_log_level(int level) {
    if (!init_done) return;

    if (level >= (battery_level + LEVEL_OFFSET) ||
        level <= (battery_level - LEVEL_OFFSET) ||
        (level == 100 && battery_level != 100))
    {
        battery_level = level;
        dump_level();
    }
}

void battery_log_plugin(bool charging) {
    if (!init_done) return;

    dump_plugin((charging) ? PLUG_IN : PLUG_OUT);
}

void battery_log_charge_info() {
    if (!init_done) return;

    static uint32_t timestamp;
    if (!is_charging()) {
        timestamp = lv_tick_get();
        return;
    }

    uint32_t t = lv_tick_elaps(timestamp);
    if (t > DUMP_CHARGE_INFO_INTERVAL) {
        timestamp = lv_tick_get();
        dump_charge_info();
        // because log been dump frequently while charging, check for backup prevent log become large
        log_file_backup();
    }
}
