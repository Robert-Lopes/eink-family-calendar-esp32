#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#define PIN_CS    15
#define PIN_DC    17
#define PIN_RST   16
#define PIN_BUSY   4

GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 2> display(
  GxEPD2_750c_Z08(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)
);

void setup() {
  Serial.begin(115200);
  display.init(115200);
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // ——— Mensagem principal em PRETO ———
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(80, 200);
    display.print("DEU CERTOOOOO!!");

    // ——— Sublinhado ———
    display.fillRect(80, 215, 640, 4, GxEPD_BLACK);

    // ——— Mensagem secundária em VERMELHO ———
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(180, 290);
    display.print("ESP32 + e-Paper funcionando!");

    // ——— Detalhes menores em preto ———
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(260, 350);
    display.print("Waveshare 7.5\" (B) V3");

  } while (display.nextPage());

  display.hibernate();
  Serial.println("Mensagem exibida!");
}

void loop() {
  delay(10000);
}