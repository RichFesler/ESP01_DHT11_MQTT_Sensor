# ESP01_DHT11_MQTT_Sensor

Simple ESP-01 project that reads a DHT11 sensor and publishes data to MQTT.

## Features
- Publishes temperature, humidity, and heartbeat every 10 seconds to MQTT Server.
- Listens for a Published MQTT reset command (`MQTT/Outside/RESET` topic).

## MQTT Topics
| Topic | Description |
|-------|--------------|
| `MQTT/Outside/Temp` | Temperature in °C |
| `MQTT/Outside/Humidity` | Relative humidity in % |
| `MQTT/Outside/Heartbeat` | Incrementing counter |
| `MQTT/Outside/RESET` | Accepts message `"RESET"` or `"REBOOT"` to restart device |

## Example Reset Command
```bash
mosquitto_pub -h 192.168.n.n -p 1883 -t "MQTT/Outside/RESET" -m "RESET"
