/*
 * Example usage of the connectivity check functions
 * This file demonstrates how to use the checkAndReconnectToIP function
 * to monitor connectivity to a specific IP address and automatically reconnect if needed.
 */

#include "checkconnection.h"

// Example 1: Basic connectivity check to 192.168.88.1
void exampleBasicCheck() {
    Serial.println("=== Basic Connectivity Check ===");
    
    // Check connectivity to 192.168.88.1 with 10 second timeout
    bool isConnected = checkAndReconnectToIP("192.168.88.1", 10000);
    
    if (isConnected) {
        Serial.println("SUCCESS: Can connect to 192.168.88.1");
    } else {
        Serial.println("FAILED: Cannot connect to 192.168.88.1");
    }
}

// Example 2: Periodic connectivity monitoring
void examplePeriodicMonitoring() {
    static unsigned long lastCheck = 0;
    static const unsigned long checkInterval = 30000; // Check every 30 seconds
    
    unsigned long currentTime = millis();
    
    if (currentTime - lastCheck >= checkInterval) {
        Serial.println("=== Periodic Connectivity Check ===");
        
        bool isConnected = checkAndReconnectToIP("192.168.88.1", 5000);
        
        if (isConnected) {
            Serial.println("Periodic check: Connection OK");
        } else {
            Serial.println("Periodic check: Connection failed, reconnection attempted");
        }
        
        lastCheck = currentTime;
    }
}

// Example 3: Custom IP address monitoring
void exampleCustomIPCheck() {
    Serial.println("=== Custom IP Check ===");
    
    // Check connectivity to a different IP address
    bool isConnected = checkAndReconnectToIP("192.168.1.1", 8000);
    
    if (isConnected) {
        Serial.println("SUCCESS: Can connect to custom IP");
    } else {
        Serial.println("FAILED: Cannot connect to custom IP");
    }
}

// Example 4: Integration with existing WiFi status checks
void exampleWiFiIntegration() {
    Serial.println("=== WiFi Integration Example ===");
    
    // First check WiFi status
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting reconnection...");
        WiFi.reconnect();
        delay(2000);
    }
    
    // Then check specific IP connectivity
    if (WiFi.status() == WL_CONNECTED) {
        bool isConnected = checkAndReconnectToIP("192.168.88.1", 5000);
        
        if (isConnected) {
            Serial.println("WiFi and IP connectivity: OK");
        } else {
            Serial.println("WiFi OK but IP connectivity failed");
        }
    } else {
        Serial.println("WiFi connection failed");
    }
}

// Example 5: Error handling and retry logic
void exampleWithRetryLogic() {
    Serial.println("=== Retry Logic Example ===");
    
    const int maxRetries = 3;
    int retryCount = 0;
    bool connectionSuccess = false;
    
    while (retryCount < maxRetries && !connectionSuccess) {
        Serial.println("Attempt " + String(retryCount + 1) + " of " + String(maxRetries));
        
        connectionSuccess = checkAndReconnectToIP("192.168.88.1", 8000);
        
        if (!connectionSuccess) {
            retryCount++;
            if (retryCount < maxRetries) {
                Serial.println("Connection failed, waiting before retry...");
                delay(5000); // Wait 5 seconds before retry
            }
        }
    }
    
    if (connectionSuccess) {
        Serial.println("SUCCESS: Connection established after retries");
    } else {
        Serial.println("FAILED: All retry attempts exhausted");
    }
}

/*
 * Usage in main loop:
 * 
 * void loop() {
 *     // Check connectivity every 30 seconds
 *     examplePeriodicMonitoring();
 *     
 *     // Or trigger manual check based on events
 *     if (someEventOccurred) {
 *         exampleBasicCheck();
 *     }
 *     
 *     // Your other code here...
 *     delay(100);
 * }
 */ 