# Connectivity Check Library

This library provides functions to check connectivity to specific IP addresses and automatically reconnect WiFi if needed.

## Features

- **Automatic WiFi reconnection**: Automatically detects WiFi disconnection and attempts to reconnect
- **IP connectivity testing**: Tests connectivity to specific IP addresses using HTTP requests
- **Configurable timeouts**: Adjustable timeout values for connection attempts
- **Smart reconnection logic**: Disconnects and reconnects WiFi if IP connectivity fails
- **Integration ready**: Easy to integrate with existing WiFi management code

## Functions

### `checkAndReconnectToIP(String targetIP, int timeout)`

Checks connectivity to a specific IP address and automatically reconnects WiFi if needed.

**Parameters:**
- `targetIP`: The IP address to test connectivity to (default: "192.168.88.1")
- `timeout`: Timeout in milliseconds for connection attempts (default: 5000)

**Returns:**
- `true`: Successfully connected to the target IP
- `false`: Failed to connect to the target IP even after reconnection attempts

**Behavior:**
1. Checks if WiFi is connected
2. If WiFi is disconnected, attempts to reconnect
3. Tests connectivity to the target IP using HTTP GET request
4. If IP connectivity fails, disconnects and reconnects WiFi
5. Retests IP connectivity after reconnection
6. Returns success/failure status

### `checkConnectivityToGateway()`

Convenience function to check connectivity to the default gateway (192.168.88.1).

**Returns:**
- `true`: Successfully connected to 192.168.88.1
- `false`: Failed to connect to 192.168.88.1

### `manualConnectionCheck()`

Manually triggers a connection check and provides detailed feedback.

## Usage Examples

### Basic Usage

```cpp
#include "checkconnection.h"

void setup() {
    // Your setup code here
}

void loop() {
    // Check connectivity every 30 seconds
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
        bool isConnected = checkAndReconnectToIP("192.168.88.1", 10000);
        if (isConnected) {
            Serial.println("Connection OK");
        } else {
            Serial.println("Connection failed");
        }
        lastCheck = millis();
    }
    
    // Your other code here
    delay(100);
}
```

### Integration with Existing Code

```cpp
void checkconnectiontask() {
    if (checkconnectiontime > configdata.checkconnectiontime) {
        Serial.println("Check connection");
        checkconnectiontime = 0;
        
        // Check connectivity to 192.168.88.1 and reconnect if needed
        bool connectionOk = checkAndReconnectToIP("192.168.88.1", 10000);
        
        if (connectionOk) {
            Serial.println("Connection to 192.168.88.1 is OK");
            // Only talk to server if connection is good
            int re = talktoServer(WiFi.localIP().toString(), name, uptime, &cfg);
            if (re != 200 && configdata.havetorestart) {
                Serial.println("Server communication failed, but connection is OK");
            }
        } else {
            Serial.println("Failed to establish connection to 192.168.88.1");
        }
    }
}
```

### Manual Connection Check

```cpp
// Call this function when you need to manually check connectivity
void someEventHandler() {
    manualConnectionCheck();
}
```

### Custom IP Address

```cpp
// Check connectivity to a different IP address
bool isConnected = checkAndReconnectToIP("192.168.1.1", 8000);
```

## Configuration

The library uses the following default values:
- **Default target IP**: 192.168.88.1
- **Default timeout**: 5000ms (5 seconds)
- **WiFi reconnection delay**: 1000ms (1 second)
- **Connection attempt delay**: 500ms

## Error Handling

The library provides comprehensive error handling:

1. **WiFi disconnection**: Automatically attempts reconnection
2. **IP connectivity failure**: Disconnects and reconnects WiFi, then retests
3. **Timeout handling**: Respects user-defined timeout values
4. **Serial output**: Provides detailed feedback for debugging

## Dependencies

- ESP8266WiFi
- ESP8266HTTPClient
- Arduino.h

## Notes

- The library uses HTTP GET requests to test IP connectivity
- WiFi reconnection attempts are limited to prevent infinite loops
- All operations are non-blocking and respect timeout values
- Serial output is used for debugging and status reporting

## Troubleshooting

### Common Issues

1. **Connection always fails**: Check if the target IP is reachable from your network
2. **WiFi reconnection loops**: Verify WiFi credentials and network availability
3. **Timeout errors**: Increase timeout values for slower networks

### Debug Output

Enable Serial output to see detailed connection status:
```
Checking connectivity to 192.168.88.1
Successfully connected to 192.168.88.1 (HTTP: 200)
```

Or for failures:
```
Failed to connect to 192.168.88.1 (HTTP: -1)
Attempting to reconnect WiFi...
WiFi reconnected after failure
Successfully connected to 192.168.88.1 after reconnection (HTTP: 200)
``` 