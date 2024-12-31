/*
**
*/

#ifndef __MOUSE__
    #define __MOUSE__

#ifndef __HRTYPES__
#include        "hrtypes.h"
#endif

/*** from mouse.c ***/
    extern InitMouse( Rect *r );
    extern Boolean InspectMouse( );
    extern InspectMouse( Point *mouse );
    extern GetMouse( Point *mouse );
    extern MoveMouse( int h, int v, Point *mouse );
    extern SetMouse( int h, int v );
    extern int Button( );
    extern int StillDown( );
    extern WaitMouseUp( );

    extern int _InitMouse( int *button, Rect *r );
    extern _GetMickey( Point *mouse );
    extern int _button( int number );
    extern int _btnrls( int number );

#endif

/*** end of mouse.h ***/
