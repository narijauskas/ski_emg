// #define USB_RAWHID
#include <Arduino.h>
// #include <SPI.h>
#include <SD.h>

#define SERIAL_DEBUG
#define SERIAL_STREAM
#define SAVE_SD

// the number of EMGs
#define N 2
const int EMG_PIN[N] = {14,15};
const int LOG_SWITCH = 36;
const int LOG_LED = 13;

#ifdef SAVE_SD
#endif



bool is_logging = false;
bool log_switch_state;
bool last_log_switch_state;
char filename[16];
int filenum = 0;
File file;

void start_logging();
void stop_logging();


void setup()
{
    pinMode(LOG_SWITCH, INPUT); // connect pullup resistor, switch float vs ground
    last_log_switch_state = digitalRead(LOG_SWITCH);

    pinMode(LOG_LED, OUTPUT);
    digitalWrite(LOG_LED, LOW);

    delay(2000);


    #ifdef SERIAL_DEBUG
    Serial.begin(115200); // Teensy ignores baud rate
    delay(5000);
    Serial.println("starting up");
    #endif


    #ifdef SAVE_SD
    SD.begin(BUILTIN_SDCARD);
    delay(1000);
    sprintf(filename, "data_%d.txt", filenum);
    while (SD.exists(filename))
    {
        Serial.print("found file: ");
        Serial.println(filename);
        sprintf(filename, "data_%d.txt", filenum++);
    }
    Serial.println("SD ready");
    Serial.print("next file: ");
    Serial.println(filename);
    #endif
}





/* -------------------------- timing control -------------------------- */
uint32_t current_micros;
uint32_t prev_micros;
// uint32_t delay_micros = 10000; //100 Hz
uint32_t delay_micros = 33333; //30 Hz

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
            start_logging();
        } else
        {
            stop_logging();
        }
    }
    last_log_switch_state = log_switch_state;
}

//increment file number and generate new file
void start_logging(){
    is_logging = true;
    digitalWrite(LOG_LED, HIGH);
    sprintf(filename, "data_%d.txt", filenum++);
    Serial.print("creating file:");
    Serial.println(filename);
    file = SD.open(filename, FILE_WRITE);
    file.println("time,EMG1,EMG2");
    file.close();
}

void stop_logging(){
    is_logging = false;
    digitalWrite(LOG_LED, LOW);
    Serial.println("stopped logging to file: ");
    Serial.println(filename);
}

void loop()
{
    start_stop_logging();    
    if (is_logging && time_passed()){

        Serial.print(current_micros);
        file = SD.open(filename, FILE_WRITE);
        file.print(current_micros);

        for (int i = 0; i < N; i++)
        {
            int val = analogRead(EMG_PIN[i]);
            Serial.print(",");
            Serial.print(val);
            file.print(",");
            file.print(val);
        }

        Serial.println();
        file.println();
        file.close();
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
