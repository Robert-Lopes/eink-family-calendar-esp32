#pragma once

#include <Arduino.h>

enum AuthStatus {
    AUTH_OK,
    AUTH_NEEDS_REAUTH
};

// Salva access_token, refresh_token e expires_at na NVS.
void saveTokens(const String& accessToken, const String& refreshToken, uint32_t expiresIn);

// Lê tokens da NVS para as variáveis passadas por referência.
void loadTokens(String& accessToken, String& refreshToken, uint32_t& expiresAt);

// Verifica validade e renova se necessário; retorna AUTH_OK ou AUTH_NEEDS_REAUTH.
AuthStatus checkAuth(String& accessToken);

// Usa o refresh_token para obter um novo access_token. Retorna true em caso de sucesso.
bool refreshAccessToken(String& accessToken);

// Limpa todos os tokens da NVS (logout/reset).
void clearTokens();
