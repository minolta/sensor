#include "connection_manager.h"

ConnectionManager::ConnectionManager(Configfile* cfg) {
    config = cfg;
    reconnectInterval = 30000; // 30 seconds default
    maxReconnectAttempts = 5;
    isReconnecting = false;
    lastStatusCheck = 0;
    
    // Initialize connection info
    resetConnectionInfo();
    connInfo.autoReconnect = true;
}

void ConnectionManager::resetConnectionInfo() {
    connInfo.ssid = "";
    connInfo.password = "";
    connInfo.localIP = "";
    connInfo.macAddress = "";
    connInfo.rssi = 0;
    connInfo.status = DISCONNECTED;
    connInfo.lastConnectionAttempt = 0;
    connInfo.connectionDuration = 0;
    connInfo.failedAttempts = 0;
}

void ConnectionManager::updateConnectionInfo() {
    if (WiFi.status() == WL_CONNECTED) {
        connInfo.localIP = WiFi.localIP().toString();
        connInfo.macAddress = WiFi.macAddress();
        connInfo.rssi = WiFi.RSSI();
        connInfo.ssid = WiFi.SSID();
        connInfo.status = CONNECTED;
        connInfo.connectionDuration = millis() - connInfo.lastConnectionAttempt;
    } else {
        connInfo.status = DISCONNECTED;
        connInfo.localIP = "";
        connInfo.rssi = 0;
    }
}

ConnectionStatus ConnectionManager::getConnectionStatus() {
    updateConnectionInfo();
    return connInfo.status;
}

bool ConnectionManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool ConnectionManager::isDisconnected() {
    return WiFi.status() != WL_CONNECTED;
}

String ConnectionManager::getStatusString() {
    switch (getConnectionStatus()) {
        case CONNECTED:
            return "Connected";
        case DISCONNECTED:
            return "Disconnected";
        case CONNECTING:
            return "Connecting";
        case CONNECTION_FAILED:
            return "Connection Failed";
        case AP_MODE:
            return "AP Mode";
        default:
            return "Unknown";
    }
}

ConnectionInfo ConnectionManager::getConnectionInfo() {
    updateConnectionInfo();
    return connInfo;
}

bool ConnectionManager::attemptConnection() {
    String ssid = config->getConfig("ssid", "forpi");
    String password = config->getConfig("password", "04qwerty");
    
    if (ssid.length() == 0) {
        Serial.println("No SSID configured");
        return false;
    }
    
    Serial.println("Attempting to connect to WiFi...");
    Serial.println("SSID: " + ssid);
    
    WiFi.begin(ssid.c_str(), password.c_str());
    connInfo.lastConnectionAttempt = millis();
    connInfo.status = CONNECTING;
    
    int attempts = 0;
    int maxAttempts = config->getConfig("maxconnecttimeout", "10").toInt();
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected successfully!");
        updateConnectionInfo();
        connInfo.failedAttempts = 0;
        return true;
    } else {
        Serial.println("\nWiFi connection failed!");
        connInfo.status = CONNECTION_FAILED;
        connInfo.failedAttempts++;
        return false;
    }
}

bool ConnectionManager::connect() {
    if (isConnected()) {
        Serial.println("Already connected to WiFi");
        return true;
    }
    
    return attemptConnection();
}

bool ConnectionManager::reconnect() {
    if (isReconnecting) {
        return false; // Already attempting to reconnect
    }
    
    isReconnecting = true;
    Serial.println("Attempting to reconnect to WiFi...");
    
    WiFi.disconnect();
    delay(1000);
    
    bool success = attemptConnection();
    isReconnecting = false;
    
    return success;
}

void ConnectionManager::disconnect() {
    WiFi.disconnect();
    connInfo.status = DISCONNECTED;
    Serial.println("WiFi disconnected");
}

void ConnectionManager::enableAutoReconnect(bool enable) {
    connInfo.autoReconnect = enable;
    WiFi.setAutoReconnect(enable);
    Serial.println("Auto reconnect: " + String(enable ? "enabled" : "disabled"));
}

void ConnectionManager::setReconnectInterval(unsigned long interval) {
    reconnectInterval = interval;
}

void ConnectionManager::setMaxReconnectAttempts(int attempts) {
    maxReconnectAttempts = attempts;
}

void ConnectionManager::update() {
    unsigned long currentTime = millis();
    
    // Update connection info periodically
    if (currentTime - lastStatusCheck > 5000) { // Check every 5 seconds
        updateConnectionInfo();
        lastStatusCheck = currentTime;
    }
    
    // Auto reconnect logic
    if (connInfo.autoReconnect && isDisconnected() && !isReconnecting) {
        if (currentTime - connInfo.lastConnectionAttempt > reconnectInterval) {
            if (connInfo.failedAttempts < maxReconnectAttempts) {
                reconnect();
            } else {
                Serial.println("Max reconnection attempts reached. Stopping auto-reconnect.");
                connInfo.autoReconnect = false;
            }
        }
    }
}

void ConnectionManager::checkConnection() {
    if (isDisconnected()) {
        Serial.println("WiFi connection lost!");
        connInfo.status = DISCONNECTED;
        
        if (connInfo.autoReconnect && connInfo.failedAttempts < maxReconnectAttempts) {
            Serial.println("Attempting to reconnect...");
            reconnect();
        }
    } else {
        // Connection is good, reset failed attempts counter
        if (connInfo.failedAttempts > 0) {
            connInfo.failedAttempts = 0;
        }
    }
}

bool ConnectionManager::pingServer(String serverUrl) {
    if (!isConnected()) {
        Serial.println("Cannot ping server: not connected to WiFi");
        return false;
    }
    
    if (serverUrl.length() == 0) {
        serverUrl = config->getConfig("talkurl", "http://192.168.88.21:3334/hello");
    }
    
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("Pinging server: " + serverUrl);
    
    http.begin(client, serverUrl);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Response: " + response);
        http.end();
        return httpResponseCode == 200;
    } else {
        Serial.println("Error on HTTP request: " + String(httpResponseCode));
        http.end();
        return false;
    }
}

String ConnectionManager::getLocalIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

String ConnectionManager::getMacAddress() {
    return WiFi.macAddress();
}

int ConnectionManager::getRSSI() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

unsigned long ConnectionManager::getConnectionDuration() {
    if (isConnected()) {
        return millis() - connInfo.lastConnectionAttempt;
    }
    return 0;
}

int ConnectionManager::getFailedAttempts() {
    return connInfo.failedAttempts;
} 