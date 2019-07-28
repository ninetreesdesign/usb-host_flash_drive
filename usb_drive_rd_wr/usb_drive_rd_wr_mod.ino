/*
    USB HOST BOARD - Flash Drive Example
    see manufacturer:      https://www.hobbytronics.co.uk/usb-host-board-v2
    Using the host-Mini    http://www.hobbytronics.co.uk/usb-host/usb-host-mini
    Code sample and cmds   https://www.hobbytronics.co.uk/usb-host-flash-drive

    Change to talk to T3 hardware serial port
    connections: Tx,Rx goes to Rx1,Tx1 of T3
    set to default baud rate of 9600
    case of cmd doesn't matter
    tests ok connected directly to PC terminal via USB-TTL adapter

    notes on hardware serial: https://www.pjrc.com/teensy/td_uart.html
    todo:
        done: make filenmae a variable in write fctns
        add readback test from file(s) (reads not working)

     Demo Arduino sketch to show how to read number of lines in a file
     and to individually read each line.
     Useful for data or config files

     Hobbytronics Ltd 2013 (www.hobbytronics.co.uk)
*/

#include <stdio.h>
//#include <SoftwareSerial.h>

#define HW1 Serial1   //  USB board connected here, Rx1 Tx1 are 0,1 respectively HW port
#define P   Serial.print
#define Pln Serial.println
#define HW1 Serial1

const byte ctrlM = 13;
const byte ctrlZ = 26;
int file_lines, i;
byte serial_char;
char file_name[13] = "1.TXT";  // File name
char s[80]; // String to hold commands to be sent
char input_buf[255]; // String to hold received characters
uint16_t t0 = millis();


void setup() {
    pinMode(2, INPUT);

    while (!Serial && millis() < 5000) {}  // timeout and proceed if monitor port not opened
    Serial.begin(115200);
    Serial.println("starting...");

    HW1.begin(9600);
    delay(1000);

    if (digitalRead(2) == 0) {      // read IO pin to detect USB drive inserted
        Pln("no USB drive.");
    }

    // Set the DATE and TIME
    sprintf(s, "%s %s", "$TIME", "09:00:00 \r");
    sendToFlash(s);
    sprintf(s, "%s %s", "$DATE", "2019-02-12 \r");
    sendToFlash(s);

    // change directory
    sprintf(s, "$CD Folder1 \r");
    sendToFlash(s);

    // show directory
    sprintf(s, "$DIR \r");
    //Pln(s);
    sendToFlash(s);
    delay(500);
    getFromFlash();

    sprintf(s, "$DATE \r");
    //Pln(s);
    sendToFlash(s);
    delay(500);
    getFromFlash();

    sprintf(s, "$TIME \r");
    //Pln(s);
    sendToFlash(s);
    delay(500);
    getFromFlash();

    // Create a file
    sprintf(s, "%s %s", "$WRITE \r", file_name);
    //Pln(s);
    sendToFlash(s);
    delay(1000);        // Needs extra time to create the file. Time depends on flash drive used

    // Write some lines to the file
    for (byte i = 0; i < 5; i++) {
        sprintf(s, "%s %2d \r", "xline", i + 1);
        sendToFlash(s);
    }
    HW1.write(26);  // closeFile();  // Send "Close file" char 26 ( Control-Z ) // Give time for the file to be saved and closed

//    // Now re-open file and append data to it
//    sprintf(s, "%s %s", "$APPEND", file_name);
//    sendToFlash(s);
//    delay(1000);
//
//    // Write ten lines to the file
//    for (byte i = 0; i < 3; i++) {
//        sendToFlash("Appended line to file.");
//        sprintf(s, "%s %2d", "appended line", i*10 + 1);
//        sendToFlash(s);
//        Pln(s);
//    }
//
//    closeFile();
//
//    // show directory
//    sprintf(s, "$DIR\r");
//    sendToFlash(s);
//    Pln(s);
//
//
//    // read file size
//    sprintf(s, "$SIZE %s line\r", file_name);
//    sendToFlash(s);
//    Pln(s);
//
//    // print the file contents
//    sprintf(s, "$TYPE %s line\r", file_name);
//    sendToFlash(s);
//    Pln(s);

//    while (!HW1.available() && (millis() - t0) < 20000);     // Wait for data to be returned, add timeout
//    // Number of lines in file returned as ascii chars, so convert to number nnn = n*100 + n*10 + n
//    uint16_t file_lines = 0;
//    while (HW1.available() > 0) {
//        file_lines *= 10;
//        file_lines += HW1.parseInt();
//    }
//    // Print filesize information
//    sprintf(s, "Lines in file = %d", file_lines);
//    Pln(s);

//    closeFile();


    Pln("\n  end.");
}


void loop() {
    // Nothing to do here
}


// Create a printing function which has a built-in delay
void sendToFlash(char *pstring) {
    HW1.println(pstring);
    delay(99);      // 50 is not enough...
}

void getFromFlash() {
    Pln("  getFromFlash()...");
    // get flash response
    int buf_pos = 0;
    char c;
    int t = millis();
    while (!HW1.available());       // && (millis() - t) < 10000) {    // add timeout

    while (HW1.available()>0) {     // Wait for data to be returned
        //c = (char)HW1.read();
        //input_buf[buf_pos] = c;
        //buf_pos++;
        // Serial.print(" ");
        delay(1);
        Serial.write(HW1.read());
        // Serial.print( (char)HW1.read() );
    }
    Pln(); //(input_buf);
}


void closeFile() {
    HW1.write(ctrlZ);  // close file
    delay(1000);
}
