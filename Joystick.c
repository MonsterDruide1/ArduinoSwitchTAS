/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/

#include "Joystick.h"

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	X,
	Y,
	A,
	B,
	L,
	R,
	THROW,
	NOTHING,
	TRIGGERS
} Buttons_t;

typedef struct {
	Buttons_t button;
	uint16_t duration;
} command; 

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.

	#ifdef ALERT_WHEN_DONE
	// Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
	#warning LED and Buzzer functionality enabled. All pins on both PORTB and \
PORTD will toggle when printing is done.
	DDRD  = 0xFF; //Teensy uses PORTD
	PORTD =  0x0;
                  //We'll just flash all pins on both ports since the UNO R3
	DDRB  = 0xFF; //uses PORTB. Micro can use either or, but both give us 2 LEDs
	PORTB =  0x0; //The ATmega328P on the UNO will be resetting, so unplug it?
	#endif
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}

typedef enum {
	SYNC_CONTROLLER,
	SYNC_POSITION,
	BREATHE,
	PROCESS,
	CLEANUP,
	DONE
} State_t;
State_t state = SYNC_CONTROLLER;

char* nextData = NULL;

bool needsNewData(){
  return nextData == NULL;
}

void putNewData(char* newData){
  nextData = newData;
}

void split(char buf[], char delimiter, char* array[])
{
    int i = 0;
    char *p = strtok (buf, delimiter);

    while (p != NULL)
    {
        array[i++] = p;
        p = strtok (NULL, delimiter);
    }
}

int getSplitCount(char buf[], char delimiter){
  int count = 0;
  char *ptr = buf;
  while((ptr = strchr(ptr, ' ')) != NULL) {
    count++;
    ptr++;
  }
  return count;
}

#define ECHOES 2
int echoes = 0;
USB_JoystickReport_Input_t last_report;

int report_count = 0;
int xpos = 0;
int ypos = 0;
int bufindex = 0;
int duration_count = 0;
int portsval = 0;
int frameNo = 0;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;

	// Repeat ECHOES times the last report
	if (echoes > 0)
	{
		memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
		echoes--;
		return;
	}
	frameNo++;

	// States and moves management
	switch (state)
	{

		case SYNC_CONTROLLER:
			state = BREATHE;
			break;

		case SYNC_POSITION:
			bufindex = 0;


			ReportData->Button = 0;
			ReportData->LX = STICK_CENTER;
			ReportData->LY = STICK_CENTER;
			ReportData->RX = STICK_CENTER;
			ReportData->RY = STICK_CENTER;
			ReportData->HAT = HAT_CENTER;


			state = BREATHE;
			break;

		case BREATHE:
			state = PROCESS;
			break;

		case PROCESS:
		
			if(nextData == NULL){
        state = CLEANUP;
        return;
			}
			
			char* partsOfNextEntry[4];
			split(nextData,' ',partsOfNextEntry);
			nextData = NULL;
			
			if(frameNo == strtol(partsOfNextEntry[0])){
        char* buttons[getSplitCount(partsOfNextEntry[1],';')];
        split(partsOfNextEntry[1],';',buttons);
        char* lStick[2];
        split(partsOfNextEntry[2],';',lStick);
        char* rStick[2];
        split(partsOfNextEntry[3],';',rStick);
        
        for(int i=0;i<sizeof(buttons)/sizeof(buttons[0]);i++) {
          char* currentButton = buttons[i];
          if(strcmp(currentButton, "KEY_A")){
            ReportData->Button |= SWITCH_A;
          }
          if(strcmp(currentButton, "KEY_B")){
            ReportData->Button |= SWITCH_B;
          }
          if(strcmp(currentButton, "KEY_X")){
            ReportData->Button |= SWITCH_X;
          }
          if(strcmp(currentButton, "KEY_Y")){
            ReportData->Button |= SWITCH_Y;
          }
          if(strcmp(currentButton, "KEY_ZL")){
            ReportData->Button |= SWITCH_ZL;
          }
          if(strcmp(currentButton, "KEY_ZR")){
            ReportData->Button |= SWITCH_ZR;
          }
          if(strcmp(currentButton, "KEY_R")){
            ReportData->Button |= SWITCH_R;
          }
          if(strcmp(currentButton, "KEY_L")){
            ReportData->Button |= SWITCH_L;
          }
          if(strcmp(currentButton, "KEY_PLUS")){
            ReportData->Button |= SWITCH_PLUS;
          }
          if(strcmp(currentButton, "KEY_MINUS")){
            ReportData->Button |= SWITCH_MINUS;
          }
          if(strcmp(currentButton, "KEY_LSTICK")){
            ReportData->Button |= SWITCH_LCLICK;
          }
          if(strcmp(currentButton, "KEY_RSTICK")){
            ReportData->Button |= SWITCH_RCLICK;
          }
          if(strcmp(currentButton, "KEY_HOME")){
            ReportData->Button |= SWITCH_HOME;
          }
          if(strcmp(currentButton, "KEY_CAPTURE")){
            ReportData->Button |= SWITCH_CAPTURE;
          }
          
          //FIXME missing DPad buttons
        }
        
        ReportData->LX = strtol(lStick[0])/128;
        ReportData->LY = strtol(lStick[1])/128;
        ReportData->RX = strtol(rStick[0])/128;
        ReportData->RY = strtol(rStick[1])/128;
			}

			break;

		case CLEANUP:
			state = DONE;
			break;

		case DONE:
			#ifdef ALERT_WHEN_DONE
			portsval = ~portsval;
			PORTD = portsval; //flash LED(s) and sound buzzer if attached
			PORTB = portsval;
			_delay_ms(250);
			#endif
			return;
	}

	// Prepare to echo this report
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
	echoes = ECHOES;

}