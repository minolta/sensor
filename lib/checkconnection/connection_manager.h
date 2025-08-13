#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Configfile.h"

// Connection status enumeration
enum ConnectionStatus {
    CONNECTED,
    DISCONNECTED,
    CONNECTING,
    CONNECTION_FAILED,
    AP_MODE
};

// Connection information structure
struct ConnectionInfo {
    String ssid;
    String password;
    String localIP;
    String macAddress;
    int rssi;
    ConnectionStatus status;
    unsigned long lastConnectionAttempt;
    unsigned long connectionDuration;
    int failedAttempts;
    bool autoReconnect;
};

class ConnectionManager {
private:
    Configfile* config;
    ConnectionInfo connInfo;
    unsigned long lastStatusCheck;
    unsigned long reconnectInterval;
    int maxReconnectAttempts;
    bool isReconnecting;
    
    // Private helper methods
    void updateConnectionInfo();
    bool attemptConnection();
    void resetConnectionInfo();
    
public:
    ConnectionManager(Configfile* cfg);
    
    // Connection status functions
    ConnectionStatus getConnectionStatus();
    bool isConnected();
    bool isDisconnected();
    String getStatusString();
    ConnectionInfo getConnectionInfo();
    
    // Connection management functions
    bool connect();
    bool reconnect();
    void disconnect();
    void enableAutoReconnect(bool enable);
    
    // Configuration functions
    void setReconnectInterval(unsigned long interval);
    void setMaxReconnectAttempts(int attempts);
    
    // Monitoring functions
    void update();
    void checkConnection();
    bool pingServer(String serverUrl = "");
    
    // Utility functions
    String getLocalIP();
    String getMacAddress();
    int getRSSI();
    unsigned long getConnectionDuration();
    int getFailedAttempts();
};

#endif // CONNECTION_MANAGER_H 