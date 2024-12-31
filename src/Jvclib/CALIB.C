/*
**      calib.c :
*/

#include        <math.h>
#include        "hrcap.h"
#include        "key.h"
#include "stdfox.h"

#define         HitAnyKey()             keyread()

extern EventRecord theEvent;

static Boolean isNewCalib;

int DoBlackBalance( );
int DoWhiteBalance( );
int ProcSaveSetup( );

int SelectFocusArea( char *s, Rect *rp )
{
    Point pm;
    int steps = 1;
    long ticks = 0L;
    Rect r;
    static Boolean firstTime = true;
    static Rect cr =
                {
                ( PalResH - 32 ) / 2, ( PalResV - 32 ) / 2,
                ( PalResH + 32 ) / 2, ( PalResV + 32 ) / 2
                };

    SetRect( &r, 0, 0, PalResH * DspCapInfo.capMode, PalResV * DspCapInfo.capMode );

    if ( firstTime == true )
        {
        cr.left *= DspCapInfo.capMode;
        cr.top *= DspCapInfo.capMode;
        cr.right *= DspCapInfo.capMode;
        cr.bottom *= DspCapInfo.capMode;
        firstTime = false;
        }

    for (;; )
        {
        GetEvent( &theEvent );

        switch ( theEvent.what )
            {
            case idle :
                if ( theEvent.when - ticks > 2 )
                    steps = 1;
                break;
            case keydown :
                if ( theEvent.message == 0 )
                    steps = 1;
                else
                    steps = ( theEvent.when - ticks <= 2 ) ? 8 : 1;
                ticks = theEvent.when;
                if ( theEvent.modifiers & 0x0F )
                    {
                    switch ( theEvent.message )
                        {
                        case AUP :
                            Msg2GSP( SIZMSG, 0, steps * (-1 ) );
                            break;
                        case ADOWN :
                            Msg2GSP( SIZMSG, 0, steps );
                            break;
                        case ARIGHT :
                            Msg2GSP( SIZMSG, steps * (-1 ), 0 );
                            break;
                        case ALEFT :
                            Msg2GSP( SIZMSG, steps, 0 );
                            break;
                        }
                    }
                else
                    {
                    switch ( theEvent.message )
                        {
                        case AUP :
                            Msg2GSP( CURMSG, 0, steps * (-1 ) );
                            break;
                        case ADOWN :
                            Msg2GSP( CURMSG, 0, steps );
                            break;
                        case ALEFT :
                            Msg2GSP( CURMSG, steps * (-1 ), 0 );
                            break;
                        case ARIGHT :
                            Msg2GSP( CURMSG, steps, 0 );
                            break;
//                              case ESC:
//                                      clrbox(&MsgBox);
                        case CR :
                            Msg2GSP( CANMSG, 0, 0 );/* stop GSP               */
                            *rp = cr;
                            return theEvent.message;
                        }
                    }
                break;
            case vstaevt :
                GetCapRect( &cr );
                break;
            }
        }
}

static int GetAverage( Rect *rp, int *r, int *g, int *b,
               float *rrms, float *grms, float *brms )
{
    int v, h, height, width;
    long totalpix;
    Pix32 *bp;
    unsigned long addr;
    float rf, gf, bf, tr, tg, tb;

    height = rp->bottom - rp->top;
    width = ( rp->right - rp->left ) / 3;
    if ( ( bp = ( Pix32 * ) xmalloc( width * sizeof ( Pix32 ) ) ) == 0 )
        return - 1;

    tr = tg = tb = 0.0;
    addr = VRAMEND - ( rp->top + 1 ) * PITCH + rp->left / 3 * 32;
    totalpix = height * width;

    for ( v = 0; v < height; v++ )
        {
        ReadGSPBuffer(( int * ) bp, addr, width * sizeof ( Pix32 ) / sizeof ( int ) );
        for ( h = 0; h < width; h++ )
            {
            tr += bp[h].r;
            tg += bp[h].g;
            tb += bp[h].b;
            }
        addr -= PITCH;
        }
    rf = tr / totalpix;
    gf = tg / totalpix;
    bf = tb / totalpix;

    *r = ( int ) rf;
    *g = ( int ) gf;
    *b = ( int ) bf;

    /* check factory setting mode */
    if ( 1 )
        {                              // CalibProcs[4].enabled) {
        tr = tg = tb = 0.0;
        addr = VRAMEND - ( rp->top + 1 ) * PITCH + rp->left / 3 * 32;
        for ( v = 0; v < height; v++ )
            {
            ReadGSPBuffer(( int * ) bp, addr, width * sizeof ( Pix32 ) / sizeof ( int ) );
            for ( h = 0; h < width; h++ )
                {
                tr += ( bp[h].r - rf ) * (bp[h].r - rf );
                tg += ( bp[h].g - gf ) * (bp[h].g - gf );
                tb += ( bp[h].b - bf ) * (bp[h].b - bf );
                }
            addr -= PITCH;
            }
        *rrms = sqrt(( double ) ( tr / totalpix ) );
        *grms = sqrt(( double ) ( tg / totalpix ) );
        *brms = sqrt(( double ) ( tb / totalpix ) );
        }

    xfree( bp );
    return 0;
}

static int WriteEnvFile( char *filename )
{
    int fh;

    fh = xopen( filename, O_BINARY | O_CREAT | O_WRONLY );// ,S_IWRITE|S_IREAD);
    if ( fh == -1 )
        {
        ErrorMessage( "ENV file open error" );
        return - 1;
        }
    if ( xwrite( fh, ( char * ) &HRSetup, sizeof ( HRSetup ) ) != sizeof ( HRSetup ) )
        {
        ErrorMessage( "ENV file write error" );
        xclose( fh );
        return - 1;
        }
    xclose( fh );
    isNewCalib = false;
    return 0;
}

static int ProcSaveSetup( )
{
    char filename[64];

//      if (GetFileName(filename,HREnvFile) == CR) {
    if ( WriteEnvFile( HREnvFile ) == 0 )
        Complete( );
//      } else
//              clrbox(&MsgBox);

    return - 1;
}

#define TARGET  3                      /* 1% range(256/100)                   */

int DoBlackBalance( )
{
    static struct
        {
        Byte cmd;
        SETUP set;
        } SetCmd =
        {
        SetupCmd,
            {
            0, 0, 0
            }
        };
    int retry;
    int r, g, b;
    float rrms, grms, brms;
    Rect rec;

    SetIrisLut( );

    Msg2GSP( CMDMSG, 2, 'F' );
    WaitAckGSP( &theEvent );

    if ( SelectFocusArea( "Close lens and press ENTER", &rec ) == ESC )
        return - 1;

    SetCapLut( );
    LiveVista( );

    SetCmd.set.setR = 128;             /* default value (2.5V)                */
    SetCmd.set.setG = 128;
    SetCmd.set.setB = 128;
    retry = 64;

    do
        {
/*
                if (kbhit())
                        if (getch() == ESC) {
                                clrbox(&MsgBox);
                                return -1;
                        }
*/
        Msg2HR(( char * ) &SetCmd, sizeof ( SetCmd ), 18 );
        Delay( 10 );

        Msg2GSP( CMDMSG, 1, 'G' );
        WaitAckGSP( &theEvent );

        GetAverage( &rec, &r, &g, &b, &rrms, &grms, &brms );
//              DrawRGB(r,g,b,rrms,grms,brms);

        if ( r > TARGET ) SetCmd.set.setR -= 2;
        if ( g > TARGET ) SetCmd.set.setG -= 2;
        if ( b > TARGET ) SetCmd.set.setB -= 2;

        retry--;

        }
    while ( retry && ( ( r > TARGET ) || ( g > TARGET ) || ( b > TARGET ) ) );

    if ( retry == 0 )
        {
        ErrorMessage( "Calib err. Close Lens correctly." );
        return - 1;
        }

    HRSetup.setup[HRSetup.gamma][HRSetup.gainNum] = SetCmd.set;

    Complete( );
    isNewCalib = true;
    return - 1;
}

#define         AGCMIN          81*256U/* 1.6V                                */
#define         AGCMAX          226*256U/* 4.4V                               */
#define         STRTRNG         4*256

/* The white balance calibration changes the RED and BLUE gain value, with
        the base GREEN gain value */
int DoWhiteBalance( )
{
    static struct
        {
        Byte cmd;
        RGBGain gain;
        } WhiteCmd =
        {
        GainCmd,
            {
            0, 0, 0, 0, 0, 0
            }
        };
    int retry;
    int r, g, b;
    float rrms, grms, brms;
    long rval, bval;
    long rangeR, rangeB;
    int dr, db, sign;
    int dirR, dirB;
    Rect rec;

    SetIrisLut( );

    Msg2GSP( CMDMSG, 2, 'F' );
    WaitAckGSP( &theEvent );

    if ( SelectFocusArea( "Place white object and press ENTER", &rec ) == ESC )
        return - 1;

    SetCapLut( );
    LiveVista( );

//      DrawRGBItems();

    /* grab one frame and defines the first direction */
    WhiteCmd.gain = HRSetup.gain[HRSetup.gamma][HRSetup.gainNum];
    Msg2HR(( char * ) &WhiteCmd, sizeof ( WhiteCmd ), 18 );
    Delay( 10 );
    Msg2GSP( CMDMSG, 1, 'G' );
    WaitAckGSP( &theEvent );
    GetAverage( &rec, &r, &g, &b, &rrms, &grms, &brms );
    dirR = r - g < 0 ? -1 : 1;
    dirB = b - g < 0 ? -1 : 1;

    rval = WhiteCmd.gain.gainR.high * 256 + WhiteCmd.gain.gainR.low;
    bval = WhiteCmd.gain.gainB.high * 256 + WhiteCmd.gain.gainB.low;
    retry = 64;                        /* retry counter                       */
    rangeR = STRTRNG;                  /* starting control range              */
    rangeB = STRTRNG;

    do
        {
/*
                if (kbhit()) {
                        if (getch() == ESC) {
                                clrbox(&MsgBox);
                                return -1;
                        }
                }
*/
        WhiteCmd.gain.gainR.high = rval / 256;
        WhiteCmd.gain.gainR.low = rval % 256;
        WhiteCmd.gain.gainB.high = bval / 256;
/*              WhiteCmd.gain.gainB.low =  bval % 256; */
        /*** for the bug of v.1.00 camera (product sample version),  ***/
        /*** the LSB should be zero.                                                             ***/
        WhiteCmd.gain.gainB.low = ( bval % 256 ) & 0xFE;
        Msg2HR(( char * ) &WhiteCmd, sizeof ( WhiteCmd ), 18 );
        Delay( 10 );

        Msg2GSP( CMDMSG, 1, 'G' );
        WaitAckGSP( &theEvent );

        GetAverage( &rec, &r, &g, &b, &rrms, &grms, &brms );
//              DrawRGB(r,g,b,rrms,grms,brms);

        sign = r - g < 0 ? -1 : 1;
        dr = abs( r - g );
        if ( dr > TARGET )
            {
            rval += rangeR * sign;
            if ( dirR * sign < 0 )
                {
                rangeR = rangeR / 2;
                }
            dirR = sign;
            }

        sign = b - g < 0 ? -1 : 1;
        db = abs( b - g );
        if ( db > TARGET )
            {
            bval += rangeB * sign;
            if ( dirB * sign < 0 )
                {
                rangeB = rangeB / 2;
                }
            dirB = sign;
            }

        if ( ( rval < AGCMIN ) || ( rval > AGCMAX ) ||
                ( bval < AGCMIN ) || ( bval > AGCMAX ) )
            {
            ErrorMessage( "Calib error. Can not control AGC." );
            return - 1;
            }

        retry--;

        }
    while ( retry && ( ( dr > TARGET ) || ( db > TARGET ) ) );

    HRSetup.gain[HRSetup.gamma][HRSetup.gainNum] = WhiteCmd.gain;

    Complete( );
    isNewCalib = true;
    return - 1;
}


/*** end of calib.c ***/
