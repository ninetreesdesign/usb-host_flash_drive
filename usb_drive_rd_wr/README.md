# usb_drive_rd_mod

my code to write to Host USB Drive board.
Tested with Teensy 3.2

### points to know:
	- the command strings to the drive must end with \r only. No \n
	- all commands need delays; some are significantly longer. The type of usb drive makes times vary. an SD card/usb-style drive is much slower. probably to be avoided

## Notes for an email to send to maker

Problems reading and writing to USB drive

If I connect from USB HOST board directly to a terminal
I get expected output from commands:
```
dir
2019-02-12 18:20:04 <DIR>           .
2019-02-12 18:20:04 <DIR>           ..
2019-02-12 19:10:04               8 ARDU_101.TXT
2019-02-12 18:21:00              18 TEXT1.TXT
SMALL      :\FOLDER1>

SMALL      :\FOLDER1> date 2019-11-11
Date Set successfully
SMALL      :\FOLDER1> date
Current date: 2019-11-11
```
Using the Example read script, I get scrambled and lost characters.

I found with my own script, the 50ms delay wasn't enough. 100+ gave some response.

I created a 1.txt file at root of USB drive.
Its contents are 3 lines:
The first one.
The second one.
The third is a bit longer.

The output is

Lines in file = 3
Line 1 = The Line 2 = firsLine 3 = t on

By adding delay(100) after each sent command
   adding a delay(1) before each character read in the while loop, 
   
   I get a clear output.


Lines in file = 3
Line 1 = The first one.
Line 2 = The second one.
Line 3 = The third is a bit longer.

Then I added a $DIR command afterward, and it worked.
I then changed to read file 2.txt with 21 lines in it, and the subsequent $DIR command doesn't display; I thought it must be getting stuck in one of the while loops, but a print statement shows it doesn't exit the loop to print each line.

I found that it's trying to read one too many lines. Decreasing by 1 fixed. Doesn't work on original file. I think issue is whether there is a CR at end of line or of file. The line count is correct.
I'm not sure how to resolve yet; maybe read into a line buffer, then check for EOL?

I've designed this into a data logging system, but with a few hours into this part, I still don't have a working solution.
 I am still running into issues with write and read.
 
Maybe these notes are helpful? Maybe you have other guidance or a more complete example?

I am including my modified version of the example for reference.


