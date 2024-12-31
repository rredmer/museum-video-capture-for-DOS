#include                <stdio.h>
#include                <stdarg.h>
#include        "stdfox.h"
#ifdef FOXPRO
  #include        <pro_ext.h>            // FoxPro Applications Programming Interface
#endif

Warning( char *s )
{
#ifdef FOXPRO
        _UserError(s);
    #else
        fprintf( stderr, "\n      %s\n", s );
    #endif
}

Abort( char *s )
{
    putch( 7 );                        /* beep sound                          */
    Warning( s );
//      exit(0);
}

ErrorMessage( char *format, ... )
{
    va_list args;
#ifdef FOXPRO
        char msg[100];
        va_start(args, format);
//        strcpy(msg,"ErrMsg - \"");
//        vsprintf(msg+strlen(msg), format, args);
        vsprintf(msg, format, args);
//        strcat(msg,"\"");
        _UserError(msg);                // this does not return
    #else
        va_start( args, format );
        fputs( "ErrMsg - \"", stderr );
        vfprintf( stderr, format, args );
        fputs( "\"\n", stderr );
    #endif
}

WaitWindow( char *format, ... )
{
    va_list args;
#ifdef FOXPRO
        char msg[100];
        va_start(args, format);
        strcpy(msg,"WAIT WINDOW \"");
        vsprintf(msg+strlen(msg), format, args);
        strcat(msg,"\"");
        _Execute(msg);                // this does not return
    #else
        va_start( args, format );
        fputs( "ErrMsg - \"", stderr );
        vfprintf( stderr, format, args );
        fputs( "\"\n", stderr );
    #endif
}

Complete( )
{
    printf( "Complete !!" );
}
