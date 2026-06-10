#pragma once

#include <Arduino.h>

// Renderiza o QR Code da verificationUrl e o userCode no display e-ink.
// Usa firstPage()/nextPage() internamente. Chama display.hibernate() ao final.
void renderQRCode(const String& verificationUrl, const String& userCode);
