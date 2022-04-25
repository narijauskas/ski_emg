#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// following example from:
// http://www.iotsharing.com/2017/06/how-to-use-udpip-with-arduino-esp32.html

const int P1 = 32;
const int P2 = 33;

// wifi name and password
const char *ssid = "CenturyLink4932";
const char *pwd = "5ucmhdkpda8nvv";

// udp settings
const char * udpAddress = "192.168.0.36";
const int udpPort = 54545;
WiFiUDP udp;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    // Connect to the WiFi network
    WiFi.begin(ssid, pwd);
    delay(1000);
    Serial.println("Attempting WiFi connection.");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    // put your main code here, to run repeatedly:
    // data will be sent to server
    uint16_t v1 = analogRead(P1);
    uint16_t v2 = analogRead(P2);

    char str[50];
    sprintf(str, "%d,%d,", v1, v2);

    uint8_t buffer[50];
    
    for (int i = 0; i < 50; i++)
    {
        buffer[i] = str[i];
    }
    
    // This initializes udp and transfer buffer
    udp.beginPacket(udpAddress, udpPort);
    udp.write(buffer, 11);
    udp.endPacket();
    // Serial.println("packet sent");
    memset(buffer, 0, 50);
    // processing incoming packet, must be called before reading the buffer
    udp.parsePacket();
    // receive response from server, it will be HELLO WORLD
    if (udp.read(buffer, 50) > 0)
    {
        Serial.print("Server to client: ");
        Serial.println((char *)buffer);
    }
    // Wait for 0.033 seconds (about 30fps)
    delay(33);
}
