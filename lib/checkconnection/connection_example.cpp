#include "connection_manager.h"
#include "Configfile.h"

// Example usage of ConnectionManager
// This file demonstrates how to use the connection status and reconnect functions

/*
Example usage in main.cpp:

#include "connection_manager.h"

// Global variables
Configfile cfg("/config.cfg");
ConnectionManager* connectionManager = nullptr;

void setup() {
    Serial.begin(9600);
    
    // Initialize config
    cfg.loadConfig();
    
    // Create connection manager
    connectionManager = new ConnectionManager(&cfg);
    
    // Configure connection manager
    connectionManager->setReconnectInterval(30000); // 30 seconds
    connectionManager->setMaxReconnectAttempts(5);
    connectionManager->enableAutoReconnect(true);
    
    // Initial connection
    if (connectionManager->connect()) {
        Serial.println("Initial connection successful");
    } else {
        Serial.println("Initial connection failed");
    }
}

void loop() {
    // Update connection manager (call this regularly)
    connectionManager->update();
    
    // Check connection status
    if (connectionManager->isDisconnected()) {
        Serial.println("WiFi disconnected!");
        
        // Manual reconnect if needed
        if (connectionManager->reconnect()) {
            Serial.println("Manual reconnect successful");
        }
    }
    
    // Get connection information
    ConnectionInfo info = connectionManager->getConnectionInfo();
    Serial.println("Status: " + connectionManager->getStatusString());
    Serial.println("IP: " + info.localIP);
    Serial.println("RSSI: " + String(info.rssi));
    Serial.println("Failed attempts: " + String(info.failedAttempts));
    
    // Ping server to check connectivity
    if (connectionManager->pingServer()) {
        Serial.println("Server ping successful");
    } else {
        Serial.println("Server ping failed");
    }
    
    delay(5000); // Check every 5 seconds
}

// Enhanced checkconnectiontask function using ConnectionManager
void enhancedCheckConnectionTask() {
    static unsigned long lastCheck = 0;
    unsigned long currentTime = millis();
    
    // Check every 10 seconds
    if (currentTime - lastCheck > 10000) {
        lastCheck = currentTime;
        
        // Update connection status
        connectionManager->update();
        
        // Check if we need to ping server
        if (connectionManager->isConnected()) {
            if (!connectionManager->pingServer()) {
                Serial.println("Server ping failed, connection may be unstable");
                
                // Optionally restart if configured
                if (configdata.havetorestart) {
                    Serial.println("Restarting due to connection issues...");
                    ESP.restart();
                }
            }
        } else {
            Serial.println("WiFi not connected, attempting reconnect...");
            connectionManager->reconnect();
        }
    }
}

// Function to display connection status on OLED
void displayConnectionStatus() {
    if (oledok) {
        ConnectionInfo info = connectionManager->getConnectionInfo();
        
        displayslot.head = "WiFi Status";
        displayslot.description = connectionManager->getStatusString();
        displayslot.description1 = info.localIP;
        displayslot.foot = "RSSI: " + String(info.rssi) + "dBm";
        displayslot.foot2 = "Attempts: " + String(info.failedAttempts);
        
        dd(); // Display function
    }
}

// Function to get connection status as JSON
String getConnectionStatusJSON() {
    ConnectionInfo info = connectionManager->getConnectionInfo();
    
    String json = "{";
    json += "\"status\":\"" + connectionManager->getStatusString() + "\",";
    json += "\"ssid\":\"" + info.ssid + "\",";
    json += "\"ip\":\"" + info.localIP + "\",";
    json += "\"mac\":\"" + info.macAddress + "\",";
    json += "\"rssi\":" + String(info.rssi) + ",";
    json += "\"failed_attempts\":" + String(info.failedAttempts) + ",";
    json += "\"connection_duration\":" + String(info.connectionDuration) + ",";
    json += "\"auto_reconnect\":" + String(info.autoReconnect ? "true" : "false");
    json += "}";
    
    return json;
}
*/ 