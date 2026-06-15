#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <math.h>
#include <time.h>

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"
#define SLEEP_MIN 30
#define uS_TO_S   1000000ULL

#define PIN_CS   15
#define PIN_DC   17
#define PIN_RST  16
#define PIN_BUSY  4

GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 2>
    display(GxEPD2_750c_Z08(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

struct Clima {
    float temp, temp_aparente, vento, onda;
    int   umidade, wmo;
    float prev_temp[5];
    int   prev_wmo[5];
};

// ── Texto ─────────────────────────────────────────

const char* descWMO(int c) {
    if (c == 0)  return "Ceu limpo";
    if (c <= 3)  return "Parcialmente nublado";
    if (c <= 48) return "Nevoeiro";
    if (c <= 67) return "Chuva";
    if (c <= 77) return "Neve";
    if (c <= 82) return "Pancadas";
    return               "Tempestade";
}

int moonPhase() {
    time_t now = time(nullptr);
    double days = (now - 947182440.0) / 86400.0;
    double p = fmod(days, 29.53059);
    return (int)(p < 0 ? p + 29.53059 : p);
}

const char* moonPhaseText(int p) {
    if (p < 2 || p > 27)   return "Lua Nova";
    if (p < 7)              return "Lua Crescente";
    if (p == 7)             return "Quarto Crescente";
    if (p < 13)             return "Gibosa Crescente";
    if (p <= 16)            return "Lua Cheia";
    if (p < 21)             return "Gibosa Minguante";
    if (p <= 22)            return "Quarto Minguante";
    return                         "Lua Minguante";
}

// ── Ícones ────────────────────────────────────────

void drawSun(int cx, int cy, int r, uint16_t col) {
    display.fillCircle(cx, cy, r, col);
    for (int i = 0; i < 8; i++) {
        float a = i * M_PI / 4.0f;
        int x1 = cx + (int)((r + 5)  * cosf(a));
        int y1 = cy + (int)((r + 5)  * sinf(a));
        int x2 = cx + (int)((r + 13) * cosf(a));
        int y2 = cy + (int)((r + 13) * sinf(a));
        display.drawLine(x1, y1, x2, y2, col);
        display.drawLine(x1+1, y1, x2+1, y2, col);
    }
}

void drawCloud(int cx, int cy, int r, uint16_t col) {
    display.fillCircle(cx,         cy,       r,        col);
    display.fillCircle(cx - r,     cy + r/2, r*3/4,    col);
    display.fillCircle(cx + r,     cy + r/4, r*4/5,    col);
    display.fillRect(cx - r*7/4,   cy + r/2, r*7/2, r/2+2, col);
}

void drawRain(int cx, int cy, int r, uint16_t col) {
    drawCloud(cx, cy, r, col);
    int dy = cy + r/2 + r*3/4 + 6;
    for (int i = 0; i < 4; i++) {
        int rx = cx - r + i * (r/2);
        display.drawLine(rx,   dy, rx - 5, dy + 10, col);
        display.drawLine(rx+1, dy, rx - 4, dy + 10, col);
    }
}

void drawStorm(int cx, int cy, int r, uint16_t col) {
    drawCloud(cx, cy, r, col);
    int lx = cx - r/5, ly = cy + r + 5;
    display.fillTriangle(lx,       ly,       lx + r/2, ly,       lx + r/6, ly + r/2, GxEPD_RED);
    display.fillTriangle(lx + r/6, ly + r/2, lx - r/5, ly + r/2, lx + r/2, ly + r,  GxEPD_RED);
}

void drawFog(int cx, int cy, int r, uint16_t col) {
    for (int i = 0; i < 5; i++) {
        int fy  = cy - r + i * (r/2);
        int len = r*2 - (i % 2 ? r/3 : 0);
        display.fillRoundRect(cx - len/2, fy, len, 4, 2, col);
    }
}

void drawMoon(int cx, int cy, int r) {
    display.fillCircle(cx, cy, r, GxEPD_BLACK);
    display.fillCircle(cx + r/2, cy - r/3, r*9/10, GxEPD_WHITE);
    display.fillCircle(cx + r*2,     cy - r,     2, GxEPD_BLACK);
    display.fillCircle(cx + r + r/2, cy - r*3/2, 2, GxEPD_BLACK);
}

void drawMoonPhase(int cx, int cy, int r, int p) {
    if (p < 2 || p > 27) {
        display.drawCircle(cx, cy, r,   GxEPD_BLACK);
        display.drawCircle(cx, cy, r-1, GxEPD_BLACK);
        return;
    }
    display.fillCircle(cx, cy, r, GxEPD_BLACK);
    if (p >= 13 && p <= 16) return;
    if      (p < 7)  display.fillCircle(cx + r/2,  cy, r*9/10, GxEPD_WHITE);
    else if (p < 13) display.fillCircle(cx - r/3,  cy, r*2/3,  GxEPD_WHITE);
    else if (p < 21) display.fillCircle(cx + r/3,  cy, r*2/3,  GxEPD_WHITE);
    else             display.fillCircle(cx - r/2,  cy, r*9/10, GxEPD_WHITE);
}

void drawWeatherIcon(int cx, int cy, int r, int wmo, bool night) {
    if (night && wmo == 0)  { drawMoon(cx, cy, r); return; }
    if (wmo == 0)           { drawSun(cx, cy, r, GxEPD_RED); return; }
    if (wmo <= 3) {
        drawSun(cx - r*2/3, cy - r*2/3, r*3/5, GxEPD_RED);
        drawCloud(cx + r/4, cy + r/5, r*3/4, GxEPD_BLACK);
        return;
    }
    if (wmo <= 48) { drawFog(cx, cy, r, GxEPD_BLACK);              return; }
    if (wmo <= 82) { drawRain(cx, cy, r*3/4, GxEPD_BLACK);         return; }
    drawStorm(cx, cy, r*3/4, GxEPD_BLACK);
}

// ── APIs ──────────────────────────────────────────

bool buscarClima(Clima &c) {
    Serial.println("[clima] buscando...");
    HTTPClient http;
    http.begin(
        "http://api.open-meteo.com/v1/forecast"
        "?latitude=-20.33&longitude=-40.29"
        "&current=temperature_2m,apparent_temperature,relative_humidity_2m,windspeed_10m,weathercode"
        "&daily=weathercode,temperature_2m_max"
        "&forecast_days=5&timezone=America/Sao_Paulo");
    http.setTimeout(15000);

    int code = http.GET();
    Serial.printf("[clima] HTTP %d\n", code);
    if (code != 200) {
        Serial.printf("[clima] ERRO: %s\n", http.errorToString(code).c_str());
        http.end(); return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) { Serial.printf("[clima] JSON: %s\n", err.c_str()); return false; }

    c.temp          = doc["current"]["temperature_2m"];
    c.temp_aparente = doc["current"]["apparent_temperature"];
    c.umidade       = doc["current"]["relative_humidity_2m"];
    c.vento         = doc["current"]["windspeed_10m"];
    c.wmo           = doc["current"]["weathercode"];
    for (int i = 0; i < 5; i++) {
        c.prev_temp[i] = doc["daily"]["temperature_2m_max"][i];
        c.prev_wmo[i]  = doc["daily"]["weathercode"][i];
    }
    Serial.printf("[clima] %.1fC wmo=%d\n", c.temp, c.wmo);
    return true;
}

void buscarOnda(Clima &c) {
    c.onda = -1;
    HTTPClient http;
    http.begin("http://marine-api.open-meteo.com/v1/marine"
               "?latitude=-20.33&longitude=-40.29&current=wave_height");
    http.setTimeout(15000);
    int code = http.GET();
    Serial.printf("[onda] HTTP %d\n", code);
    if (code != 200) { http.end(); return; }
    String payload = http.getString();
    http.end();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
        JsonVariant wh = doc["current"]["wave_height"];
        c.onda = wh.isNull() ? -1.0f : wh.as<float>();
    }
    Serial.printf("[onda] %.1fm\n", c.onda);
}

void syncNTP() {
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("[ntp] sincronizando");
    struct tm info;
    for (int i = 0; i < 20 && !getLocalTime(&info); i++) {
        delay(500); Serial.print(".");
    }
    if (getLocalTime(&info))
        Serial.printf(" ok %02d:%02d\n", info.tm_hour, info.tm_min);
    else
        Serial.println("\n[ntp] timeout");
}

// ── Tela ──────────────────────────────────────────

void desenharTela(const Clima &c) {
    struct tm tm;
    bool hasTime = getLocalTime(&tm);
    bool night   = hasTime && (tm.tm_hour >= 20 || tm.tm_hour < 6);

    const char* semana[] = {"Dom","Seg","Ter","Qua","Qui","Sex","Sab"};
    const char* meses[]  = {"Jan","Fev","Mar","Abr","Mai","Jun","Jul","Ago","Set","Out","Nov","Dez"};

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // ── Cabeçalho ──────────────────────────────
        display.setFont(&FreeMonoBold12pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(15, 33);
        display.print("Vila Velha, ES");

        if (hasTime) {
            char buf[40];
            snprintf(buf, sizeof(buf), "%s %02d/%s",
                     semana[tm.tm_wday], tm.tm_mday, meses[tm.tm_mon]);
            int16_t x1, y1; uint16_t w, h;
            display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
            display.setCursor(800 - (int)w - 15, 33);
            display.print(buf);
        }
        display.drawFastHLine(0, 43, 800, GxEPD_BLACK);

        // ── Ícone principal ─────────────────────────
        drawWeatherIcon(185, 153, 48, c.wmo, night);

        // ── Dados ───────────────────────────────────
        display.setFont(&FreeMonoBold18pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(395, 115);
        display.printf("%.1f C", c.temp);

        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(395, 152);
        display.print(descWMO(c.wmo));

        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(395, 183);
        display.printf("Sensacao:  %.1f C", c.temp_aparente);
        display.setCursor(395, 208);
        display.printf("Umidade:   %d %%", c.umidade);
        display.setCursor(395, 233);
        display.printf("Vento:     %.0f km/h", c.vento);
        if (c.onda >= 0) {
            display.setCursor(395, 258);
            display.printf("Ondas:     %.1f m", c.onda);
        }

        display.drawFastHLine(0, 273, 800, GxEPD_BLACK);

        // ── Previsão 5 dias ─────────────────────────
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_RED);
        display.setCursor(15, 296);
        display.print("Proximos 5 dias:");

        display.setTextColor(GxEPD_BLACK);
        for (int i = 0; i < 5; i++) {
            int col = 60 + i * 148;

            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(col - 14, 322);
            if (!hasTime) {
                const char* dias[] = {"Seg","Ter","Qua","Qui","Sex"};
                display.print(dias[i]);
            } else if (i == 0) {
                display.print("Hoje");
            } else {
                display.print(semana[(tm.tm_wday + i) % 7]);
            }

            drawWeatherIcon(col, 358, 20, c.prev_wmo[i], false);

            char tbuf[8];
            snprintf(tbuf, sizeof(tbuf), "%.0fC", c.prev_temp[i]);
            int16_t tx, ty; uint16_t tw, th;
            display.getTextBounds(tbuf, 0, 0, &tx, &ty, &tw, &th);
            display.setCursor(col - (int)tw/2, 405);
            display.print(tbuf);
        }

        display.drawFastHLine(0, 418, 800, GxEPD_BLACK);

        // ── Fase da lua ─────────────────────────────
        int phase = moonPhase();
        drawMoonPhase(28, 450, 18, phase);

        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(56, 456);
        display.print(moonPhaseText(phase));

        char rbuf[32];
        snprintf(rbuf, sizeof(rbuf), "Atualiza em %d min", SLEEP_MIN);
        int16_t rx, ry; uint16_t rw, rh;
        display.getTextBounds(rbuf, 0, 0, &rx, &ry, &rw, &rh);
        display.setCursor(800 - (int)rw - 15, 456);
        display.print(rbuf);

    } while (display.nextPage());
}

// ── Setup ─────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    display.init(115200);

    Serial.printf("[wifi] conectando a '%s'...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries++ < 20) {
        delay(500); Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[wifi] ERRO: nao conectou");
    } else {
        Serial.printf("[wifi] ok: %s\n", WiFi.localIP().toString().c_str());
        syncNTP();

        Clima c;
        if (buscarClima(c)) {
            buscarOnda(c);
            Serial.println("[display] desenhando...");
            desenharTela(c);
            Serial.println("[display] pronto");
        } else {
            Serial.println("[erro] falha ao buscar dados");
        }
    }

    esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_MIN * 60 * uS_TO_S);
    esp_deep_sleep_start();
}

void loop() {}
