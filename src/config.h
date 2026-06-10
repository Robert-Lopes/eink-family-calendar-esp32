#ifndef CONFIG_H
#define CONFIG_H

// ===== WiFi =====
#define WIFI_SSID     "A05 De Robert"
#define WIFI_PASSWORD "naoseiasenha"

// ===== Pinagem do Display E-Ink =====
#define PIN_CS   15
#define PIN_DC   17
#define PIN_RST  16
#define PIN_BUSY  4

// ===== Google OAuth 2.0 =====
#define GOOGLE_CLIENT_ID     ""
#define GOOGLE_CLIENT_SECRET ""
#define GOOGLE_SCOPE         "https://www.googleapis.com/auth/calendar.readonly"

// ===== Endpoints OAuth =====
#define GOOGLE_DEVICE_CODE_URL "https://oauth2.googleapis.com/device/code"
#define GOOGLE_TOKEN_URL       "https://oauth2.googleapis.com/token"

// ===== Google Calendar API =====
#define GOOGLE_CALENDAR_API_BASE "https://www.googleapis.com/calendar/v3"

// ===== Configurações do dispositivo =====
#define CALENDAR_FETCH_DAYS  14
#define MAX_EVENTS            9
#define DEEP_SLEEP_MINUTES   60
#define NVS_NAMESPACE        "oauth"

// ===== NTP =====
#define NTP_SERVER              "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC      (-3 * 3600)   // Brasília (UTC-3)
#define NTP_DAYLIGHT_OFFSET_SEC 0

#endif
