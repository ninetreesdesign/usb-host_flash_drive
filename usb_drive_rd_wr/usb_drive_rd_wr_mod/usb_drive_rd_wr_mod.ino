// script to send commands to USB drive, then read and print the responses
// modified to fix items in https://www.hobbytronics.co.uk/usb-host-flash-drive
// my modified example. Needs additional delays, add'l newlines

// Note: Use silent mode "$" (no echo) for commands sent to drive
// Terminate ONLY with \r. no NL in msg; spaces are ok. no leading \r or \n


//  03/18/18    added drive_inserted status check
//              timeout on getFromFlash()
//              start tests for writing data
//  03/21       added EEPROM rd wr
//  09/17/19    revisit to check hardware; revisions for clarity... I broke something check the modified _mod

//#include <stdio.h>
#include <EEPROM.h>

#define HWserial Serial1

int  DRIVE_STATUS_PIN = 2;          // flash drive
int  LED_PIN          = 13;         // built-in

bool g_flash_drive_detected = 0;    // flag
bool g_echo_flag = 1;               // prints to the terminal what is written to flashDrive (HW port)
bool g_diag_flag = 1;               // true while debugging

// define locations to store settings on EEprom
const uint8_t ADDR_DATE   =  05;    // 10 char
const uint8_t ADDR_TIME   =  15;    // 8 char
const uint8_t ADDR_FN_PRE =  30;    // 4 char log file name prefix     ex: test
const uint8_t ADDR_FN_SEQ =  40;    // 4 byte log file sequence number ex: 0001

const byte CTRL_Z = 26;
const byte CR = 13;                 // Carriage Return
const byte LF = 10;                 // Linefeed
const int FNAME_LENGTH = 13;        // AAAAAAAA.AAA + 1
char g_new_fname[FNAME_LENGTH]    = "TEST_02.TXT";    // must be <= 8.3 length
char g_filename[FNAME_LENGTH]     = "A102CRLF.TXT";   // user enters this
// char g_filename[] = "A101CR.TXT";           // doesn't read correctly. without LF, all lines join
const uint8_t STRING_LENGTH = 180;
char cbuf[STRING_LENGTH];                   // String to hold commands to be sent
int g_delay_for_flash = 100;
int t_start = millis();

void setup() {
    pinMode(LED_PIN,OUTPUT);
    pinMode(DRIVE_STATUS_PIN,INPUT);

    Serial.begin(9600);
    while (!Serial && millis() < 5000) {}  // timeout and proceed if monitor port not opened
    Serial.println("\n\nSerial port initialized.\n");

    HWserial.begin(9600);   // flash drive baud rate
    delay(10);


    // write header info to EEprom
    // these don't depend on flash drive being inserted

    // set date and time default values
    char the_date[11] = "2020-01-01";
    char the_time[9]  = "01:00:00";
    // set date and time default values
    char theDate[11] = "2020-01-01";
    char theTime[9]  = "01:00:00";
    sprintf(cbuf,"%s %s \r", "$DATE", theDate);
    sendToFlash(cbuf,10);
    sprintf(cbuf,"%s %s \r", "$TIME", theTime);
    sendToFlash(cbuf,10);
    Serial.println();
    delay(2501);    // should see seconds increase

//    eepromWriteChars(ADDR_DATE, the_date, strlen(the_date) );
//    eepromWriteChars(ADDR_TIME, the_time, strlen(the_time) );
//
//    // read back the date and time
//    for (byte i=0; i<strlen(the_date); i++) {
//        the_date[i] = EEPROM.read(ADDR_DATE+i);
//    }
//    for (byte i=0; i<strlen(the_time); i++) {
//        the_time[i] = EEPROM.read(ADDR_TIME+i);
//    }

    // worked
    // Serial.println(the_date);    // temp
    // Serial.println(the_time);    // temp

    g_flash_drive_detected = checkDriveStatus();
    if (g_flash_drive_detected) {
        sprintf(cbuf,"%s %s \r", "$DATE", the_date);
        sendToFlash(cbuf,100);  //characters, delay time
        getFromFlash();

        sprintf(cbuf,"%s %s \r", "$TIME", the_time);
        sendToFlash(cbuf,100);
        getFromFlash();
        Serial.println();

        getDirectory(); // reads and prints directory and files of flash drive

        uint32_t num_lines = getFileSize(g_filename);
        // uint32_t num_bytes = getFileSize(g_filename, 'B');
        //sprintf(cbuf,"  %s,  %lu Lines,  %u Bytes\n",g_filename, num_lines, num_bytes);
        sprintf(cbuf,"  %s,  %lu Lines  \n",g_filename, num_lines);
        Serial.print(cbuf);

        // works
        //printFileContents(g_filename,num_lines);

        // create a file and write to it
        newFile(g_new_fname);
        testWrite();    // outputs a few lines
        closeFile();

        appendFile(g_new_fname);
        testWrite();    // outputs a few lines
        closeFile();

        num_lines = getFileSize(g_new_fname);
        sprintf(cbuf,"  %s,  %lu Lines  \n",g_new_fname, num_lines);
        Serial.print(cbuf);

    }
    else
        Serial.println("Skipped, no drive detected.");

    Serial.println("\nDone.");
}


/// main loop ------------------------------------------
void loop() {
    checkDriveStatus();
    delay(500);
}


// functions -------------------------------------------

bool checkDriveStatus() {
    bool current_state = digitalRead(DRIVE_STATUS_PIN);
    if (g_diag_flag)
        current_state  ? Serial.println("#DRIVE_DETECTED") : Serial.println("#DRIVE_NOT_DETECTED");

    digitalWrite(LED_PIN,current_state);  // show on built-in led
    return (current_state);  // true/false
}


// Create a printing function which has a built-in delay
void sendToFlash(char buf[STRING_LENGTH], uint16_t t_delay) {
    HWserial.print(buf);
    delay(t_delay); //try 200       // 100, 50 is not enough...

    if (g_echo_flag) {              // echo msg to terminal
        Serial.println(buf);        // add LF to the terminal print
    }
}


void getFromFlash() {
    char c = '\0';
    const int SHORT_BUF_LENGTH = 16;
    char short_buf[SHORT_BUF_LENGTH];
    uint16_t char_counter = 0;
    bool timeout_flag = 0;
    int t0 = millis();

    // Hang out here
    while(!HWserial.available() && !timeout_flag) {
        timeout_flag = ((millis() - t0) < 900);    // stop waiting after 900ms
    }   // Wait for data to be returned

    if (!timeout_flag) {
        // loop waiting for a character for each line separately
        // Read and display contents of line returned
        while (HWserial.available() > 0) {
            c = HWserial.read();
            delay(5);   // ***
            Serial.write(c);
            if (char_counter < SHORT_BUF_LENGTH) {
                short_buf[char_counter] = c;
                char_counter++;
            }
        }
        // if the last received char is not an EOL, send one.
        if (c != '\r' && c != '\n') {
            Serial.write(CR);
            Serial.write(LF);
        }
    }
    else
        Serial.println("Drive read timed out.");
}


// Create new file, will overwrite
void newFile(char g_new_fname[13]) {        // length of an 8.3 file name
    if (checkDriveStatus() == 1) {
        sprintf(cbuf, "%s %s\r", "$WRITE", g_new_fname);
        sendToFlash(cbuf,1000);   // Needs extra time to create the file. Time depends on flash drive used
    }
    else
        Serial.println("newFile: NO FLASH DRIVE!");
}


// opens an existing file, writes new stuff at end
void appendFile(char g_new_fname[13]) {
    if (checkDriveStatus() == 1) {
        // Now re-open file and append data to it
        sprintf(cbuf, "%s %s\r", "$APPEND", g_new_fname);
        sendToFlash(cbuf,800);
    }
    else
        Serial.println("appendFile: NO FLASH DRIVE!");
}


void closeFile() {
    if (checkDriveStatus() == 1) {
        HWserial.write(CTRL_Z);      // Send "Close file" character
        delay(1000);                // Give time for the file to be saved and closed
        Serial.println("  Sent CloseFile char. \n");
    }
    else
        Serial.println("closeFile: NO FLASH DRIVE!");
}


// show directory of text files only
// err: still drops last part of dir...
void getDirectory() {
    if (checkDriveStatus() == 1) {
        // sprintf(cbuf, "$DIR *.TXT \r");         //  pattern match
        sprintf(cbuf, "$DIR /F *.TXT \r");    // filename-only flag, pattern match
        sendToFlash(cbuf,900);  // needs a long time. Doubling doesn't fix.

        getFromFlash();
        Serial.println("  End of get directory.");
    }
    else
        Serial.println("NO FLASH DRIVE!\n");
}


// Issue command to return number of lines in file
uint16_t getFileSize( char fn[13] ) {
    // Issue command to return number of lines in file
    sprintf(cbuf,"$SIZE %s line\r",fn);
    HWserial.write(cbuf);
    Serial.println(cbuf);
    delay(900);   // ***
    bool timeout_flag = 0;
    int t0 = millis();
    while(!HWserial.available() && !timeout_flag) {
        timeout_flag = ((millis() - t0) < 100);    // stop waiting after ...
        ;   // Wait for data to be returned
    }

    // Number of lines in file
    uint16_t file_lines = 0;
    while (HWserial.available() > 0) {
        file_lines+=HWserial.parseInt();
        delay(500);
    }
    // Print file size information
    file_lines -= 1;        // reads an extra after last

    // temp print out
    //sprintf(cbuf,"Total Lines: %d",file_lines);
    //Serial.println(cbuf);

    return(file_lines);
}


////uint32_t getFileSize(char fn[13], char size_units) {
//uint16_t xgetFileSize(char fn[13]) {
//    bool timeout_flag = 0;
//    if (1) { // (size_units == 'L') {
//        uint16_t size_L = 0;   // Number of lines in file
//        sprintf(cbuf,"$SIZE   %s LINE\r",fn);
//        sendToFlash(cbuf,900);
//        size_L = atoi(short_buf);
//        size_L -= 1;        // reads an extra after last; so decrease count by 1
//
//        return(size_L); // tbd
//    }
////    else if (size_units == 'B') {  // TBD
////        uint32_t size_B = 0;
////        sprintf(cbuf,"$SIZE   %s BYTE\r",fn);
////        sendToFlash(cbuf,900);
////        ///size_B = (uint32_t)getFromFlash();
////        getFromFlash();
////
////        // return(size_B); // tbd
////    }
//}


//this doesn't seem to work at all.
//void printFileContents(char fn[13]) {
//    sprintf(cbuf,"#PRINT_FILE, %s", fn);
//    Serial.println(cbuf);
//    // send command to read content of a specific line
//    sprintf(cbuf,"$TYPE %s \r",fn);
//    sendToFlash(cbuf,900);
//    getFromFlash();
//}


void printFileContents(char fn[13], uint16_t num_lines) {
// read the file, Loop through each line
// *line number count starts at 1
    for(uint16_t i=0; i<num_lines; i++) {
        sprintf(cbuf,"Line %2d contains:",i+1);
        Serial.print(cbuf);
        // send command to read content of a specific line
        sprintf(cbuf,"$READ %s %2d\r",fn,i+1);        // function counts starting with 1
        HWserial.write(cbuf);
        (i==0) ? delay(1800) : delay(900);  // Sometimes first line times out... needs extra time

        getFromFlash();
    }
    Serial.println();
}


// Write some lines to the open file
void testWrite() {
    for (int i = 0; i < 10; i++) {
        sprintf(cbuf, "A line of text. %2d: %6.2f sec \r\n",  i+1, (micros() - t_start)/1e6 );
        sendToFlash(cbuf, 100);
    }
}


// also store in MCU's eeprom
void eepromWriteChars(byte addr, char str[16], byte N) {
    for (byte i=0; i<N; i++) {
        EEPROM.write(addr+i,str[i]);
    }
}
