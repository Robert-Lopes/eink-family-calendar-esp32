#pragma once

// Executa o OAuth 2.0 Device Authorization Flow completo:
// exibe QR Code no display, faz polling até autenticação ou timeout,
// e salva os tokens na NVS via token_storage.
// Retorna true se autenticação bem-sucedida, false caso contrário.
bool startDeviceFlow();
