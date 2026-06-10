#pragma once

#include "calendar_client.h"
#include <vector>

// Renderiza a lista de eventos no display e-ink.
// Usa firstPage()/nextPage() internamente. Chama display.hibernate() ao final.
void renderCalendar(const std::vector<CalendarEvent>& events);
