// #define USB_RAWHID
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ICM42688.h>
#include <Adafruit_GPS.h>
//#include <USBHost_t36.h>

#define DEBUG_SERIAL
#define STREAM_SERIAL
#define SAVE_SD

#ifdef DEBUG_SERIAL
#ifndef USE_SERIAL
#define USE_SERIAL
#endif
#endif

#ifdef STREAM_SERIAL
#ifndef USE_SERIAL
#define USE_SERIAL
#endif
#endif

// GPS setup
#define GPSSerial Serial5
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
char gps;
const char GPS_ENABLE = 0;

// the number of IMUs
#define N 10
const int LOG_SWITCH = 36;
const int LOG_LED = 37;
const int chipSelect = BUILTIN_SDCARD; 

//designated chip select pins
const char CS0 = 8;
const char CS1 = 24;
const char CS2 = 7;
const char CS3 = 10;
const char CS4 = 6;
const char CS5 = 41;
const char CS6 = 15;
const char CS7 = 40;
const char CS8 = 14;
const char CS9 = 39;

bool is_logging = false;
bool log_switch_state;
bool last_log_switch_state;
char filename[16];
int filenum = 0;
int status; // used to check success of IMU data transfer
File file;

ICM42688 IMUs[] = {ICM42688(SPI,CS0),
                   ICM42688(SPI,CS1),
                   ICM42688(SPI,CS2),
                   ICM42688(SPI,CS3),
                   ICM42688(SPI,CS4),
                   ICM42688(SPI1,CS5),
                   ICM42688(SPI1,CS6),
                   ICM42688(SPI1,CS7),
                   ICM42688(SPI1,CS8),
                   ICM42688(SPI1,CS9)};

// accel bias and scale factors
double accelScale[10][3] = {{2,2,2}, {2,2,2}, {2,2,2}, {2,2,2}, {2,2,2},
                           {2,2,2}, {2,2,2}, {2,2,2}, {2,2,2}, {2,2,2}};

double accelBias[10][3] = {{-0.82, -0.95, 1.36}, 
                          {-1.16,  0.79,  0.8}, 
                          {-0.61, -1.26,-1.34}, 
                          { 0.63, -1.14, 0.64},
                          { 0.15, -0.04, 1.53},
                          {-0.92,  0.39, 1.63},
                          { 0.54,  0.96,-0.07},
                          {-0.75, -1.13, 1.28},
                          {-1.23,  -0.95, 0.25},
                          {-0.76, -1.26, 1.19},};

void start_logging();
void stop_logging();
void loadAccelFactors();
void setupIMUs(); 


void setup()
{
    pinMode(LOG_SWITCH, INPUT); // connect pullup resistor, switch float vs ground
    log_switch_state = digitalRead(LOG_SWITCH);
    last_log_switch_state = digitalRead(LOG_SWITCH);

    pinMode(LOG_LED, OUTPUT);
    digitalWrite(LOG_LED, LOW);

    pinMode(CS0, OUTPUT);
    pinMode(CS1, OUTPUT);
    pinMode(CS2, OUTPUT);
    pinMode(CS3, OUTPUT);
    pinMode(CS4, OUTPUT);
    pinMode(CS5, OUTPUT);
    pinMode(CS6, OUTPUT);
    pinMode(CS7, OUTPUT);
    pinMode(CS8, OUTPUT);
    pinMode(CS9, OUTPUT);

    digitalWrite(CS0, HIGH);
    digitalWrite(CS1, HIGH);
    digitalWrite(CS2, HIGH);
    digitalWrite(CS3, HIGH);
    digitalWrite(CS4, HIGH);
    digitalWrite(CS5, HIGH);
    digitalWrite(CS6, HIGH);
    digitalWrite(CS7, HIGH);
    digitalWrite(CS8, HIGH);
    digitalWrite(CS9, HIGH);

    #ifdef USE_SERIAL
    Serial.begin(115200); // Teensy ignores baud rate
    delay(500);
    #endif
    #ifdef DEBUG_SERIAL
    delay(500);
    Serial.println("starting up");
    #endif

    #ifdef SAVE_SD
    SD.begin(BUILTIN_SDCARD);
    delay(1000);
    file = SD.open(filename, FILE_WRITE);
    #ifdef DEBUG_SERIAL
    Serial.println("SD ready");
    #endif
    sprintf(filename, "data_%d.txt", filenum);
    while (SD.exists(filename))
    {
        #ifdef DEBUG_SERIAL
        Serial.print("found file: ");
        Serial.println(filename);
        #endif
        sprintf(filename, "data_%d.txt", ++filenum);
    }
    #ifdef DEBUG_SERIAL
    Serial.print("next file: ");
    Serial.println(filename);
    #endif
    #endif

    #ifdef DEBUG_SERIAL
    Serial.println("Trying to initialize IMUs");
    #endif

    delay(1000);
    setupIMUs();
    loadAccelFactors();

    #ifdef DEBUG_SERIAL
    Serial.println("IMUs succesfully initialized");
    #endif
    delay(500);

    //GPS setup
    pinMode(GPS_ENABLE, OUTPUT);
    digitalWrite(GPS_ENABLE, HIGH);
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPSSerial.println(PMTK_Q_RELEASE);
    delay(1000);
}

// run configuration function for all IMUs
void setupIMUs() 
    {
    for (int i = 0; i < N; i++)
    {
        status = IMUs[i].begin();
        if (status < 0){
        Serial.println("IMU " + String(i) + " initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
        while(1) {}
        }
    }
}

void loadAccelFactors(){

    for (int i = 0; i < N; i++)
    {
        IMUs[i].setAccelCalX(accelBias[i][0],accelScale[i][0]);
        IMUs[i].setAccelCalY(accelBias[i][1],accelScale[i][1]);
        IMUs[i].setAccelCalZ(accelBias[i][2],accelScale[i][2]);
    } 
}

/* -------------------------- timing control -------------------------- */
uint32_t current_micros;
uint32_t prev_micros;
uint32_t delay_micros = 1000; //1000 Hz
// uint32_t delay_micros = 50000; //20 Hz
// uint32_t delay_micros = 33333; //30 Hz
// uint32_t delay_micros = 20000; //50 Hz


bool time_passed(){
    current_micros = micros();
    if (current_micros - prev_micros >= delay_micros){
        prev_micros = current_micros; // update time
        return true;
    }
    return false;
}

/* --------------------------  -------------------------- */



void start_stop_logging(){
    log_switch_state = digitalRead(LOG_SWITCH);
    if (log_switch_state != last_log_switch_state){
        if (LOW == log_switch_state){
            stop_logging();
        } else
        {
            start_logging();
        }
        delay(10); // prevent bounceback
    }
    last_log_switch_state = log_switch_state;
}

//increment file number and generate new file
void start_logging(){
    is_logging = true;
    digitalWrite(LOG_LED, HIGH);
    sprintf(filename, "data_%d.txt", filenum++);
    #ifdef DEBUG_SERIAL
    Serial.print("creating file:");
    Serial.println(filename);
    // Serial.printf("data_%d.txt", filenum);
    #endif
    #ifdef SAVE_SD
    file = SD.open(filename, FILE_WRITE);
    #endif
}

void stop_logging(){
    is_logging = false;
    digitalWrite(LOG_LED, LOW);
    #ifdef DEBUG_SERIAL
    Serial.println("stopped logging to file: ");
    Serial.println(filename);
    #endif

    #ifdef SAVE_SD
    file.close();
    #endif
}

void loop()
{
    start_stop_logging();    
    if (is_logging && time_passed()){
        #ifdef STREAM_SERIAL
        Serial.print(current_micros);
        #endif
        #ifdef SAVE_SD
        //file = SD.open(filename, FILE_WRITE);
        file.print(current_micros);
        #endif

        //this loop reads all sensors only
        for (int i = 0; i < N; i++)
        {
            IMUs[i].readSensor();
        }

        // this loop transfers sensor data read above
        // datalogging format:
// current micros time, already written above
// IMU0 gyro XYZ ... IMU0 accel XYZ; 
// ...
// IMU5 agyro XYZ ... IMU5 accel XYZ;
// GPS lat, GPS long
   
        for (int i = 0; i < N; i++)
        {
            #ifdef STREAM_SERIAL
            Serial.print(","); Serial.print(IMUs[i].getGyroX_rads());
            Serial.print(","); Serial.print(IMUs[i].getGyroY_rads());
            Serial.print(","); Serial.print(IMUs[i].getGyroZ_rads());
            Serial.print(","); Serial.print(IMUs[i].getAccelX_mss());
            Serial.print(","); Serial.print(IMUs[i].getAccelY_mss());
            Serial.print(","); Serial.print(IMUs[i].getAccelZ_mss());

            #endif
            #ifdef SAVE_SD
            file.print(","); file.print(IMUs[i].getGyroX_rads());
            file.print(","); file.print(IMUs[i].getGyroY_rads());
            file.print(","); file.print(IMUs[i].getGyroZ_rads());
            file.print(","); file.print(IMUs[i].getAccelX_mss());
            file.print(","); file.print(IMUs[i].getAccelY_mss());
            file.print(","); file.print(IMUs[i].getAccelZ_mss());
            #endif
        }
        //read GPS
        gps = GPS.read();
        GPS.parse(GPS.lastNMEA());

        #ifdef STREAM_SERIAL
        Serial.print(","); Serial.print(GPS.latitude,4);
        Serial.print(","); Serial.print(GPS.longitude,4);
        Serial.print(","); Serial.print(GPS.speed);
        Serial.println();
        #endif
        #ifdef SAVE_SD
        file.print(","); file.print(GPS.latitude,4);
        file.print(","); file.print(GPS.longitude,4);
        file.print(","); file.print(GPS.speed);
        file.println();
        //file.close();
        #endif
    }
}

//TODO: start logging
//TODO: stop logging

//     // put your main code here, to run repeatedly:
//     // data will be sent to server
//     uint16_t v1 = analogRead(P1);
//     uint16_t v2 = analogRead(P2);

//     char str[50];

//     uint8_t buffer[50];
    
//     for (int i = 0; i < 50; i++)
//     {
//         buffer[i] = str[i];
//     }
    
//     // This initializes udp and transfer buffer
//     udp.beginPacket(udpAddress, udpPort);
//     udp.write(buffer, 11);
//     udp.endPacket();
//     // Serial.println("packet sent");
//     memset(buffer, 0, 50);
//     // processing incoming packet, must be called before reading the buffer
//     udp.parsePacket();
//     // receive response from server, it will be HELLO WORLD
//     if (udp.read(buffer, 50) > 0)
//     {
//         Serial.print("Server to client: ");
//         Serial.println((char *)buffer);
//     }
//     // Wait for 0.033 seconds (about 30fps)
//     delay(33);
// }



    // // get current time & check if enough has passed to run each callback
    // uint32_t prev_micros = micros();


    // for (int ix = 0; ix < N_CALLBACKS; ix++){
    //     if (current_micros - callbacks[ix].prev_micros >= callbacks[ix].delay_micros){
    //         // update time
    //         // see http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592
    //         callbacks[ix].prev_micros = current_micros;

    //         // call the pointer to the callback fxn if it's enabled
    //         if (callbacks[ix].enabled){ (*(callbacks[ix].fxn))(); }
    //     }
    // }
