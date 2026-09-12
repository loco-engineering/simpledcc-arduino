#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"

// Initialize Wi-Fi in Station mode, fallback to AP mode if no credentials found
void wifi_init_sta(void);

// Expose the global AP mode flag so others (e.g. web server) know the state
extern bool wifi_ap_mode_active;

#endif // WIFI_MANAGER_H
