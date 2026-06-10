#include <Arduino.h>
#include <SPI.h>
#include <time.h>
#include <esp_sleep.h>

#include "config.h"
#include "display_driver.h"
#include "wifi_connect.h"
#include "token_storage.h"
#include "device_flow.h"
#include "calendar_client.h"
#include "calendar_renderer.h"

static void syncNTP() {
    configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    Serial.print("Sincronizando NTP");
    struct tm ti;
    int attempts = 0;
    while (!getLocalTime(&ti) && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println(getLocalTime(&ti) ? " OK" : " FALHOU");
}

void setup() {
    Serial.begin(115200);

    display.init(115200);
    display.setRotation(0);

    connectWiFi();
    syncNTP();

    String accessToken;
    AuthStatus status = checkAuth(accessToken);

    if (status == AUTH_NEEDS_REAUTH) {
        bool ok = startDeviceFlow();
        if (!ok) {
            // Exibe erro e dorme para tentar novamente
            display.setFullWindow();
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                display.setTextColor(GxEPD_BLACK);
                display.setFont(&FreeMonoBold12pt7b);
                display.setCursor(40, 200);
                display.print("Falha na autenticacao.");
                display.setCursor(40, 240);
                display.print("Reiniciando em 5 min...");
            } while (display.nextPage());
            display.hibernate();
            esp_deep_sleep(5ULL * 60 * 1000000);
        }
        // Após Device Flow bem-sucedido, recarregar token
        checkAuth(accessToken);
    }

    auto events = fetchCalendarEvents(accessToken);
    renderCalendar(events);

    esp_deep_sleep((uint64_t)DEEP_SLEEP_MINUTES * 60 * 1000000ULL);
}

void loop() {
    // Nunca executado — ESP32 reinicia do setup() após deep sleep
}
