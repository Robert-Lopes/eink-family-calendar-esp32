#include "wifi_connect.h"
#include <WiFi.h>
#include <esp_sleep.h>
#include "../config.h"
#include "../display/display_driver.h"

void connectWiFi() {
    Serial.print("Conectando ao WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFalha ao conectar ao WiFi");

        display.setFullWindow();
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMonoBold12pt7b);
            display.setCursor(40, 200);
            display.print("WiFi nao conectado.");
            display.setCursor(40, 240);
            display.print("Tentando novamente em 5 min.");
        } while (display.nextPage());
        display.hibernate();

        // Tenta novamente em 5 minutos
        esp_deep_sleep(5ULL * 60 * 1000000);
    }

    Serial.println("\nWiFi conectado: " + WiFi.localIP().toString());
}
