#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_PREFERENCE_NETWORK_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_PREFERENCE_NETWORK_H_

#define MAX_PN_TITLE_LEN   30

enum {
    ID_3G_ONLY   =2,
    ID_3G_4G     =5,
    ID_4G_ONLY   =6,
    ID_5G_ONLY   =7,
    ID_4G_5G     =8,
    ID_3G_4G_5G  =9,
};
void get_default_pref_network(char* title);
void pref_network_create(void) ;
void init_pref_network();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_PREFERENCE_NETWORK_H_ */
