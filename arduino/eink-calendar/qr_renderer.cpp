#include "qr_renderer.h"
#include "display_driver.h"

void renderQRCode(const String& verificationUrl, const String& userCode) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Título
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 60);
        display.print("Autenticacao necessaria");

        display.fillRect(20, 70, 760, 3, GxEPD_BLACK);

        // Passo 1
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(20, 130);
        display.print("1. Acesse no seu celular:");

        display.setTextColor(GxEPD_RED);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 185);
        display.print("google.com/device");

        // Passo 2
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(20, 260);
        display.print("2. Digite o codigo:");

        display.setTextColor(GxEPD_RED);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 315);
        display.print(userCode);

        // Rodapé
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(20, 440);
        display.print("Aguardando autorizacao...");

    } while (display.nextPage());
    display.hibernate();

    Serial.println("Tela de autenticacao exibida.");
    Serial.println("URL: " + verificationUrl);
    Serial.println("Codigo: " + userCode);
}
