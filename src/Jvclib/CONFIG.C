/*
**      config.c : Interpret the HRCAP config file.
*/

#include "hrcap.h"
#include "stdfox.h"

static int CfgIoBase( char *s )
{
    if ( s[0] == '0' && tolower( s[1] ) == 'x' )
        {
        sscanf( s + 2, "%x", &VistaEnv.ioBase );
        return 0;
        }
    else
        return - 1;
}

int CfgCapMode( char *s )
{
    int ret = 0;

    if ( ! stricmp( s, "X1" ) )
        DspCapInfo.capMode = X1;
    else if ( ! stricmp( s, "X2" ) )
        DspCapInfo.capMode = X2;
    else if ( ! stricmp( s, "X3" ) )
        DspCapInfo.capMode = X3;
    else if ( ! stricmp( s, "X6" ) )
        DspCapInfo.capMode = X6;
    else
        ret = -1;

    DspCapInfo.horzRes = PalResH * DspCapInfo.capMode;
    DspCapInfo.vertRes = PalResV * DspCapInfo.capMode;
    return ret;
}

int CfgDspMode( char *s )
{
    int ret = 0;

/* check display specification */
    if ( ! stricmp( s, "StdPal" ) )
        {
        DspCapInfo.dspMode = StdPal;
        DspCapInfo.horzCrtRes = PalResH;
        DspCapInfo.vertCrtRes = PalResV;
        }
    else if ( ! stricmp( s, "NonIntPal" ) )
        {
        DspCapInfo.dspMode = NonIntPal;
        DspCapInfo.horzCrtRes = PalResH;
        DspCapInfo.vertCrtRes = PalResV;
        }
    else if ( ! stricmp( s, "HiRes" ) )
        {
        DspCapInfo.dspMode = HiRes;
        DspCapInfo.horzCrtRes = HiresH;
        DspCapInfo.vertCrtRes = HiresV;
        }
    else
        {
        ret = -1;
        }
    return ret;
}

static int CfgPixSize( char *s )
{
    int pix;

    sscanf( s, "%d", &pix );
    if ( pix == 16 || pix == 32 )
        {
        DspCapInfo.pixSize = pix;
        return 0;
        }
    else
        return - 1;
}

static int CfgEnv( char *s )
{
    strcpy( HREnvFile, HRPath );       /* The ENV files must exist in the same*/
    strcat( HREnvFile, s );            /* directory of HRCAP                  */
    return 0;
}

static int CfgCapLut( char *s )
{
    strcpy( CapLutFile, HRPath );      /* The LUT files must exist in the same*/
    strcat( CapLutFile, s );           /* directory of HRCAP                  */
    return 0;
}

static int CfgDspLut( char *s )
{
    strcpy( DspLutFile, HRPath );      /* The LUT files must exist in the same*/
    strcat( DspLutFile, s );           /* directory of HECAP                  */
    return 0;
}

static int CfgCommPort( char *s )
{
    int ret = 0;

    if ( ! stricmp( s, "COM1" ) )
        CommPort = COM1;
    else if ( ! stricmp( s, "C1" ) )
        CommPort = COM1;
    else if ( ! stricmp( s, "COM2" ) )
        CommPort = COM2;
    else if ( ! stricmp( s, "C2" ) )
        CommPort = COM2;
    else
        ret = -1;
    return ret;
}

static int CfgCommBaud( char *s )
{
    int baud;

    sscanf( s, "%d", &baud );
    if ( baud == 4800 || baud == 9600 )
        {
        CommBaud = baud;
        return 0;
        }
    else
        return - 1;
}

static int CfgGenlock( char *s )
{
    int ret = 0;

    if ( ! stricmp( s, "Master" ) )
        {
        DspCapInfo.genlock = master;
        HRSetup.genlock = slave;
        }
    else if ( ! stricmp( s, "Slave" ) )
        {
        DspCapInfo.genlock = slave;
        HRSetup.genlock = master;
        }
    else
        ret = -1;
    return ret;
}

static int CfgGamma( char *s )
{
    int ret = 0;

    if ( ! stricmp( s, "ON" ) )
        {
        HRSetup.gamma |= 1;
        }
    else if ( ! stricmp( s, "OFF" ) )
        HRSetup.gamma = 0;
    else
        ret = -1;
    return ret;
}

static int CfgGain( char *s )
{
    int gain;

    sscanf( s, "%d", &gain );
    switch ( gain )
        {
        case - 6 :
            HRSetup.gainNum = 0;
            break;
        case 0 :
            HRSetup.gainNum = 1;
            break;
        case 6 :
            HRSetup.gainNum = 2;
            break;
        default :
            return - 1;
        }
    return 0;
}

static int CfgSensGreen( char *s )
{
    return 0;
}

static int CfgKnee( char *s )
{
    int ret = 0;

    if ( ! stricmp( s, "ON" ) )
        HRSetup.knee = HRSetup.gamma ? 1 : 0;
    else if ( ! stricmp( s, "OFF" ) )
        HRSetup.knee = 0;
    else
        ret = -1;
    return ret;
}

static int CfgShutter( char *s )
{
    static int shutterTable[] =
               {
               0, 60, 100, 120, 125, 250, 500, 1000, 2000, 4000, 10000
               };
    int speed, i;

    sscanf( s, "%d", &speed );
    if ( speed < 0 )
        {
        shutterSpeed[1] = ( Byte ) speed;
        return 0;
        }

    for ( i = 0; i < sizeof ( shutterTable ) / sizeof ( int ); i++ )
        {
        if ( shutterTable[i] == speed ) break;
        }
    if ( i == sizeof ( shutterTable ) / sizeof ( int ) )
        return - 1;
    shutterSpeed[1] = i;
    return 0;
}

static int CfgHesync( char *s )
{
    sscanf( s, "%d", &capRegs[0] );
    return 0;
}

static int CfgHeblnk( char *s )
{
    sscanf( s, "%d", &capRegs[1] );
    return 0;
}

static int CfgHsblnk( char *s )
{
    sscanf( s, "%d", &capRegs[2] );
    return 0;
}

static int CfgHtotal( char *s )
{
    sscanf( s, "%d", &capRegs[3] );
    return 0;
}

static int CfgVesync( char *s )
{
    sscanf( s, "%d", &capRegs[4] );
    return 0;
}

static int CfgVeblnk( char *s )
{
    sscanf( s, "%d", &capRegs[5] );
    return 0;
}

static int CfgVsblnk( char *s )
{
    sscanf( s, "%d", &capRegs[6] );
    return 0;
}

static int CfgVtotal( char *s )
{
    sscanf( s, "%d", &capRegs[7] );
    return 0;
}

struct ConfigProc
    {
    char *name;
    int ( *proc ) ( char * );
};
typedef struct ConfigProc ConfigProc;

static ConfigProc ConfigProcs[] =
                  {
                          {
                          "IOBASE", CfgIoBase
                          },
                          {
                          "CaptureMode", CfgCapMode
                          },
                          {
                          "DisplayMode", CfgDspMode
                          },
                          {
                          "PixelSize", CfgPixSize
                          },
                          {
                          "CaptureLut", CfgCapLut
                          },
                          {
                          "DisplayLut", CfgDspLut
                          },
                          {
                          "EnvFile", CfgEnv
                          },
                          {
                          "CommPort", CfgCommPort
                          },
                          {
                          "CommBaud", CfgCommBaud
                          },
                          {
                          "Genlock", CfgGenlock
                          },
                          {
                          "Gamma", CfgGamma
                          },
                          {
                          "Knee", CfgKnee
                          },
                          {
                          "ShutterSpeed", CfgShutter
                          },
                          {
                          "Gain", CfgGain
                          },
                          {
                          "GreenIn0dB", CfgSensGreen
                          },
                          {
                          "HESYNC", CfgHesync
                          },
                          {
                          "HEBLNK", CfgHeblnk
                          },
                          {
                          "HSBLNK", CfgHsblnk
                          },
                          {
                          "HTOTAL", CfgHtotal
                          },
                          {
                          "VESYNC", CfgVesync
                          },
                          {
                          "VEBLNK", CfgVeblnk
                          },
                          {
                          "VSBLNK", CfgVsblnk
                          },
                          {
                          "VTOTAL", CfgVtotal
                          },
                  };

static GetCfgParam( char *param )
{
    int i, ret;
    char s[81], *p;

    p = param;
    for ( i = 0; i < 80; i++ )
        {
        s[i] = *p++;
        if ( isalnum( s[i] ) == 0 )
            {
            s[i] = '\0';
            while ( isspace( *p ) ) p++;
            break;
            }
        }
    if ( i >= 80 ) return;

    for ( i = 0; i < sizeof ( ConfigProcs ) / sizeof ( ConfigProc ); i++ )
        {
        if ( ! stricmp( ConfigProcs[i].name, s ) )
            {
            ret = ( * ( ConfigProcs[i].proc ) ) ( p );
            if ( ret == -1 )
                {
                sprintf( s, "Invalid definition : %s.", param );
                Warning( s );
                }
            }
        }
}

int GetConfig( char *filename )
{
    int fp;
    char string[100];
    Boolean ret = 0;

    if ( ( fp = xopen( filename, O_RDONLY | O_TEXT ) ) != -1 )
        {
        while ( xgets( fp, string, sizeof ( string ) ) )
            {
            if ( string[0] != '#' && string[0] != '\n' )
                {
                if ( strchr( string, '\n' ) )
                    *strchr( string, '\n' ) = '\0';
                GetCfgParam( string );
                }
            }
        xclose( fp );
        }
    else
        {
        sprintf( string, "Unable to locate file %s. Using defaluts", filename );
        Warning( string );
        ret = -1;
        }
    return ret;
}

/*** end of config.c ***/
