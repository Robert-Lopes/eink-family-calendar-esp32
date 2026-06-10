#include "qr_renderer.h"
#include "display_driver.h"
#include <qrcode.h>

// QR Code posicionado à esquerda; texto de instrução e user_code à direita
static const int QR_X      = 40;
static const int QR_Y      = 80;
static const int QR_PIXELS = 5;   // cada módulo = 5px → QR ~200px para versão 4

void renderQRCode(const String& verificationUrl, const String& userCode) {
    QRCode qrcode;
    uint8_t qrData[qrcode_getBufferSize(4)];
    qrcode_initText(&qrcode, qrData, 4, ECC_LOW, verificationUrl.c_str());

    int qrSize = qrcode.size * QR_PIXELS;

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Instrução superior
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(QR_X, 50);
        display.print("Escaneie com seu celular:");

        // QR Code em preto
        for (int y = 0; y < qrcode.size; y++) {
            for (int x = 0; x < qrcode.size; x++) {
                if (qrcode_getModule(&qrcode, x, y)) {
                    display.fillRect(
                        QR_X + x * QR_PIXELS,
                        QR_Y + y * QR_PIXELS,
                        QR_PIXELS, QR_PIXELS,
                        GxEPD_BLACK
                    );
                }
            }
        }

        // URL em texto abaixo do QR
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(QR_X, QR_Y + qrSize + 25);
        display.print("google.com/device");

        // User code em vermelho à direita do QR
        int rightX = QR_X + qrSize + 40;
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(rightX, 160);
        display.print("Digite o codigo:");

        display.setTextColor(GxEPD_RED);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(rightX, 220);
        display.print(userCode);

        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(rightX, 270);
        display.print("em google.com/device");

    } while (display.nextPage());
    display.hibernate();

    Serial.println("QR Code exibido no display.");
}
