#include "calendar_renderer.h"
#include "display_driver.h"
#include <time.h>

// Extrai "HH:MM" de uma string ISO 8601 (ex: "2026-06-10T09:00:00-03:00")
static String extractTime(const String& dt) {
    int tIdx = dt.indexOf('T');
    if (tIdx < 0) return "";
    return dt.substring(tIdx + 1, tIdx + 6);
}

// Extrai "DD/MM" de uma string ISO 8601 ou "YYYY-MM-DD"
static String extractDate(const String& dt) {
    // dt pode ser "YYYY-MM-DD" ou "YYYY-MM-DDTHH:..."
    if (dt.length() < 10) return dt;
    String day   = dt.substring(8, 10);
    String month = dt.substring(5, 7);
    return day + "/" + month;
}

void renderCalendar(const std::vector<CalendarEvent>& events) {
    // Cabeçalho: data de hoje
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    char header[32];
    strftime(header, sizeof(header), "%d/%m/%Y", &ti);

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Cabeçalho em vermelho
        display.setTextColor(GxEPD_RED);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 45);
        display.print("Calendario Familiar");

        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(20, 75);
        display.print(header);

        // Linha separadora
        display.fillRect(20, 85, 760, 2, GxEPD_BLACK);

        if (events.empty()) {
            display.setFont(&FreeMonoBold12pt7b);
            display.setCursor(20, 130);
            display.print("Nenhum evento nos proximos dias.");
        } else {
            int y = 120;
            for (const auto& ev : events) {
                if (y > 450) break;

                // Data em vermelho
                display.setTextColor(GxEPD_RED);
                display.setFont(&FreeMonoBold9pt7b);
                display.setCursor(20, y);
                display.print(extractDate(ev.startDateTime));

                // Hora (se não for evento de dia inteiro)
                display.setTextColor(GxEPD_BLACK);
                if (!ev.isAllDay) {
                    display.setCursor(80, y);
                    display.print(extractTime(ev.startDateTime));
                } else {
                    display.setCursor(80, y);
                    display.print("dia todo");
                }

                // Título do evento
                display.setFont(&FreeMonoBold12pt7b);
                display.setCursor(180, y);
                // Truncar título se muito longo
                String title = ev.title;
                if (title.length() > 30) title = title.substring(0, 28) + "..";
                display.print(title);

                y += 38;
            }
        }

    } while (display.nextPage());
    display.hibernate();

    Serial.println("Calendário renderizado no display.");
}
