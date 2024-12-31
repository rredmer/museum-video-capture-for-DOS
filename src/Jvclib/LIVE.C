#include        "hrcap.h"
#include        "key.h"

static int gLiveMode = 0;

/*========================================================================
 *                                              Live Camera Video Mode
 *-----------------------------------------------------------------------*/
int InLiveMode( )
{
    return gLiveMode;
}

void ExitLive( )
{
    if ( gLiveMode )
        {
        CancelExec( 1 );
        gLiveMode = 0;
        }
}

int JvcCropMove( int x_inc, int y_inc )
{
    EventRecord event;
    if ( gLiveMode )
        {
        Msg2GSP( CURMSG, x_inc, y_inc );
        WaitAckGSP( &event );
        GetCapRect( &DspCapInfo.capRect );
        return 1;
        }
    return 0;
}

int JvcCropSize( int x_inc, int y_inc )
{
    EventRecord event;
    if ( gLiveMode )
        {
        Msg2GSP( SIZMSG, x_inc, y_inc );
        WaitAckGSP( &event );
        GetCapRect( &DspCapInfo.capRect );
        return 1;
        }
    return 0;
}

GetCapRect( register Rect *r )
{
    ReadGSPBuffer(( int * ) r, VistaEnv.commbuf, 4 );
}

int LiveVista( )
{
    EventRecord event;

//      Msg2GSP(CMDMSG,1,'F');
    Msg2GSP( CMDMSG, 0, 'F' );
    WaitAckGSP( &event );
    gLiveMode = 1;
}

int JvcLive( )
{
    int ret;

    if ( ( ret = CheckHR( ) ) == 0 )
        {
        if ( ( ret = InitHR( ) ) == 0 )
            {
            SetLiveLut( );             /* set live lut for focusing           */
            SetShutterSpeed( );        // was in DoLive()
            return LiveVista( );       // DoLive();
            }
        }
    else
        HRError( );

    return - 1;
}

/*** end of live.c ***/

