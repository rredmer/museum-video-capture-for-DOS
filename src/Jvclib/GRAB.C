/*
**      grab.c :
*/

#include        "hrcap.h"
#include        "key.h"
#include "stdfox.h"

WaitAckGSP( EventRecord *e )
{
    do
        {
        GetEvent( e );
        }
    while ( e->what != vstaevt );
}

int WaitAckHR( )
{
    int ch;
    long coldTime;

    coldTime = getticks( );
    do
        {
        ch = Read_Modem( );
        if ( ch == ACK )
            return 0;
        else if ( ch == NAK )
            return - 1;
        }
    while ( getticks( ) - coldTime < 91L );/* 5 sec timeout                   */
    return - 1;
}

int DoGrab( )
{
    static struct
        {
        Byte cmd;
        Byte mode;
        } Motor =
        {
        MotorCmd, 0
        };
    EventRecord event;

    int i, ch, *px, *py;
    int buildH, imageH;
    int ret = 0;
    int grabTimes;
    long time;

    switch ( DspCapInfo.capMode )
        {
        case X1 : Motor.mode = 0;
            grabTimes = 3;
            break;
        case X2 : Motor.mode = 1;
            grabTimes = 12;
            break;
        case X3 : Motor.mode = 2;
            grabTimes = 27;
            break;
        case X6 : Motor.mode = 3;
            grabTimes = 108;
            break;
        }

    Msg2GSP( CMDMSG, 0, 'G' );
    WaitAckGSP( &event );

    Delay( 10 );             /* about 500ms delay for stable camera genlocking*/

    SetShutterSpeed( );

    if ( Msg2HR(( Byte * ) &Motor, sizeof ( Motor ), 18 ) == NAK )
        {
        ErrorMessage( "Unable Genlocking" );
        Msg2GSP( CANMSG, 0, 0 );
        return 0;
        }

    for ( i = 0; i < grabTimes; i++ )
        {
        Write_Modem( ENQ );
        if ( WaitAckHR( ) == -1 )
            {
            Msg2GSP( CANMSG, 0, 0 );
            return - 1;
            }
        Msg2GSP( CMDMSG, 0, 0 );
        }
    Write_Modem( ENQ );
    if ( WaitAckHR( ) == -1 )
        {
        Msg2GSP( CANMSG, 0, 0 );
        return - 1;
        }

    #ifndef FOXPRO
        printf( "Build Capture Image ..." );
    #endif

    imageH = DspCapInfo.capRect.right - DspCapInfo.capRect.left;
    time = getticks( );
    do
        {
        GetEvent( &event );
        if ( event.when - time > 18 )
            {
            time = event.when;
            buildH = ReadGSPWord( VistaEnv.commbuf );
//                      PutPercent(buildH,imageH);
            }
        }
    while ( event.what != vstaevt || event.message != ETX );

    DspCapInfo.imageType = TGAType2;

    #ifndef FOXPRO
        printf( " Done\n" );
    #endif

    return 0;
}


/*** end of grab.c ***/
