/*===========================================================================
**
**  Application: JVC TK-F7300U Frame Capture Camera Control Software.
**
**  Module.....: USERAPP.C
**
**  Description: This application is provided to demonstrate the use of the
**               Application Programming Interface (API) developed for the 
**               JVC TK-F7300U Frame Capture Camera by Redmer Consultants
**               Enterprises.
**
**               Please note that the low-level event loop processor is
**               still active and controlled by a background process as
**               opposed to being tightly integrated with the User Interface.
**               This source code is provided for demonstration purposes 
**               only.  End user applications must declare the necessary
**               globals and process the API calls in the order specified
**               in this module for proper execution.
**
**  Environment: This application was developed/tested under the following
**               operating conditions:
**
**               Development:
**               Gateway P5-90XL Pentium based PC, 16MB RAM, 1GB PCI IDE HD
**               Compiled under Microsoft Windows NT Advanced Server 3.5
**               running DOS Emulation.
**               Microsoft C++ Optimizing Compiler Version 8.00c
**               ** Large memory model used for compilation (/AL).
**
**               Runtime:
**               Dell 486/66 80486 based PC, 16MB RAM, 1GB SCSI, ATVISTA VMX
**               DOS 6.0
**
**  Author.....: Ronald D. Redmer
**               (c) 1995 Redmer Consultants Enterprises.
**
**=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <bios.h>
#include <io.h>
#include "hrcap.h"                      // HRCAP specific typedefs & prototypes
#include "key.h"                        // HRCAP key deifinitions

extern DspCapINFO DsoCapInfo;           // Declared in API Library
extern EventRecord theEvent;            // Declared in API Library

/*===========================================================================
**
**  Function...: main
**
**  Description: This is a sample main function for controlling the
**               JVC TK-F7300U Frame capture camera.
**
**=========================================================================*/
int main()
{
int   steps=1;
long  ticks=0L;
int   liveMode=1;
Rect  r,*cr;

cr = &DspCapInfo.capRect;

// This MUST be the first function call of the program!
if( JvcInit() != 0 )            // Initialize the camera & data structs
	{
	return 2;               // return error code to DOS
	}

// Display a simple menu which mimics the HRCAP main menu.
printf("\nJVC Interface Options\n\n");
printf("I)\tIris Calibration\n");
printf("B)\tBlack Balance\n");
printf("W)\tWhite Balance\n");
printf("L)\tLive Video Mode\n");
printf("R)\tSet Capture Resolution\n");
printf("G)\tGrab Image\n");
printf("S)\tSave Image\n");
printf("ESC)\tExit Program\n\n");
printf("Press a Key...");
	
// This is a sample menu processing loop
for(;;)
	{
	GetEvent(&theEvent);                    // Obtain the event struct
	switch(theEvent.what)                   // Branch on event
		{
		case idle:
			if (theEvent.when - ticks > 2)
				steps = 1;
			break;
		case vstaevt:
			if(liveMode)
				{
				GetCapRect(cr);
				printf("Capture Rect: T-%d, B-%d, L-%d, R-%d\n",
					cr->top, cr->bottom, cr->left, cr->bottom);
				}
				break;
		case keydown:
			{
			if (theEvent.message == 0)
				steps = 1;
			else
				steps = (theEvent.when - ticks <= 2) ? 8 : 1;
			ticks = theEvent.when;
			if (theEvent.modifiers & 0x0F) 
				{       // Shift Key Pressed
				switch(theEvent.message) 
					{
					case AUP:
						JvcCropSize(0,-steps);
						break;
					case ADOWN:
						JvcCropSize(0,steps);
						break;
					case ARIGHT:
						JvcCropMove(-steps,0);
						break;
					case ALEFT:
						JvcCropMove(steps,0);
						break;
					}
				} 
			else 
				{
				switch(theEvent.message) 
					{
					case AUP:
						if(liveMode)
							Msg2GSP(CURMSG,0,steps*(-1));
							break;
					case ADOWN:
						if(liveMode)
							Msg2GSP(CURMSG,0,steps);
							break;
					case ALEFT:
						if(liveMode)
							Msg2GSP(CURMSG,steps*(-1),0);
						break;
					case ARIGHT:
						if(liveMode)
							Msg2GSP(CURMSG,steps,0);
						break;
					case 'l':
					case 'L':       
						JvcLive();
						liveMode = 1;
						break;
					case 'i':
					case 'I':       
						JvcIrisCalib();
						break;
					case 'b':
					case 'B':       
						JvcBBCalib();
						break;
					case 'w':
					case 'W':       
						JvcWBCalib();
						break;
					case 'g':
					case 'G':       
						CancelExec(1);
						JvcGrab();
						liveMode = 0;
						break;
					case 's':
					case 'S':
						{                          
						char name[20];
						printf("\nEnter a filename: ");
						JvcSaveImage(_strupr( gets(name) ));
						break;
						}
					case 'r':
					case 'R':
						{                       
						char mode[10];
						CancelExec(1);
						printf("\nEnter the Desired Capture Res. (X1,X2,X3): ");
						if( JvcSetResolution(_strupr( gets(mode) )) == 0 )
							printf("New Resolution is: %s\n", mode );
						else
							printf("Invalid Resolution value. NOT Changed!\n");
							break;
						}
					case ESC:
						{
						printf("Exiting\n");
						Term_Comm();
						return 0;
						}
					default:
						printf("Invalid Character 0x%04x\n", theEvent.message);
						break;
					}
				}
			}
		}
	}
}
