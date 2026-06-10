#pragma once

#include <Arduino.h>
#include <vector>

struct CalendarEvent {
    String title;
    String startDateTime;  // ISO 8601 (dateTime) ou "YYYY-MM-DD" (all-day)
    String endDateTime;
    bool   isAllDay;
};

// Busca eventos do calendário primário para os próximos CALENDAR_FETCH_DAYS dias.
// accessToken: Bearer token do Google OAuth.
// Retorna vetor de eventos ordenados por startDateTime.
std::vector<CalendarEvent> fetchCalendarEvents(const String& accessToken);
