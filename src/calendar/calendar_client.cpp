#include "calendar_client.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../config.h"

// Formata timestamp Unix como string ISO 8601 para a Calendar API
static String toISO8601(time_t t) {
    struct tm ti;
    gmtime_r(&t, &ti);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &ti);
    return String(buf);
}

std::vector<CalendarEvent> fetchCalendarEvents(const String& accessToken) {
    std::vector<CalendarEvent> events;

    time_t now    = time(nullptr);
    time_t future = now + (time_t)CALENDAR_FETCH_DAYS * 86400;

    String url = String(GOOGLE_CALENDAR_API_BASE)
               + "/calendars/primary/events"
               + "?timeMin=" + toISO8601(now)
               + "&timeMax=" + toISO8601(future)
               + "&maxResults=" + String(MAX_EVENTS)
               + "&singleEvents=true"
               + "&orderBy=startTime";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Authorization", "Bearer " + accessToken);

    int code = http.GET();
    if (code != 200) {
        Serial.printf("Falha ao buscar eventos: HTTP %d\n", code);
        http.end();
        return events;
    }

    // Filtrar apenas o campo "items" para economizar heap
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err) {
        Serial.println("Erro ao parsear JSON da Calendar API.");
        return events;
    }

    JsonArray items = doc["items"].as<JsonArray>();
    for (JsonObject item : items) {
        CalendarEvent ev;
        ev.title = item["summary"] | "(sem título)";

        // Evento de dia inteiro usa "date"; evento com hora usa "dateTime"
        if (item["start"]["date"].is<const char*>()) {
            ev.isAllDay      = true;
            ev.startDateTime = item["start"]["date"].as<String>();
            ev.endDateTime   = item["end"]["date"].as<String>();
        } else {
            ev.isAllDay      = false;
            ev.startDateTime = item["start"]["dateTime"].as<String>();
            ev.endDateTime   = item["end"]["dateTime"].as<String>();
        }

        events.push_back(ev);
    }

    Serial.printf("%d evento(s) obtido(s).\n", events.size());
    return events;
}
