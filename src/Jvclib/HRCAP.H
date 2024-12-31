/*
**      hrcap.h : HRCAP header file
*/

#include        <stdio.h>
#include        <stdlib.h>
#include        <fcntl.h>
#include        <sys\types.h>
#include        <sys\stat.h>
#include        <io.h>
#include        <ctype.h>
#include        <string.h>
#include        <malloc.h>

#include        <setjmp.h>

//----- #include        "d:\gsptools\gspregs.h"

#ifndef         __HRCAP__
    #define         __HRCAP__

    #include        "hrtypes.h"


/* CRT mode */
    #define StdPal          0
    #define NonIntPal       1
    #define HiRes           2

/* Capture mode */
    #define X1                      1
    #define X2                      2
    #define X3                      3
    #define X6                      6

/* Display resolution */
    #define HiresH          1024
    #define HiresV          768

    #define PalResH         736
    #define PalResV         576


/* Capture resolution */
    #define PalCapH         736
    #define PalCapV         576

    #define MaxV4M          1000      /* max vertical resolution with 4M Vista*/
    #define HiresCapH       1024
    #define HiresCapV       768


/* Communication bit */
    #define INTOUT          0x0080     /* MSG interrupt out                   */
    #define MSGOUT          0x0070     /* message out                         */
    #define INTIN           0x0008     /* MSG interrupt in                    */
    #define MSGIN           0x0007     /* message in                          */


/* Command message */
    #define CMDMSG          1          /* message from gsp                    */
    #define CURMSG          2
    #define SIZMSG          3
    #define ZOMMSG          4
    #define PIXMSG          5
    #define CANMSG          7


    #define WB              0x0007     /* white & black                       */
    #define ORANGE  0x0006             /* Orange                              */
    #define HILITE  0x000f             /* Hilite                              */
    #define REVERS  0x0070             /* reverse video                       */


/* AtVista register/memory address */
    #define ROWTBL  0xF8000000
    #define VRAMSTR 0xFE000000         /* 4MB AtVista memory start            */
    #define VRAMEND 0xFFFB0000

    #define LUT0    0xFFFB0000
    #define LUT1    0xFFFB8000
    #define LUT2    0xFFFC0000
    #define LUT3    0xFFFC8000
    #define LUT4    0xFFFD0000
    #define LUT5    0xFFFD8000
    #define LUT6    0xFFFE0000
    #define LUT7    0xFFFE8000

    #define PITCH   0x8000L            /* default image pitch                 */


/* TGA file format */
    #define TGAType2        2
    #define TGAType3        3
    #define TGAType10       10
    #define TGAType11       11


/* Command code for HR-CAMERA */
    enum
        {
        NopCmd = 0x80,
        GenLockCmd,
        SamplingCmd,
        GammaCmd,
        PhaseCmd,
        SetupCmd,
        GainCmd,
        InitCmd,
        RepCmd,
        PhaseTestCmd,
        MotorCmd,
        FocusCmd,

        SubsetCmd = 0x90,
        ShutterCmd,
        FFCmd,
        OptIrisCmd,
        OptFocusCmd,
        OptZoomCmd
    };

/* Macros */
    #define HitAnyKey()     keyread()


#endif

extern VistaENV VistaEnv;
extern HvipENV HvipEnv;

extern char HRPath[80];                /* HRCAP directory path name           */
extern char DspLutFile[80];            /* display LUT file name               */
extern char CapLutFile[80];            /* capture LUT file name               */
extern char GspOutFile[80];            /* .OUT file name                      */
extern char HRCfgFile[80];             /* .CFG file name                      */
extern char HREnvFile[80];             /* .ENV file name                      */
extern char ColorBarFile[80];          /* .COL file name                      */

extern LUT CapLutTbl[256];             /* capture lut data                    */
extern LUT DspLutTbl[256];             /* display lut data                    */
extern DspCapINFO DspCapInfo;

extern int CharAttr;

extern Boolean hvipExist;

extern EventRecord theEvent;

extern Rect ScrnBox;
extern Rect CmdBox;
extern Rect SubBox;
extern Rect MsgBox;

extern int capRegs[];

extern HRSETUP HRSetup;
extern Byte shutterSpeed[2];

#define COM1            0x3F8
#define COM2            0x2F8
extern int CommPort;
extern int CommBaud;

/*
**      function prototypes
*/

/*** from hrcap.c ***/
extern Warning( char *s );
extern Abort( char *s );
extern Delay( int cnt );
extern MsgFromHR( Byte *s, int len );
extern int CheckGSPMsg( );
extern int MsgFromGSP( unsigned *s );
extern Msg2GSP( int cmd, int par1, int par2 );
extern int Msg2HR( Byte *s, int len, int ticks );
extern SetRect( Rect *r, int left, int top, int right, int bottom );
extern Complete( );
extern ErrorMessage( char *s, ... );
extern HRError( );
extern DrawCapRes( int width, int height );
extern SetIrisLut( );
extern SetCapLutLinear( );
extern SetCapLut( );
extern Boolean IsMainCommand( int ch );
extern DrawCommand( HrProc *proc, Rect *box );
extern int DoCommand( int ch, HrProc *proc, HrProc **selected, Rect *box );
extern MarkCommand( char *string, Boolean on );
extern GetEvent( EventRecord *event );
extern CancelExec( int ch );

/*** from config.c ***/
extern GetConfig( char *filename );
extern GetOptions( int argc, char *argv[] );
extern int CfgDspMode( char *s );
extern SetShutterSpeed( );

/*** from display.c ***/
extern unsigned long _GetPixel( );
extern DrawPixelVal( unsigned long rgb );
extern int Display( int index );
extern DoDisplay( );

/*** from grab.c ***/
extern WaitAckGSP( EventRecord *e );
extern int WaitAckHR( );
extern int DoGrab( );

/*** from focus.c ***/
extern DoFocus( );

/*** from live.c ***/
extern GetCapRect( Rect *r );
extern LiveVista( );
extern DoLive( );

/*** from image.c ***/
extern PutPercent( int per, int cent );
extern DrawSubcommand( char *cmd[] );
extern GetFileName( char *filename, char *def );
extern DoFiles( );

/*** from tgafile.c ***/
extern int SaveTgaFile( char *filename, GPtr port, Rect *r, int fileType );
extern int LoadTgaFile( char *filename, GPtr port, int *fileType );

/*** from calib.c ***/
extern int SelectFocusArea( char *s, Rect *r );
extern DoBlackBalance( );
extern DoWhiteBalance( );

/*** from loadcoff.c ***/
extern void GSPExec( unsigned long execAddress );
extern void GSPInit( );
extern int LoadCoff( char *fname );

/*** from hload.c ***/
extern hload( );

/*** from videoc.c ***/
extern box( Rect *rp );
extern dbox( Rect *rp );
extern clrbox( Rect *rp );
extern movein( Rect *rp, int line, int col );

/*** from tty.c ***/
extern DoTTY( );

/*** from atasm.asm ***/
extern outpw( int portNum, int writeData );
extern int inpw( int portNum );
extern movemem( char *src, char *dst, int len );
extern unsigned long GetCommAddr( );
extern int ReadGSPData( );
extern WriteGSPData( int writeData );
extern SetGSPMsg( int message );
extern int GetGSPMsg( );
extern SetGSPCtl( int ctl );
extern GetGSPCtl( );
extern SetGSPAddr( unsigned long address );
extern WriteGSPWord( unsigned long address, int writeData );
extern WriteGSPLong( unsigned long address, unsigned long writeData );
extern int ReadGSPWord( unsigned long address );
extern unsigned long ReadGSPLong( unsigned long address );
extern WriteGSPBuffer( unsigned *bufPtr, unsigned long address, int cnt );
extern ReadGSPBuffer( unsigned *bufPtr, unsigned long address, int cnt );
extern _WriteVstaMem( unsigned *bufPtr, unsigned long address, int cnt, int memtype );
extern _ReadVstaMem( unsigned *bufPtr, unsigned long address, int cnt, int memtype );
extern CopyPat( char *src, int cnt, int pattlen );
extern keywrite( int code );
extern int keyread( );
extern int keyhit( );
extern keyflush( );
extern int shftstat( );
extern shftrls( int maskBit );
extern long getticks( );
extern curoff( );
extern curon( );
extern initscr( );
extern endwin( );
extern getcur( );
extern move( int line, int column );
extern setatr( int attribute );
extern addch( int ch );
extern repch( int ch, int len );
extern addstr( char *str );
extern int chkcrt( );




/*** end of hrcap.h ***/
