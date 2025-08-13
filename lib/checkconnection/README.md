# Connection Manager Library

This library provides enhanced WiFi connection management for ESP8266 devices with automatic reconnection, connection status monitoring, and server ping functionality.

## Features

- **Connection Status Monitoring**: Real-time WiFi connection status tracking
- **Automatic Reconnection**: Configurable auto-reconnect with retry limits
- **Server Ping**: Test connectivity to remote servers
- **Connection Information**: Get detailed connection info (IP, MAC, RSSI, etc.)
- **Configurable Settings**: Customizable reconnect intervals and attempt limits
- **JSON Status Output**: Get connection status in JSON format for APIs

## Files

- `connection_manager.h` - Header file with class definitions
- `connection_manager.cpp` - Implementation file
- `connection_example.cpp` - Usage examples
- `checkconnection.h` - Original simple connection check (legacy)
- `checkconnection.cpp` - Original implementation (legacy)

## Quick Start

### 1. Include the library

```cpp
#include "connection_manager.h"
```

### 2. Create a ConnectionManager instance

```cpp
Configfile cfg("/config.cfg");
ConnectionManager* connectionManager = new ConnectionManager(&cfg);
```

### 3. Configure settings

```cpp
connectionManager->setReconnectInterval(30000); // 30 seconds
connectionManager->setMaxReconnectAttempts(5);
connectionManager->enableAutoReconnect(true);
```

### 4. Connect to WiFi

```cpp
if (connectionManager->connect()) {
    Serial.println("Connected successfully!");
} else {
    Serial.println("Connection failed!");
}
```

### 5. Monitor in loop

```cpp
void loop() {
    connectionManager->update(); // Call regularly
    
    if (connectionManager->isDisconnected()) {
        Serial.println("WiFi disconnected!");
    }
    
    delay(1000);
}
```

## API Reference

### Connection Status Functions

#### `ConnectionStatus getConnectionStatus()`
Returns the current connection status:
- `CONNECTED` - Successfully connected to WiFi
- `DISCONNECTED` - Not connected to WiFi
- `CONNECTING` - Currently attempting to connect
- `CONNECTION_FAILED` - Connection attempt failed
- `AP_MODE` - Device is in Access Point mode

#### `bool isConnected()`
Returns `true` if currently connected to WiFi.

#### `bool isDisconnected()`
Returns `true` if not connected to WiFi.

#### `String getStatusString()`
Returns a human-readable status string.

### Connection Management Functions

#### `bool connect()`
Attempts to connect to WiFi using configured SSID and password.
Returns `true` if successful.

#### `bool reconnect()`
Disconnects and attempts to reconnect to WiFi.
Returns `true` if successful.

#### `void disconnect()`
Disconnects from WiFi.

#### `void enableAutoReconnect(bool enable)`
Enables or disables automatic reconnection.

### Configuration Functions

#### `void setReconnectInterval(unsigned long interval)`
Sets the interval between reconnection attempts (in milliseconds).

#### `void setMaxReconnectAttempts(int attempts)`
Sets the maximum number of reconnection attempts before giving up.

### Monitoring Functions

#### `void update()`
Updates connection status and handles auto-reconnection.
Call this function regularly in your main loop.

#### `void checkConnection()`
Checks current connection status and triggers reconnection if needed.

#### `bool pingServer(String serverUrl = "")`
Pings a server to test connectivity.
Returns `true` if ping is successful.

### Utility Functions

#### `String getLocalIP()`
Returns the device's local IP address.

#### `String getMacAddress()`
Returns the device's MAC address.

#### `int getRSSI()`
Returns the WiFi signal strength (RSSI).

#### `unsigned long getConnectionDuration()`
Returns how long the device has been connected (in milliseconds).

#### `int getFailedAttempts()`
Returns the number of failed connection attempts.

#### `ConnectionInfo getConnectionInfo()`
Returns a structure with all connection information.

## Integration with Existing Code

### Replace the existing `checkconnectiontask()` function:

```cpp
void enhancedCheckConnectionTask() {
    static unsigned long lastCheck = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastCheck > 10000) { // Check every 10 seconds
        lastCheck = currentTime;
        
        connectionManager->update();
        
        if (connectionManager->isConnected()) {
            if (!connectionManager->pingServer()) {
                if (configdata.havetorestart) {
                    ESP.restart();
                }
            }
        } else {
            connectionManager->reconnect();
        }
    }
}
```

### Add to your main.cpp:

```cpp
// Add to global variables
ConnectionManager* connectionManager = nullptr;

// Add to setup()
connectionManager = new ConnectionManager(&cfg);
connectionManager->setReconnectInterval(30000);
connectionManager->setMaxReconnectAttempts(5);
connectionManager->enableAutoReconnect(true);

// Add to loop()
connectionManager->update();
```

## Configuration

The ConnectionManager uses the following configuration keys from your config file:

- `ssid` - WiFi network name
- `password` - WiFi password
- `maxconnecttimeout` - Maximum connection timeout (seconds)
- `talkurl` - Server URL for ping testing

## Example Output

### Serial Output:
```
Attempting to connect to WiFi...
SSID: MyWiFiNetwork
WiFi connected successfully!
HTTP Response code: 200
Response: OK
```

### JSON Status:
```json
{
  "status": "Connected",
  "ssid": "MyWiFiNetwork",
  "ip": "192.168.1.100",
  "mac": "AA:BB:CC:DD:EE:FF",
  "rssi": -45,
  "failed_attempts": 0,
  "connection_duration": 3600000,
  "auto_reconnect": true
}
```

## Troubleshooting

1. **Connection fails immediately**: Check SSID and password in config
2. **Auto-reconnect not working**: Ensure `enableAutoReconnect(true)` is called
3. **Server ping fails**: Check server URL and network connectivity
4. **Memory issues**: Consider reducing reconnect interval or max attempts

## Migration from Legacy Code

The original `checkconnection.h` and `checkconnection.cpp` files are still available for backward compatibility. To migrate:

1. Replace `#include "checkconnection.h"` with `#include "connection_manager.h"`
2. Replace `talktoServer()` calls with `connectionManager->pingServer()`
3. Add `connectionManager->update()` to your main loop
4. Use `connectionManager->getConnectionStatus()` instead of manual WiFi.status() checks 