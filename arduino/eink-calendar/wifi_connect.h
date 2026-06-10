#pragma once

// Conecta ao WiFi usando WIFI_SSID/WIFI_PASSWORD de config.h.
// Em caso de falha após retries, exibe erro no display e entra em deep sleep.
void connectWiFi();
