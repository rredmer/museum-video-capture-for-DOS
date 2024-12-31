#include <stdio.h>
#include <ctype.h>

#define  TRUE     1
#define  FALSE    0

/*============================================================================
**
**  Purpose:    The purpose of this routine is to determine whether a
**              string is composed entirely of white-space.
**
**  Input:      pzString
**
**  Output:     TRUE/FALSE the string is white-space
**
**  Notes:
**
----------------------------------------------------------------------------*/
int isblank( char *pzString )
{
    if ( pzString == NULL )
        return TRUE;

    while ( *pzString )
        if ( ! isspace( *pzString++ ) )
            return FALSE;

    return TRUE;
}
