/*
 * device_information.h
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_ABOUT_DEVICE_INFORMATION_H_
#define LV_POCKET_ROUTER_SRC_ABOUT_DEVICE_INFORMATION_H_

#include "lv_pocket_router/src/util/util.h"

#define MAX_TITLE_LENGTH            18
#define MAX_DATA_LENGTH             32
#define MAX_INFO_LENGTH             (MAX_TITLE_LENGTH + MAX_DATA_LENGTH)

void readVersion(char* file, char* ver);
void show_device_information(void);
char* getPhoneNum();
char* getImei();
char* getWlanMacAddress();
char* getBtnMacAddress();
char* getModuleSwVersion();
char* getSwVersion();
char* getBtName();
char* getLanIpAddress();
char* getWanIpAddress();
void readSwVersion();
void readWlanInfo();
void readLanInfo();
#if defined (CUST_ZYXEL)
void readZyxelFwVersion();
char* getZyxelFwVersion();
#endif
char* getDLinkFwVersion();
#endif /* LV_POCKET_ROUTER_SRC_ABOUT_DEVICE_INFORMATION_H_ */
