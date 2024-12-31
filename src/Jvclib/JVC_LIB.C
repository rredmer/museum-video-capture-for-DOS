#include <math.h>
#include <dos.h>
#include <direct.h>
#include <errno.h>

#include "hrcap.h"
#include "mouse.h"
#include "key.h"
#include "term.h"
#include "stdfox.h"

/*====================================
 *      Constants
 *-----------------------------------*/
#define         GSPOUTFILE      "HRCAP.OUT"
#define         HRCFGFILE       "HRCAP.CFG"
#define         HRENVFILE       "HRCAP.ENV"
#define         HRCOLORFILE     "HRCAP.COL"

/*====================================
 *      Structure Definitions
 *-----------------------------------*/
VistaENV VistaEnv =
                {                      /* AtVista environment                 */
                    0x228,             /* io base                             */
                    4,                 /* memory                              */
                    VRAMSTR,           /* mem start                           */
                    VRAMEND,           /* vram end                            */
                    0L,                /* communication address               */
                };

/*====================================
 *      Global/Static Variables
 *-----------------------------------*/
int CommPort = COM1;
int CommBaud = 9600;

EventRecord theEvent;

static int curDrive;                   /* current working drive               */
static char dirBuff[128];              /* current working directory           */

static unsigned long lutBank[] =
                     {
                         LUT0, LUT1, LUT2, LUT3, LUT4, LUT5, LUT6, LUT7
                     };

char HRPath[80];                       /* HRCAP directory path name           */
char DspLutFile[80];                   /* display LUT file name               */
char CapLutFile[80];                   /* capture LUT file name               */
char GspOutFile[80];                   /* .OUT file name                      */
char HRCfgFile[80];                    /* .CFG file name                      */
char HREnvFile[80];                    /* .ENV file name                      */
char ColorBarFile[80];                 /* .COL file name                      */
char SetupPath[80];

/* LUT data area */
LUT CapLutTbl[256];                    /* capture lut data                    */
LUT DspLutTbl[256];                    /* display lut data                    */
static LUT IrsLutTbl[256];             /* iris lut data                       */
static LUT LiveLutTbl[256];            /* live lut data                       */
static LUT LinearLutTbl[256];          /* linear lut data                     */

DspCapINFO DspCapInfo =
                {                      /* display/capture status information  */
                    slave,             /* genlock mode                        */
                    TGAType2,          /* image type                          */
                    16,                /* pixSize                             */
                    X1,                /* capture mode                        */
                    PalCapH,           /* horzRes                             */
                    PalCapV,           /* vertRes                             */
                    StdPal,            /* display mode                        */
                    PalResH,           /* horzCrtRes                          */
                    PalResV,           /* vertCrtRes                          */
                        {
                        0L, 0L, 0L,    /* GPort                               */
                            {
                            0, 0, 0, 0
                            }
                        },
                        {
                        0, 0, 32767, 32767
                        }
                    ,                  /* capRect                             */
                        {
                        0, 0, 0, 0
                        }
                    ,                  /* dspRect                             */
                        {
                        0L, 0L, 0L,    /* dspPort                             */
                            {
                            0, 0, 0, 0
                            }
                        },
                        {
                        0L, 0L, 0L,    /* devPort                             */
                            {
                            0, 0, 0, 0
                            }
                        },
                };

HRSETUP HRSetup =
        {
            1,                         /* genlock(master)                     */
            0,                         /* smaple(Vista)                       */
            0,                         /* gamma                               */
            0,
            0,                         /* phase(through)                      */
            1,                         /* gain table number                   */
                {
                    {
                    0, 0, 0
                    }
                ,                      /* setup                               */
                    {
                    0, 0, 0
                    }
                },

                {                      /* white balance gain                  */
                    {
                        {
                        0, 0, 0, 0, 0, 0
                        },
                        {
                        184, 0, 184, 0, 184, 0
                        },
                        {
                        0, 0, 0, 0, 0, 0
                        },
                        {
                        0, 0, 0, 0, 0, 0
                        }
                    },

                    {
                        {
                        0, 0, 0, 0, 0, 0
                        },
                        {
                        184, 0, 184, 0, 184, 0
                        },
                        {
                        0, 0, 0, 0, 0, 0
                        },
                        {
                        0, 0, 0, 0, 0, 0
                        }
                    }
                }
        };

int capRegs[8] =
        {                              /* 34010 video registers               */
            207,
            69,
            439,
            453,
            2,
            17,
            310,
            312
        };

Byte shutterSpeed[2] =
                {
                    ShutterCmd, 0
                };

/*========================================================================
 *                                      General Support Routines
 *-----------------------------------------------------------------------*/
GetEvent( EventRecord *event )
{
    Point mouse;
    int button, ch, msg;

    if ( ( msg = GetGSPMsg( ) ) & 0x0080 )
        {
        event->what = vstaevt;
        event->keycode = 0;
        event->message = ( msg >> 4 ) & 0x07;
        SetGSPMsg( 0 );
        }
    else if ( keyhit( ) )
        {
        ch = keyread( );
        event->keycode = ch;
        event->message = ( ch & 0x00FF ) ? toupper( ch & 0x00FF ) : ch;
        event->what = keydown;
        event->modifiers = shftstat( );
        }
    else
        {
        event->what = idle;
        }
    event->when = getticks( );
}

Delay( int cnt )
{                               /* one tick is equal to about 55ms (1/18.2 ms)*/
    long current;

    if ( cnt == 0 )
        return 0;

    current = getticks( );   // getticks could be replaced with bios_timeofday()
    while ( ( int ) ( getticks( ) - current ) < cnt );
    return 0;
}

SetRect( Rect *r, int left, int top, int right, int bottom )
{
    r->left = left;
    r->top = top;
    r->right = right;
    r->bottom = bottom;
}

HRError( )
{
    ErrorMessage( "HR-Camera not connected" );
}

/*========================================================================
 *                                      Hardware Communications Routines
 *-----------------------------------------------------------------------*/

Ack2GSP( )
{
    SetGSPMsg( 0 );
}

MsgFromHR( Byte *s, int len )
{
    int ch;

    while ( len-- )
        {
        while ( ( ch = Read_Modem( ) ) < 0 );
        *s++= ( Byte ) ch;
        }
}

int CheckGSPMsg( )
{
    int msg;

    msg = GetGSPMsg( );
    if ( msg  & INTOUT )
        return msg & MSGOUT;
    else
        return 0;
}

int MsgFromGSP( unsigned *s )
{
    int len, msg;

    do
        {
        msg = GetGSPMsg( );
        }
    while ( ( msg & INTOUT ) == 0 );   /* wait until message request          */
    len = ReadGSPWord( VistaEnv.commbuf );
    ReadGSPBuffer( s, VistaEnv.commbuf + 16, len );
    SetGSPMsg( 0 );                    /* clear intout flag                   */
    return msg & MSGOUT;
}

Msg2GSP( int cmd, int par1, int par2 )
{
    long msg;

    msg = ( ( unsigned long ) par1 << 16 ) + ( par2 & 0xFFFF );
    SetGSPAddr( msg );
    SetGSPMsg( INTIN + cmd );
    while ( GetGSPMsg( ) & INTIN );
}

int Msg2HR( Byte *s, int len, int ticks )
{
    int ch;

    while ( len-- )
        Write_Modem(( int ) *s++ );

    do
        {
        if ( ( ch = Read_Modem( ) ) > 0 )
            return ch;
        Delay( 1 );                    /* delay 55ms                          */
        }
    while ( ticks-- );
    return - 1;
}

int CheckHR( )
{
    int retry, ch;

    for ( retry = 0; retry < 3; retry++ )
        {
        Write_Modem( NopCmd );
        Delay( 9 );                    /* delay about 500ms                   */
        if ( ( ch = Read_Modem( ) ) > 0 )
            {
            if ( ch == ACK )
                return 0;
            }
        }
    return - 1;
}

int InitHR( )
{
    struct
        {
        Byte cmd;
        Byte genlock;                  /* genlock (master or slave)           */
        Byte sample;             /* sampling clock (942fH/908fH - Targa/Vista)*/
        Byte gamma;                    /* gamma and knee on/off               */
        Byte phase;                 /* phase control delay 0-15 (0:through HD)*/
        SETUP setup;                   /* RGB setup                           */
        RGBGain gain;                  /* RGB gain                            */
        } init;

    init.cmd = InitCmd;
    init.genlock = HRSetup.genlock;
    init.sample = HRSetup.sample;
    init.gamma = HRSetup.gamma + ( HRSetup.knee << 1 );
    init.phase = HRSetup.phase;
    init.setup = HRSetup.setup[HRSetup.gamma][HRSetup.gainNum];
    init.gain = HRSetup.gain[HRSetup.gamma][HRSetup.gainNum];

    if ( Msg2HR(( Byte * ) &init, sizeof ( init ), 18 ) != ACK )
        return - 1;

    return 0;
}

/* Initial shutter speed */
int SetShutterSpeed( )
{
    if ( Msg2HR( shutterSpeed, 2, 18 ) != ACK )
        return - 1;
}

/*========================================================================
 *                                      LUT Creation and Installation Routines
 *-----------------------------------------------------------------------*/
static SetLut( int bank, LUT *lut )
{
    LUT *lp;
    int i, j, *bp, *buf;

    if ( ( buf = ( int * ) xmalloc( 4096 ) ) == 0 )
        return 0;

    bp = buf;
    for ( i = 0; i < 2; i++ )
        {                              /* create 1024x2 lut table             */
        for ( j = 0, lp = lut; j < 256; j++, lp++ )
            {
            *bp++= ( bank << 8 ) + j;
            *bp++= 0;
            *bp++= ( lp->RGBA.g << 8 ) + lp->RGBA.b;
            *bp++= lp->RGBA.r;
            }
        }

    /* bump row table data */
    WriteGSPBuffer( buf, lutBank[bank], 2048 );

    xfree( buf );
    return 1;
}

static int LoadLutFile( LUT *LutTb, char *file )
{
    int fh;

    if ( ( fh = xopen( file, O_RDONLY | O_BINARY ) ) == -1 )
        return - 1;                    /* open error                          */

    if ( xread( fh, ( char * ) LutTb, sizeof ( LUT ) * 256 ) != sizeof ( LUT ) * 256 )
        return - 2;                    /* read error                          */

    xclose( fh );
    return 0;
}

static SetLutAllLinear( )
{
    int bank;

    for ( bank = 0; bank < 8; bank++ )
        SetLut( bank, LinearLutTbl );
}

static SetDspLut( )
{
    return SetLut( 6, DspLutTbl );     /* display lut = No.6 LUT              */
}

SetLiveLut( )
{
    return SetLut( 7, LiveLutTbl );    /* capture lut = No.7 LUT              */
}

SetCapLut( )
{
    return SetLut( 7, CapLutTbl );
}

SetIrisLut( )
{
    return SetLut( 7, IrsLutTbl );
}

SetLinearLut( )
{
    return SetLut( 7, LinearLutTbl );
}

SetCapLutLinear( )
{
    return SetLut( 7, LinearLutTbl );
}

static MakeLinearLut( LUT *LutTb )
{
    int i;

    for ( i = 0; i < 256; i++ )
        {
        LutTb->RGBA.r = LutTb->RGBA.g = LutTb->RGBA.b = i;
        LutTb->RGBA.a = 0;
        LutTb++;
        }
}

static MakeIrisLut( )
{
    int i;

    MakeLinearLut( IrsLutTbl );

    for ( i = 256 * 95 / 100; i < 256; i++ )
        {
        IrsLutTbl[i].RGBA.r = 255;
        IrsLutTbl[i].RGBA.g = 0;
        IrsLutTbl[i].RGBA.b = 0;
        }
}

static MakeLiveLut( )
{
    int i;

    for ( i = 0; i < 256; i++ )
        {
        LiveLutTbl[i].RGBA.r = DspLutTbl[CapLutTbl[i].RGBA.r].RGBA.r;
        LiveLutTbl[i].RGBA.g = DspLutTbl[CapLutTbl[i].RGBA.g].RGBA.g;
        LiveLutTbl[i].RGBA.b = DspLutTbl[CapLutTbl[i].RGBA.b].RGBA.b;
        LiveLutTbl[i].RGBA.a = 0;
        }
}

/*========================================================================
 *                                      General Initialization Routines
 *-----------------------------------------------------------------------*/
static int LoadEnvFile( )
{
    int fh;

    if ( ( fh = xopen( HREnvFile, O_RDONLY | O_BINARY ) ) == -1 )
        return - 1;
    if ( xread( fh, ( char * ) &HRSetup, sizeof ( HRSetup ) ) != sizeof ( HRSetup ) )
        return - 2;

    xclose( fh );
    return 0;
}

static Boolean SetVistaEnv( )
{
    unsigned long ramAddr, data;

    ramAddr = 0xFF800000L;
    do
        {
        WriteGSPLong( ramAddr, 0x5A5A5A5AL );
        data = ReadGSPLong( ramAddr );
        if ( data != 0x5A5A5A5AL )
            break;

        ramAddr -= 0x00800000L;        /* decrement 1MB Vista memory          */

        }
    while ( ramAddr >= 0xF9000000L );

    if ( ramAddr == 0xFF800000L )      /* not found Vista board               */
        return false;

    ramAddr += 0x00800000L;
    VistaEnv.vistaMem = ( int ) ( ( ( long ) ramAddr >> 24 ) * (-2 ) );
    VistaEnv.vistaEnd = 0xFFFB0000L;
    VistaEnv.vistaStr = ramAddr;
    return true;
}

static SetCapRes( )
{
    int width, height, psize;
    unsigned long total;
    int MaxH, MaxV;

    if ( VistaEnv.vistaMem == 4 )
        {
        switch ( DspCapInfo.capMode )
            {
            case X1 :
                if ( DspCapInfo.dspMode == HiRes )
                    Abort( "Invalid display mode."
                        " Set StdPal or NonIntPal display mode." );
                DspCapInfo.horzRes = PalCapH;
                DspCapInfo.vertRes = PalCapV;
                break;
            case X2 :
                if ( DspCapInfo.pixSize == 32 )
                    {
                    DspCapInfo.horzRes = HiresH;
                    DspCapInfo.vertRes = HiresV;
                    }
                else
                    {
                    DspCapInfo.horzRes = PalCapH * 2;
                    DspCapInfo.vertRes = MaxV4M;
                    }
                break;

            case X3 :
            case X6 :
                Abort( "HRCAP needs VMX memory in x3 capture mode" );
                break;

            }
        }
    else
        {
        psize = ( DspCapInfo.pixSize == 32 ) ? 24 : 16;
        total = ( VistaEnv.vistaEnd - VistaEnv.vistaStr - PITCH * 20L -
            PITCH * (unsigned long ) DspCapInfo.vertCrtRes ) / ( unsigned long ) psize;
        width = ( ( int ) sqrt(( double ) total * 4 / 3 ) >> 1 ) << 1;
        height = width * 3 / 4;

        DspCapInfo.horzRes = min( PalCapH * DspCapInfo.capMode, width );
        DspCapInfo.vertRes = min( PalCapV * DspCapInfo.capMode, height );
        }

    SetRect( &DspCapInfo.dspRect,
        0, 0, DspCapInfo.horzCrtRes, DspCapInfo.vertCrtRes );
}

int JvcInitEnv( char *setup )
{
    strcpy( SetupPath, setup );
        #ifdef FOXPRO
                SaveFD();
                SaveMem();
        #endif
}

int JvcInit( )
{
    int i, ret;
    Boolean vistaExist, mouseExist;
    unsigned long comm;
    char string[80];
    Rect r;

#ifdef V1
        _dos_getdrive(&curDrive);      /* get current drive                   */
        getcwd(dirBuff,127);           /* get current working directory       */

/* init system file names */
/*
#ifndef FOXPRO
   printf("CurrDir = \"%s\"\n", dirBuff);
#else
        SaveFD();
        SaveMem();
#endif
*/
        strcpy(HRPath,dirBuff);
    #else
        strcpy( HRPath, SetupPath );
    #endif
    i = strlen( HRPath );
    if ( HRPath[i - 1] != '\\' )
        {
        HRPath[i++] = '\\';
        HRPath[i] = '\0';
        }

    strcpy( GspOutFile, HRPath );
    strcat( GspOutFile, GSPOUTFILE );

    strcpy( HRCfgFile, HRPath );
    strcat( HRCfgFile, HRCFGFILE );

    strcpy( HREnvFile, HRPath );
    strcat( HREnvFile, HRENVFILE );

    strcpy( ColorBarFile, HRPath );
    strcat( ColorBarFile, HRCOLORFILE );

    DspLutFile[0] = '\0';
    CapLutFile[0] = '\0';

/* load the environment values in HR-CAMERA */
    if ( LoadEnvFile( ) < 0 )
        {
        ErrorMessage( "File error : %s\007\n", HREnvFile );
        return - 1;
        }

/* get HRCAP parameters from configuration file */
    if ( GetConfig( HRCfgFile ) == -1 )
        return - 1;

/* Initialize AtVista */
    /* check AtVista system */
    vistaExist = SetVistaEnv( );
    if ( vistaExist == false )
        Abort( "ATVista Board not exist" );

    if ( VistaEnv.vistaMem < 4 )
        Abort( "HRCAP needs 4MB AtVista board" );

/* if calibration mode on, make AtVista to the standard PAL/32  */
/*
        if (MainProcs[8].enabled == true) {
                DspCapInfo.dspMode = StdPal;
                DspCapInfo.horzCrtRes = PalResH;
                DspCapInfo.vertCrtRes = PalResV;
                DspCapInfo.capMode = X1;
                DspCapInfo.pixSize = 32;
                shutterSpeed[1] = 0;
        }
*/
/* set capture and display resolution to match memory size */
    SetCapRes( );

/* print AtVista memory status */\

    #ifndef FOXPRO
        printf( "        AtVista : TotalMem=%d(MB), RamStart=%08lX\n",
            VistaEnv.vistaMem, VistaEnv.vistaStr );

/* print video sync mode */
        printf( "        Video Sync : %s\n",
            ( DspCapInfo.genlock == master ) ? "Master" : "Slave" );

/* Check PC/AT CRT mode */
//      CharAttr = (chkcrt()) ? ORANGE : HILITE;
    #endif

/* create LUT table data */
    MakeIrisLut( );                    /* iris lut                            */
    MakeLinearLut( LinearLutTbl );     /* linear lut                          */

    if ( DspLutFile[0] == '\0' )
        {                              /* display lut                         */
        MakeLinearLut( DspLutTbl );
        printf( "        Dispaly LUT : Linear lut\n" );
        }
    else
        {
        if ( LoadLutFile( DspLutTbl, DspLutFile ) < 0 )
            {
            ErrorMessage( "File error : %s\007\n", DspLutFile );
            return - 1;
            }
        #ifndef FOXPRO
            printf( "        Display LUT File : %s\n", DspLutFile );
        #endif
        }

    if ( CapLutFile[0] == '\0' )
        {                              /* capture lut                         */
        MakeLinearLut( CapLutTbl );
        printf( "        Capture LUT : Linear lut\n" );
        }
    else
        {
        if ( LoadLutFile( CapLutTbl, CapLutFile ) < 0 )
            {
            ErrorMessage( "File error : %s\007\n", CapLutFile );
            return - 1;
            }
        #ifndef FOXPRO
            printf( "        Capture LUT File : %s\n", CapLutFile );
        #endif
        }

    MakeLiveLut( );

/* load the GSP program into AtVista memory */
    if ( ret = LoadCoff( GspOutFile ) )
        {
        ErrorMessage( "Read Error or Bad Format File : %s\007\n", GspOutFile );
        return - 1;
        }

    #ifndef FOXPRO
        printf( "        Load GSP Prog : %s\n", GspOutFile );
    #endif

    /* get commnucation buffer address */
    while ( ( GetGSPMsg( ) & INTOUT ) == 0 );/* wait until wake up GSP        */
    VistaEnv.commbuf = GetCommAddr( ); /* get communcation buffer address     */
    comm = VistaEnv.commbuf;
    WriteGSPBuffer(( unsigned int * ) &DspCapInfo, comm,
        sizeof ( DspCapInfo ) / sizeof ( int ) );
    comm += ( sizeof ( DspCapInfo ) / sizeof ( int ) ) * 16;
    WriteGSPBuffer(( unsigned int * ) &VistaEnv, comm,
        sizeof ( VistaEnv ) / sizeof ( int ) );
    comm += ( sizeof ( VistaEnv ) / sizeof ( int ) ) * 16;
    WriteGSPBuffer( &capRegs[0], comm, 8 );
    Ack2GSP( );                        /* clear GSP message bit               */

    while ( ( GetGSPMsg( ) & INTOUT ) == 0 );
    ReadGSPBuffer(( unsigned int * ) &DspCapInfo,
        VistaEnv.commbuf, sizeof ( DspCapInfo ) / sizeof ( int ) );

    SetLutAllLinear( );                /* set 0-7 lut                         */
    SetDspLut( );                      /* set display lut                     */
    Ack2GSP( );

/* Initialize the serial device for HR-Camera interface */
    Init_Comm( CommPort, CommBaud );

    #ifndef FOXPRO
        printf( "        HR-Camera Serial Port : %s  %dbps\n",
            ( CommPort == COM1 ) ? "COM1" : "COM2", CommBaud );
    #endif

    JvcLive( );

    return 0;
}

/*========================================================================
 *                                                      Calibration Routines
 *-----------------------------------------------------------------------*/
int JvcIrisCalib( HrProc *proc )
{
    if ( CheckHR( ) == 0 )
        {
        if ( InitHR( ) == 0 )
            {
            SetIrisLut( );             /* set live lut for iris               */
            SetShutterSpeed( );        // was in DoLive()
            return LiveVista( );       // DoLive();
            }
        }
    else
        HRError( );

    return - 1;
}

int JvcWBCalib( )
{
    if ( CheckHR( ) == 0 )
        {
        if ( InitHR( ) == 0 )
            {
            SetLinearLut( );
            DoWhiteBalance( );
            return 0;
            }
        }
    else
        HRError( );

    return - 1;
}

int JvcBBCalib( )
{
    if ( CheckHR( ) == 0 )
        {
        if ( InitHR( ) == 0 )
            {
            SetLinearLut( );
            DoBlackBalance( );
            return 0;
            }
        }
    else
        HRError( );

    return - 1;
}

/*========================================================================
 *                                              Capture Area Box Operations
 *-----------------------------------------------------------------------*/

/*========================================================================
 *                                                      Image Capture
 *-----------------------------------------------------------------------*/
int JvcSetResolution( char *res )
{
    int ret;
    unsigned long comm;

    ExitLive( );

    if ( CfgCapMode( res ) == -1 )
        return - 1;

    SetCapRes( );

/* load the GSP program into AtVista memory */
    if ( ret = LoadCoff( GspOutFile ) )
        {
        ErrorMessage( "Read Error or Bad Format File : %s\007\n", GspOutFile );
        return - 1;
        }

    /* get communication buffer address */
    while ( ( GetGSPMsg( ) & INTOUT ) == 0 );/* wait until wake up GSP        */
    VistaEnv.commbuf = GetCommAddr( ); /* get communcation buffer address     */
    comm = VistaEnv.commbuf;
    WriteGSPBuffer(( unsigned int * ) &DspCapInfo, comm,
        sizeof ( DspCapInfo ) / sizeof ( int ) );
    comm += ( sizeof ( DspCapInfo ) / sizeof ( int ) ) * 16;
    WriteGSPBuffer(( unsigned int * ) &VistaEnv, comm,
        sizeof ( VistaEnv ) / sizeof ( int ) );
    comm += ( sizeof ( VistaEnv ) / sizeof ( int ) ) * 16;
    WriteGSPBuffer( &capRegs[0], comm, 8 );
    Ack2GSP( );                        /* clear GSP message bit               */

    while ( ( GetGSPMsg( ) & INTOUT ) == 0 );
    ReadGSPBuffer(( unsigned int * ) &DspCapInfo,
        VistaEnv.commbuf, sizeof ( DspCapInfo ) / sizeof ( int ) );

    SetLutAllLinear( );                /* set 0-7 lut                         */
    SetDspLut( );                      /* set display lut                     */
    Ack2GSP( );

    JvcLive( );

    return 0;
}

int JvcGrab( )
{
    if ( CheckHR( ) == 0 )
        {
        if ( InitHR( ) == 0 )
            {
            ExitLive( );
            SetCapLut( );
            DoGrab( );
            }
        }
    else
        HRError( );

    return - 1;
}

int JvcSaveImage( char *filename )
{
    int ret = 0;
    Rect r;
    int fileType;
    //      char *ptr;
    static Boolean compress = false;
    extern int FoxFError;

#ifdef FOXPRO
        ReleaseFD();
#endif

    if ( DspCapInfo.imageType == TGAType2 ||
            DspCapInfo.imageType == TGAType10 )
        {
        fileType = ( compress ) ? TGAType10 : TGAType2;
        }
    else if ( DspCapInfo.imageType == TGAType3 ||
            DspCapInfo.imageType == TGAType11 )
        {
        fileType = ( compress ) ? TGAType11 : TGAType3;
        }
    else
        fileType = TGAType2;

    r.left = r.top = 0;
    r.right = DspCapInfo.capRect.right - DspCapInfo.capRect.left;
    r.bottom = DspCapInfo.capRect.bottom - DspCapInfo.capRect.top;

    /**
       if( TiffCreateTiffFile(r.right-r.left,r.bottom-r.top,filename) )
       TiffWriteImage(&DspCapInfo.imagePort,&r);
       else
       ErrorMessage("Error Creating TIFF File");

       TiffCloseImageFile();
     **/

    if ( strchr( filename, '.' ) == NULL )
        strcat( filename, ".TGA" );

    ret = SaveTgaFile( filename, &DspCapInfo.imagePort, &r, fileType );

#ifdef FOXPRO
        SaveFD();
#endif

    switch ( ret )
        {
        case - 1 : ErrorMessage( "File Open Error - %s [%d]", filename, errno );
            break;
        case - 2 : ErrorMessage( "File Write Error - %s [%d]", filename, FoxFError );
            break;
        case - 3 : ErrorMessage( "Memory Allocation Error" );
            break;
        }

    return ret;
}

int CancelExec( int i )
{
    Msg2GSP( CANMSG, 0, 0 );
}

