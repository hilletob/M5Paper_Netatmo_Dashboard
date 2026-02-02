#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "../config.h"

class HTTPUtils {
public:
    // Make a GET request and parse JSON response
    static bool httpGetJSON(const char* url, JsonDocument& doc, const char* authToken = nullptr) {
        WiFiClientSecure client;
        client.setInsecure();  // Skip certificate validation for simplicity

        HTTPClient http;
        http.setReuse(false);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setUserAgent(HTTP_USER_AGENT);

        if (!http.begin(client, url)) {
            ESP_LOGE("http", "Failed to begin HTTP connection to: %s", url);
            return false;
        }

        // Add authorization header if provided
        if (authToken != nullptr) {
            String authHeader = "Bearer ";
            authHeader += authToken;
            http.addHeader("Authorization", authHeader);
        }

        ESP_LOGI("http", "GET %s", url);
        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_MOVED_PERMANENTLY) {
            ESP_LOGE("http", "GET failed, code: %d, error: %s",
                    httpCode, http.errorToString(httpCode).c_str());
            http.end();
            return false;
        }

        String payload = http.getString();
        http.end();

        // Parse JSON response
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            ESP_LOGE("http", "JSON parse failed: %s", error.c_str());
            ESP_LOGD("http", "Response: %s", payload.substring(0, 200).c_str());
            return false;
        }

        ESP_LOGI("http", "GET success, size: %d bytes", payload.length());
        return true;
    }

    // Make a POST request with form data and parse JSON response
    static bool httpPostForm(const char* url, const char* formData, JsonDocument& doc) {
        WiFiClientSecure client;
        client.setInsecure();  // Skip certificate validation

        HTTPClient http;
        http.setReuse(false);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setUserAgent(HTTP_USER_AGENT);

        if (!http.begin(client, url)) {
            ESP_LOGE("http", "Failed to begin HTTP connection to: %s", url);
            return false;
        }

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        ESP_LOGI("http", "POST %s", url);
        int httpCode = http.POST(formData);

        if (httpCode != HTTP_CODE_OK) {
            ESP_LOGE("http", "POST failed, code: %d, error: %s",
                    httpCode, http.errorToString(httpCode).c_str());
            http.end();
            return false;
        }

        String payload = http.getString();
        http.end();

        // Parse JSON response
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            ESP_LOGE("http", "JSON parse failed: %s", error.c_str());
            ESP_LOGD("http", "Response: %s", payload.substring(0, 200).c_str());
            return false;
        }

        ESP_LOGI("http", "POST success, size: %d bytes", payload.length());
        return true;
    }

    // Make a simple GET request and return the response as a string
    static bool httpGetString(const char* url, String& response) {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        http.setReuse(false);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setUserAgent(HTTP_USER_AGENT);

        if (!http.begin(client, url)) {
            ESP_LOGE("http", "Failed to begin HTTP connection to: %s", url);
            return false;
        }

        ESP_LOGI("http", "GET %s", url);
        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK) {
            ESP_LOGE("http", "GET failed, code: %d", httpCode);
            http.end();
            return false;
        }

        response = http.getString();
        http.end();

        ESP_LOGI("http", "GET success, size: %d bytes", response.length());
        return true;
    }

    // Retry wrapper for GET requests with exponential backoff
    static bool httpGetJSONWithRetry(const char* url, JsonDocument& doc,
                                    const char* authToken = nullptr,
                                    int maxRetries = 3) {
        for (int i = 0; i < maxRetries; i++) {
            if (httpGetJSON(url, doc, authToken)) {
                return true;
            }

            if (i < maxRetries - 1) {
                int delayMs = (1 << i) * 1000;  // 1s, 2s, 4s
                ESP_LOGW("http", "Retry %d/%d after %dms", i + 1, maxRetries, delayMs);
                delay(delayMs);
            }
        }

        ESP_LOGE("http", "All retries failed for: %s", url);
        return false;
    }

};

#endif  // HTTP_UTILS_H
