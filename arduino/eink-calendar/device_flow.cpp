#include "device_flow.h"
#include "token_storage.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "qr_renderer.h"

bool startDeviceFlow() {
    WiFiClientSecure client;
    client.setInsecure();

    // --- Passo 1: Solicitar device_code ---
    HTTPClient http;
    http.begin(client, GOOGLE_DEVICE_CODE_URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String reqBody = "client_id=" + String(GOOGLE_CLIENT_ID)
                   + "&scope=" + String(GOOGLE_SCOPE);

    int code = http.POST(reqBody);
    if (code != 200) {
        Serial.printf("Falha ao obter device_code: HTTP %d\n", code);
        http.end();
        return false;
    }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    String deviceCode      = doc["device_code"].as<String>();
    String userCode        = doc["user_code"].as<String>();
    String verificationUrl = doc["verification_url"].as<String>();
    int    interval        = doc["interval"] | 5;
    int    expiresIn       = doc["expires_in"] | 1800;

    Serial.println("URL: " + verificationUrl);
    Serial.println("Código: " + userCode);

    // --- Passo 2: Exibir QR Code no display ---
    renderQRCode(verificationUrl, userCode);

    // --- Passo 3: Polling ---
    unsigned long startMs  = millis();
    unsigned long expireMs = startMs + (unsigned long)expiresIn * 1000;

    while (millis() < expireMs) {
        delay((unsigned long)interval * 1000);

        HTTPClient pollHttp;
        pollHttp.begin(client, GOOGLE_TOKEN_URL);
        pollHttp.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String pollBody = "client_id=" + String(GOOGLE_CLIENT_ID)
                        + "&client_secret=" + String(GOOGLE_CLIENT_SECRET)
                        + "&device_code=" + deviceCode
                        + "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

        int pollCode = pollHttp.POST(pollBody);
        String resp  = pollHttp.getString();
        pollHttp.end();

        JsonDocument pollDoc;
        deserializeJson(pollDoc, resp);

        if (pollDoc["access_token"].is<const char*>()) {
            String accessToken  = pollDoc["access_token"].as<String>();
            String refreshToken = pollDoc["refresh_token"].as<String>();
            uint32_t expiresAt  = pollDoc["expires_in"] | 3600;
            saveTokens(accessToken, refreshToken, expiresAt);
            Serial.println("Autenticação concluída!");
            return true;
        }

        const char* error = pollDoc["error"] | "";
        if (strcmp(error, "slow_down") == 0) {
            interval += 5;
            Serial.println("slow_down: aumentando intervalo para " + String(interval) + "s");
        } else if (strcmp(error, "access_denied") == 0) {
            Serial.println("Acesso negado pelo usuário.");
            return false;
        } else if (strcmp(error, "expired_token") == 0) {
            Serial.println("device_code expirou.");
            return false;
        }
        // authorization_pending: continuar polling
    }

    Serial.println("Timeout do Device Flow.");
    return false;
}
