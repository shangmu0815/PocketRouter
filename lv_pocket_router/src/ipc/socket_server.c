#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include "lv_pocket_router/src/util/power_manager.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/about/device_information.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "socket_server.h"
#include "socket_format.h"
#include "../../../lvgl/lvgl.h"

enum PROFILE_ID {
    PROFILE_NAME,
    APN,
    USER_NAME,
    PROFILE_PASSWORD,
    PDP_TYPE
};
char* packet_data;
typedef struct {
    char* profile_name;
    char* apn;
    char* user_name;
    char* password;
    char* pdp_type;
} PROFILE_CONTENT;

char *profile_info_delim_ = "`";

void socket_server_cleanup(){
    if(packet_data != NULL){
        lv_mem_free(packet_data);
        packet_data = NULL;
    }
}

void sendToClient(const int client_fd, const char* data) {
    int result = send(client_fd, data, strlen(data), 0);
    log_d("Send to client, data: %s  result: %d ", data, result);
    socket_server_cleanup();
}

char* getPackageData(const char* id, const char* value) {
    //+2: " ", '\0'
    int len = strlen(id) + strlen(value) + 2;
    packet_data = lv_mem_alloc(sizeof(char) * len);
    memset(packet_data, '\0', sizeof(char) * len);
    sprintf(packet_data, "%s %s", id, value);

    return packet_data;
}

void handleSetting(const int client_fd, const char* id, char* data) {
    if (strcmp(id, FORMAT_ID_BRIGHTNESS) == 0) {
        log_d("set brightness: %s", data);
        set_brightness(atoi(data));
        sendToClient(client_fd, getPackageData(id, "res=1"));
    }
    if (strcmp(id, FORMAT_ID_SSID) == 0) {
        log_d("set FORMAT_ID_SSID: %s", data);
        write_wlan_ssid(WIFI_BAND_24G, data);
        update_ssid_label();
        update_ssid_address();
        sendToClient(client_fd, getPackageData(id, "res=1"));
    }
    if (strcmp(id, FORMAT_ID_PASSWORD) == 0) {
        log_d("set FORMAT_ID_PASSWORD: %s", data);
        write_wlan_password(WIFI_BAND_24G, data);
        update_pw_label();
        sendToClient(client_fd, getPackageData(id, "res=1"));
    }
    if (strcmp(id, FORMAT_ID_SECURITY_TYPE) == 0) {
        log_d("set FORMAT_ID_SECURITY_TYPE: %s", data);
        write_wlan_security(WIFI_BAND_24G, data);
        sendToClient(client_fd, getPackageData(id, "res=1"));
    }
    if (strcmp(id, FORMAT_ID_APN) == 0) {
        log_d("set FORMAT_ID_APN: %s", data);
        char *set_profilename;
        modify_profile_settings  modify_profile;

        char *token;
        int col = 0;
        while (data != NULL) {
            token = strsep(&data, profile_info_delim_);
            if(col  == PROFILE_NAME){
                set_profilename = token;
            }
            if(col  == APN){
                modify_profile.apn_name = token;
            }
            if(col  == USER_NAME){
                modify_profile.username = token;
            }
            if(col  == PROFILE_PASSWORD){
                modify_profile.password = token;
            }
            if(col  == PDP_TYPE){
                modify_profile.pdp_type = token;
            }
            col++;
        }
        log_d("handleSetting_set_profilename: %s", set_profilename);
        log_d("handleSetting_modify_profile.apn_name: %s", modify_profile.apn_name);
        log_d("handleSetting_modify_profile.username: %s", modify_profile.username);
        log_d("handleSetting_modify_profile.password: %s", modify_profile.password);
        log_d("handleSetting_modify_profile.pdp_type: %s", modify_profile.pdp_type);
        //insert to modem
        modify_apn_profile(&modify_profile);

        PROFILE_CONTENT profile;
        profile.profile_name  = set_profilename;
        profile.apn    = modify_profile.apn_name;
        profile.user_name = modify_profile.username;
        profile.password = modify_profile.password;
        profile.pdp_type = modify_profile.pdp_type;
        log_d("handleSetting_ds_get_value(default_profile_name):%s", ds_get_value("default_profile_name"));
        //update profile if input profile is the same as ds_get_value("default_profile_name")
        if (strcmp(ds_get_value("default_profile_name"), set_profilename) == 0) {
            //update profile
            update_profile_name_node(ds_get_value("default_profile_name"), profile);
        } else {
            //insert to xml
            ds_set_value("default_profile_name", set_profilename);
            //insert new profile
            write_new_profile(profile);
        }
        sendToClient(client_fd, getPackageData(id, "res=1"));
    }
    if (strcmp(id, FORMAT_ID_AP_STA_CONNECT) == 0) {
        user_connected_update(USER_CONNECTED);
    }
    if (strcmp(id, FORMAT_ID_AP_STA_DISCONNECT) == 0) {
        user_connected_update(USER_DISCONNECTED);
    }
}

void sendInformation(const int client_fd, const char* id) {
    if (id != NULL && strcmp(id, "") != 0) {
        char* data = NULL;

        if (strcmp(id, FORMAT_ID_BRIGHTNESS) == 0) {
            int level = get_brightness();
            int l = sizeof(level) + 1 ;
            char brightness[l];
            memset(brightness, '\0', sizeof(brightness));
            sprintf(brightness, "%d", level);

            data = getPackageData(id, brightness);
        } else if (strcmp(id, FORMAT_ID_BATTERY) == 0) {
            int level = get_battery_info();
            int l = sizeof(level) + 1 ;
            char battery[l];
            memset(battery, '\0', sizeof(battery));
            sprintf(battery, "%d", level);

            data = getPackageData(id, battery);
        } else if (strcmp(id, FORMAT_ID_WLAN_MAC_ADDRESS) == 0) {
            data = getPackageData(id, getWlanMacAddress());
        } else if (strcmp(id, FORMAT_ID_IMEI) == 0) {
            data = getPackageData(id, getImei());
        } else if (strcmp(id, FORMAT_ID_BT_MAC_ADDRESS) == 0) {
            data = getPackageData(id, getBtnMacAddress());
        } else if (strcmp(id, FORMAT_ID_SW_VERSION) == 0) {
            data = getPackageData(id, getSwVersion());
        } else if (strcmp(id, FORMAT_ID_SSID) == 0) {
            data = getPackageData(id, get_wlan_ssid(WIFI_BAND_24G));
        } else if (strcmp(id, FORMAT_ID_PASSWORD) == 0) {
            data = getPackageData(id, get_wlan_password(WIFI_BAND_24G));
        } else if (strcmp(id, FORMAT_ID_SECURITY_TYPE) == 0) {
            data = getPackageData(id, get_wlan_security(WIFI_BAND_24G));
        }else if (strcmp(id, FORMAT_ID_DATA_USAGE) == 0) {
            int usage = (int)get_data_usage();
            int l = sizeof(usage) + 1 ;
            char data_usage[l];
            memset(data_usage, '\0', sizeof(data_usage));
            sprintf(data_usage, "%d", usage);

            data = getPackageData(id, data_usage);
        } else if (strcmp(id, FORMAT_ID_DATA_COUNT) == 0) {
            int count = get_connected_number();
            int l = sizeof(count) + 1 ;
            char data_count[l];
            memset(data_count, '\0', sizeof(data_count));
            sprintf(data_count, "%d", count);

            data = getPackageData(id, data_count);
        } else if (strcmp(id, FORMAT_ID_OPERATOR) == 0) {
            int l = OPERATOR_NAME_MAX_LENGTH + 1;
            char oper_name[l];
            memset(oper_name, 0, l);
            get_operator_name(oper_name, l);
            data = getPackageData(id, oper_name);
        } else if (strcmp(id, FORMAT_ID_SIGNAL) == 0) {
            ril_nw_signal_rat_t src = 0;
            int strength = get_strength_state(&src);
            int l = sizeof(strength) + 1 ;
            char signal[l];
            memset(signal, '\0', sizeof(signal));
            sprintf(signal, "%d", strength);

            data = getPackageData(id, signal);
        } else if (strcmp(id, FORMAT_ID_APN) == 0) {
            log_d("sendInformation_FORMAT_ID_APN\n");
            data = getPackageData(id, get_all_profile_data());
        }

        if (data != NULL) {
            sendToClient(client_fd, data);
            memset(data, '\0', sizeof(data));
        } else {
            sendToClient(client_fd, getPackageData(id, "res=0"));
        }
    }
}

int socket_server() {
    int server_fd = 0;
    int client_fd = 0;
    int flag_reuse = 1;

    struct timeval timeout = { 30, 0 }; //30s

    log_d("create server socket.");
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        log_e("Fail to create server socket.");
        return 0;
    }

    //socket?„é€??
    struct sockaddr_un serverInfo;
    struct sockaddr_un clientInfo;

    socklen_t addrlen = sizeof(clientInfo);
    bzero(&serverInfo, sizeof(serverInfo));

    serverInfo.sun_family = AF_UNIX;
    strncpy(serverInfo.sun_path, UI_SOCKET_PATH, sizeof(serverInfo.sun_path));
    unlink(UI_SOCKET_PATH);

    //Avoid error of "Address is in use"
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag_reuse, sizeof(flag_reuse)) < 0) {
        log_e("server.setsockopt error");
    }

    if (bind(server_fd, (struct sockaddr *) &serverInfo, sizeof(serverInfo)) < 0) {
        log_e("server.bind error errno %d %s", errno, strerror(errno));
        return 0;
    }

    if (listen(server_fd, 5) < 0) {
        log_e("server.listen error errno %d %s", errno, strerror(errno));
        return 0;
    }

    while (1) {
        bool log_b = true;
        static uint32_t timestamp;
        uint32_t t = lv_tick_elaps(timestamp);
        if (t != 0 && t > 30000) { // 30 sec interval between print of error log
            log_b = true;
            timestamp = lv_tick_get();
        } else {
            log_b = false;
        }

        if (log_b) log_e("waiting for accept");
        client_fd = accept(server_fd, (struct sockaddr*) &clientInfo, &addrlen);
        if (client_fd < 0) {
            if (log_b) log_e("server accept error errno %d %s", errno, strerror(errno));
            continue;
        }

        //Set socket recv timeout error
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            log_e("server.setsockopt error");
        }

        //receive event from client
        char inputBuffer[256];
        char* id = NULL;
        char* action = NULL;
        char* data = NULL;

        int result = recv(client_fd, inputBuffer, sizeof(inputBuffer), 0);
        if (result == 0) {
            log_e("client socket is closed.\n");
        } else if (result > 0) {
            log_e("Received data: %s", inputBuffer);
            //parse events
            char* buffer = strdup(inputBuffer);
            id = strdup(strsep(&buffer, ","));
            action = strdup(strsep(&buffer, ","));
            char* data_temp = strsep(&buffer, ",");
            if (data_temp != NULL) {
                data = strdup(data_temp);
            }
            free(buffer);
        } else {
            log_e("socket server error");
        }

        if (action != NULL) {
            if (strcmp(action, FORMAT_ACTION_INFOR) == 0) {
                sendInformation(client_fd, id);
                /*char* id = NULL;
                do {
                    id = strsep(&ids, " ");
                    sendInformation(client_fd, id);
                } while (id);*/
            } else if (strcmp(action, FORMAT_ACTION_SETTING) == 0) {
                handleSetting(client_fd, id, data);
            }
        }

        if (id != NULL) {
            free(id);;
        }

        if (action != NULL) {
            free(action);
        }

        if (data != NULL) {
            free(data);
        }

        memset(inputBuffer, 0, sizeof inputBuffer);
        close(client_fd);
    }
    return 0;
}
