extern "C" {
#include "Joystick.h"
}

#include <HardwareSerial.h>
#include <Arduino.h>
#include <wiring.c>

void myDelay(unsigned long ms)
{
	uint32_t start = micros();

	while (ms > 0) {
		;
		while ( ms > 0 && (micros() - start) >= 1000) {
			ms--;
			start += 1000;
		}
	}
}

// Main entry point.
int main(void) {
    // We'll start by performing hardware and peripheral setup.
    SetupHardware();
    // We'll then enable global interrupts for our use.
    GlobalInterruptEnable();
    // Initialize the UART
    init();
    Serial1.begin(115200);
    Serial1.write("START");
    // Once that's done, we'll enter an infinite loop.
    for (;;)
    {
        if(needsNewData()){
          Serial1.write("1\n");
          Serial1.flush();
          while(Serial1.available()){
            ;
          }
          String nextData = Serial1.readStringUntil('\n');
          Serial1.write(nextData.c_str());
          putNewData(const_cast<char*>(nextData.c_str()));
        }
        // We need to run our task to process and deliver data for our IN and OUT endpoints.
        HID_Task();
        // We also need to run the main USB management task.
        USB_USBTask();
    }
}

void setup(){}
void loop(){}