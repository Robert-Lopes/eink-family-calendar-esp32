#include "token_storage.h"
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../config.h"

static Preferences prefs;

void saveTokens(const String& accessToken, const String& refreshToken, uint32_t expiresIn) {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("access_token", accessToken);
    if (refreshToken.length() > 0) {
        prefs.putString("refresh_token", refreshToken);
    }
    uint32_t expiresAt = (uint32_t)time(nullptr) + expiresIn - 60; // margem de 60s
    prefs.putUInt("expires_at", expiresAt);
    prefs.end();
    Serial.println("Tokens salvos na NVS.");
}

void loadTokens(String& accessToken, String& refreshToken, uint32_t& expiresAt) {
    prefs.begin(NVS_NAMESPACE, true);
    accessToken  = prefs.getString("access_token", "");
    refreshToken = prefs.getString("refresh_token", "");
    expiresAt    = prefs.getUInt("expires_at", 0);
    prefs.end();
}

bool refreshAccessToken(String& accessToken) {
    String refreshToken;
    uint32_t dummy;
    loadTokens(accessToken, refreshToken, dummy);

    if (refreshToken.isEmpty()) {
        Serial.println("Sem refresh_token — re-autenticação necessária.");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, GOOGLE_TOKEN_URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body = "client_id=" + String(GOOGLE_CLIENT_ID)
                + "&client_secret=" + String(GOOGLE_CLIENT_SECRET)
                + "&refresh_token=" + refreshToken
                + "&grant_type=refresh_token";

    int code = http.POST(body);
    if (code != 200) {
        Serial.printf("Falha ao renovar token: HTTP %d\n", code);
        http.end();
        return false;
    }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    if (!doc["access_token"].is<const char*>()) {
        Serial.println("Resposta de refresh inválida.");
        return false;
    }

    accessToken = doc["access_token"].as<String>();
    uint32_t expiresIn = doc["expires_in"] | 3600;
    saveTokens(accessToken, "", expiresIn);
    Serial.println("Access token renovado.");
    return true;
}

AuthStatus checkAuth(String& accessToken) {
    String refreshToken;
    uint32_t expiresAt;
    loadTokens(accessToken, refreshToken, expiresAt);

    if (refreshToken.isEmpty()) {
        Serial.println("Sem tokens — Device Flow necessário.");
        return AUTH_NEEDS_REAUTH;
    }

    uint32_t now = (uint32_t)time(nullptr);
    if (now >= expiresAt) {
        Serial.println("Access token expirado, renovando...");
        if (!refreshAccessToken(accessToken)) {
            return AUTH_NEEDS_REAUTH;
        }
    }

    return AUTH_OK;
}

void clearTokens() {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.clear();
    prefs.end();
    Serial.println("Tokens removidos da NVS.");
}
