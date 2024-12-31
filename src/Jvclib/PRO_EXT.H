/*
 +----------------------------------------------------------------------+
 |                                                       pro_ext.h                                                                      |
 +----------------------------------------------------------------------+


 ================                                       ===========
   ==             ===                                     ==       ==
   ==                   =                                         ==            ==
   ==                                                             ==            ==
   ==              ===     =====  =====   ==       ==  ===       ====            ===
   =====        ==         ==    ==        ==     =========              == ==   ==       ==     ==
   ==      ==           ==        ==  ==          ==                     ===             ==               ==
   ==     ==             ==        ===            ==                     ==                     ==                 ==
   ==      ==           ==        == ==           ==                     ==                      ==               ==
   ==           ==         ==    ==       ==      ==                     ==                       ==     ==
 =======           ===     =====  ====  ======             ======                        ===

 +--------------------------------------------------------------------------+
 |                                       FoxPro -- Relational Database System                                   |
 +--------------------------------------------------------------------------+
 |                                                                                                                                                      |
 |         Copyright (c) 1989-1993 Microsoft Corporation as an unpublished      |
 |     licensed proprietary work.  All rights reserved.                     |
 |                                                                                                                                                      |
 |         This software contains valuable trade secrets and proprietary                |
 |         information, and is protected by federal copyright laws, the                 |
 |         violation of which can result in civil damages and criminal pro-             |
 |         secution.  Access to these source materials is granted only by               |
 |         explicit written permission from Microsoft.                          |
 |                                                                                                                                                      |
 +--------------------------------------------------------------------------+
*/

#ifdef __cplusplus

extern "C" {                           /* Assume C declarations for C++       */

#endif                                 /* __cplusplus                         */

#pragma pack(1)                        // Assume byte structure packing

#ifndef PRO_EXT_INCLUDED

    #define PRO_EXT_INCLUDED

/* Semi-portable way to deal with 'far' pointers */
    #ifndef FAR
        #define FAR __far
    #endif

    #ifndef GLOBAL_INCLUDED
        typedef unsigned MHANDLE;      /* A memory handle.                    */
        typedef int NTI;               /* A name table index.                 */
        typedef unsigned long Ticks;   /* A timer tick count.                 */
        #define TRUE    1
        #define FALSE   0
        #define YES             1
        #define NO              0
        typedef char TEXT;
    #endif

#if defined(I80386)
#if I80386 && !WINAPI16
#ifdef GLOBAL_INCLUDED
#define MHANDLE unsigned short
#endif
#endif
#endif

#if __WATCOMC__
#ifndef __386__
#pragma aux default parm routine [ax bx cx dx] modify [es];
#endif
#endif

    typedef unsigned long WHandle;     /* A Window Handle                     */
    typedef long MENUID;               /* A Menu id.                          */
    typedef long MenuId;               /* A Menu id.                          */
    typedef long ITEMID;               /* An item id.                         */
    typedef long ItemId;               /* An item id.                         */

    typedef int FCHAN;                 /* A file I/O channel.                 */
    #define SCHEMEWIDTH             11
    typedef char Scheme[SCHEMEWIDTH];  /* A FoxPro color scheme               */


#ifdef GLOBAL_INCLUDED

#define WHANDLE WHandle

    #else

        typedef WHandle WHANDLE;       /* Users can use WHANDLE               */

    #endif

    #ifndef FOXWIND_INCLUDED

/*              _WOpen() flag values.                                   */

        #define WCURSOR         (1<<1) /* Enable the cursor                   */
        #define ZOOM            (1<<2) /* Permit the window to be zoomed      */
        #define ADJ                     (1<<3)/* Allow Window Size to be Adjusted*/
        #define CLOSE           (1<<4) /* Permit Closing of System Window     */
        #define MOVE            (1<<5) /* Allow the window to be moved        */
        #define HSCROLLBAR      (1<<6) /* Show Horizontal Scrollbars          */
        #define VSCROLLBAR      (1<<7) /* Show Vertical Scrollbars            */
        #define AUTOSCROLL      (1<<8) /* Window should Auto scroll           */
        #define WEVENT          (1<<10)/* Participates in activate/deact events*/
        #define SHADOW          (1<<11)/* Window will produce a shadow        */
        #define WMODAL          (1<<12)/* Window is a modal window            */
        #define WMINIMIZE       (1<<13)/* Window can be minimized             */

    #endif

    #ifndef FOXEVENT_INCLUDED

/*      _FindWindow() return values.                            */
        #define inBorder                 0/* In the window border region      */
        #define inHelp                   1/* In the Help region               */
        #define inContent                2/* In the content/text region       */
        #define inDrag                   3/* In the Drag/Title bar region     */
        #define inGrow                   4/* In the grow/resize region        */
        #define inGoAway                 5/* In the goAway/close region       */
        #define inZoom                   6/* In the zoom region               */
        #define inVUpArrow               7/* In the vertical up arrow region  */
        #define inVDownArrow     8     /* In the vertical down arrow region   */
        #define inVPageUp                9/* In the vertical page up region   */
        #define inVPageDown             10/* In the vertical page down region */
        #define inVThumb                11/* In the vertical thumb region     */
        #define inHUpArrow              12/* In the horizontal up arrow region*/
        #define inHDownArrow    13     /* In the horizontal down arrow region */
        #define inHPageUp               14/* In the horizontal page up region */
        #define inHPageDown             15/* In the horizontal page down region*/
        #define inHThumb                16/* In the horizontal thumb region   */
        #define inMenuBar               17/* In the menu bar                  */

/* EventRec.what values                 */

        #define nullEvent                               0/* Null              */
        #define mouseDownEvent                  1/* Mouse down                */
        #define keyDownEvent                    2/* Key down                  */
        #define activateEvent                   3/* Activate window           */
        #define deactivateEvent                 4/* Deactivate window         */
        #define updateEvent                             5/* Redraw window     */
        #define menuHitEvent                    6/* Menu Hit                  */
        #define menuUpdateEvent                 7/* Menu Update required      */
        #define closeEvent                              8/* Close window      */
        #define hideEvent                               9/* Hide window       */
        #define showEvent                          10/* Show window           */
        #define hotkeyEvent                        11/* An ON KEY LABEL was pressed*/
        #define sizeEvent                          12/* Size window event     */
#if defined(__WINDOWS__) || defined(_WINDLL)
#define rMouseDownEvent            13  /* Right mouse down event              */
#define zoomEvent                         999/* Not supported in Windows      */
        #else
            #define zoomEvent                          13/* Zoom window event */
        #endif

/* EventRec.modifiers defines */

        #define charCodeMask    0x0fff /* Character code mask                 */
        #define shiftCodeMask   0xf000 /* Shift key mask                      */

        #define shiftKey                0x1000/* Either shift key        1 - down*/
        #define ctrlKey                 0x2000/* Control key             1 - down*/
        #define altKey                  0x4000/* Alternate key           1 - down*/
        #define literalKey              0x8000/* This key is interpreted literally*/

/* EventRec.mbState code defines */

        #define leftButton              (1<<0)/* Left button status   1 - down*/
        #define rightButton             (1<<1)/* Right button status  1 - down*/
        #define centerButton    (1<<2) /* Center button status 1 - down       */

/* EventRec.mcState code defines */

        #define posChange               (1<<0)/* Position change                1 - change*/
        #define leftPressed             (1<<1)/* Left button pressed    1 - pressed*/
        #define leftReleased    (1<<2) /* Left button released   1 - released */
        #define rightPressed    (1<<3) /* Right button pressed   1 - pressed  */
        #define rightReleased   (1<<4) /* Right button released  1 - released */
        #define centerPressed   (1<<5) /* Center button pressed  1 - pressed  */
        #define centerReleased  (1<<6) /* Center button released 1 - released */

    #endif

/* Flag values for _InKey()             */
    #define SHOWCURSOR              (1<<0)
    #define HIDECURSOR              (1<<1)
    #define MOUSEACTIVE             (1<<2)

/* Flag values for the _ALen() function */

    enum
        {
        AL_ELEMENTS,
        AL_SUBSCRIPT1,
        AL_SUBSCRIPT2
    };

/* FPFI is a 32 bit pointer to a function returning an int */
    typedef int ( FAR *FPFI ) ( );

#ifndef ATVISTA
typedef struct {
        short h;
        short v;
} Point;

typedef struct {
        short top;
        short left;
        short bottom;
        short right;
} Rect;
#endif

    #ifndef FOXMENU_INCLUDED

/*- Menu structure type entry definitions used by _NewMenu()    */

        #define MPOPUP                  1/* Menu is a POPUP type menu         */
        #define MPAD                    2/* Menu is a PAD type menu           */
        #define MBAR                    3/* Menu is a BAR type menu           */

    #endif

/* These are the API identifiers for the System Menu Constants.                 */

    enum
        {
        _LASTITEM = -2, _FIRSTITEM,

        _SYSMENU,
        _SYSMSYSTEM, _SYSMFILE, _SYSMEDIT, _SYSMDATA, _SYSMRECORD, _SYSMPROG,
        _SYSMWINDOW,

        _SYSTEM,
        _SYSTEMABOUT, _SYSTEMHELP, _SYSTEMMACRO, _SYSTEMSEP100, _SYSTEMFILER,
        _SYSTEMCALC, _SYSTEMDIARY, _SYSTEMSPECIAL, _SYSTEMASCII,
        _SYSTEMCAPTURE, _SYSTEMPUZZLE,

        _FILE,
        _FILENEW, _FILEOPEN, _FILECLOSE, _FILECLOSEALL, _FILESEP100, _FILESAVE,
        _FILESAVEAS, _FILEREVERT, _FILESEP200, _FILESETUP, _FILEPRINT, _FILESEP300,
        _FILEQUIT,

        _EDIT,
        _EDITUNDO, _EDITREDO, _EDITSEP100, _EDITCUT, _EDITCOPY, _EDITPASTE,
        _EDITCLEAR, _EDITSEP200, _EDITSELECTALL, _EDITSEP300, _EDITGOTO,
        _EDITFIND, _EDITFINDAGAIN, _EDITREPLACE, _EDITREPLACEALL,
        _EDITSEP400, _EDITPREF,

        _DATA,
        _DATASETUP, _DATABROWSE, _DATASEP100, _DATAAPPEND, _DATACOPY, _DATASORT,
        _DATATOTAL, _DATASEP200, _DATAAVERAGE, _DATACOUNT, _DATASUM, _DATACALCULATE,
        _DATAREPORT, _DATALABEL, _DATASEP300, _DATAPACK, _DATAREINDEX,

        _RECORD,
        _RECORDAPPEND, _RECORDCHANGE, _RECORDSEP100, _RECORDGOTO, _RECORDLOCATE,
        _RECORDCONTINUE, _RECORDSEEK, _RECORDSEP200, _RECORDREPLACE, _RECORDDELETE,
        _RECORDRECALL,

        _PROG,
        _PROGDO, _PROGSEP100, _PROGCANCEL, _PROGRESUME, _PROGSEP200,
        _PROGCOMPILE, _PROGGENERATE, _PROGDOCUMENT, _PROGGRAPH,

        _WINDOW,
        _WINDOWHIDE, _WINDOWHIDEALL, _WINDOWSHOWALL, _WINDOWCLEAR, _WINDOWSEP100,
        _WINDOWMOVE, _WINDOWSIZE, _WINDOWZOOM, _WINDOWMINMAX, _WINDOWROTATE,
        _WINDOWCOLOR, _WINDOWSEP200, _WINDOWCOMMAND, _WINDOWDEBUG, _WINDOWTRACE,
        _WINDOWVIEW,

        _REPORT, _LABEL, _BROWSE, _MACRO, _DIARY, _DAFILER, _SCREEN,
        _MBLDR, _PROJECT, _RQBE,

        _SYSTEMSEP200, _SYSTEMHELPSRCH, _SYSTEMHELPHOWTO, _EDITPASTELINK, _EDITLINK,
        _EDITINSERTOBJ, _EDITCVTSTATIC, _EDITSEP500, _PROGSEP300,

        _TEXT, _EDITOBJECT, _PROGBEAUT
    };


/* Now anyone can simulate a breakpoint with BreakPoint() */
#ifdef __WATCOMC__
#pragma aux _BreakPoint = 0xcc;        /* INT 3                               */
void _BreakPoint(void);
#endif


#ifdef _MSC_VER
#define _BreakPoint() __asm     \
  {                             \
         int 3h                         \
}
#endif

/* Alternate values for parmCount to modify how FoxPro treats the function */
    #define INTERNAL                -1 /* Not callable from FoxPro            */
    #define CALLONLOAD              -2 /* Call when library is loaded         */
    #define CALLONUNLOAD    -3         /* Call when library is unloaded       */

/* The FoxInfo structure contains the descriptions of the functions
   contained in the library.
*/

    typedef struct
        {
        char FAR *funcName;            /* Function name (all caps)            */
        FPFI function;                 /* Actual function address             */
        short parmCount;             /* # parameters specified or a flag value*/
        char FAR *parmTypes;           /* Parameter list description          */
    } FoxInfo;

    typedef struct _FoxTable
        {
        struct _FoxTable FAR *nextLibrary;/* Linked list of libraries         */
        short infoCount;               /* # of functions in this library      */
        FoxInfo FAR *infoPtr;          /* Function list                       */
    } FoxTable;

/* An expression's value                */
    typedef struct
        {
        char ev_type;
        char ev_padding;
        short ev_width;
        unsigned short ev_length;
        long ev_long;
        double ev_real;
        MHANDLE ev_handle;
    } Value;

/* A reference to a database or memory variable */
    typedef struct
        {
        char l_type;
        short l_where,                 /* Database number or -1 for memory    */
                        l_NTI,         /* Variable name table offset          */
                        l_offset,      /* Index into database                 */
                        l_subs,        /* # subscripts specified 0 <= x <= 2  */
                        l_sub1, l_sub2;/* subscript integral values           */
    } Locator;

/* A parameter to a library function.                   */
    typedef union
        {
        Value val;
        Locator loc;                   /* An 'R' in l_type means the Locator  */
        /* part of this union is in use.                */
    } Parameter;

/* A paramter list to a library function.               */
    typedef struct
        {
        short int pCount;              /* Number of Parameters PASSED.        */
        Parameter p[1];                /* pCount Parameters.                  */
    } ParamBlk;

    #ifndef _INC_WINDOWS
        #ifndef _WINDEF_
            typedef unsigned short HWND;
        #endif
#else
extern  HANDLE          Inst;
    #endif

    typedef long EDPOS;                /* Editor text offset                  */
    typedef long EDLINE;               /* Editor line number                  */
    #define MAXFILENAME             130/* longest pathname                    */
    #define MAXFONTNAME             64 /* Max length of a font name           */

    #ifndef EDITOR_INCLUDED
        #define EDCOMMAND               0
        #define EDPROGRAM               1
        #define EDFILE                  2
        #define EDMEMO                  3
        #define EDQUERY                 6
        #define EDSCREEN                7
        #define EDMENU                  8

        #define RO_BYFILE               (0x01)/* readOnly bits                */
        #define RO_BYUSER               (0x02)
        #define RO_BYRECORD             (0x04)

    #endif

/* An editor's enivronment.                                             */
    typedef struct
        {
        char filename[MAXFILENAME];
        EDPOS length;                  /* #bytes in text                      */
        unsigned short lenLimit;       /* Max allowable length. 0 = infinite. */
        unsigned short dirty,          /* Has the file been changed?          */
                        autoIndent,    /* Auto indent?                        */
                        backup,        /* Make backup files?                  */
                        addLineFeeds,  /* add line feeds when saving?         */
                        autoCompile,   /* Shall we auto compile this thing?   */
                        addCtrlZ,      /* add end of file ctrl-z?             */
                        savePrefs,     /* save edit preferences?              */
                        dragAndDrop,   /* Allow drag-and-drop                 */
                        readOnly,      /* RO_BYFILE, RO_BYUSER, RO_BYRECORD   */
                        status,        /* display status bar?                 */
                        lockPrefs,     /* Can update the preferences ?        */
                        insertMode;
        short wrap;                    /* if <0, new line at Return only      */

        EDPOS selStart;                /* Selection start                     */
        EDPOS selEnd;                  /* Selection end                       */
        EDPOS selAnchor;               /* Selection anchor point              */
        short justMode;                /* Justification                       */
        short tabWidth;                /* TAB size in spaces                  */

        char fontName[MAXFONTNAME];
        short fontSize;
        short fontStyle;
        short kind;                    /* Kind of editor session              */
    } EDENV;


/* Event record definitions */
    typedef struct
        {
        unsigned short what;           /* Event code                          */
        Ticks when;                    /* Ticks since startup                 */
        Point where;                   /* Mouse location                      */
        unsigned long message;         /* Key/window                          */
        unsigned long misc;            /* Event dependant misc info           */
        unsigned long misc2;           /* Event dependant misc info           */
        unsigned char mbState;         /* Mouse buttons state                 */
        unsigned char mcState;         /* Mouse cond activate state           */
        unsigned short modifiers;
    } EventRec;

/* Flags for the _DBStatus function */
    #define DB_BOF                  1  /* BOF()                               */
    #define DB_EOF                  2  /* EOF()                               */
    #define DB_RLOCKED              4  /* Current record is RLOCKed           */
    #define DB_FLOCKED              8  /* Database is FLOCKed                 */
    #define DB_EXCLUSIVE    16         /* Database is open EXCLUSIVEly        */
    #define DB_READONLY             32 /* Database is READONLY                */

/* Flag values for _DBLock()    */
    #define DBL_RECORD              0
    #define DBL_FILE                1

/* Flag values for the _NewVar function */
    #define NV_PUBLIC               0
    #define NV_PRIVATE              1

/* Mode flag values for the __FIO function              */
    #define FIO_FGETS               0
    #define FIO_FREAD               1
    #define FIO_FPUTS               2
    #define FIO_FWRITE              3

/* Mode flag values for the _FOpen function             */
    #define FO_READONLY             0
    #define FO_WRITEONLY    1
    #define FO_READWRITE    2

/* Mode flag values for the _FCreate function   */
    #define FC_NORMAL               0
    #define FC_READONLY             1
    #define FC_HIDDEN               2
    #define FC_SYSTEM               4
    #define FC_TEMPORARY    128

/* Mode flag values for the _FSeek function             */
    #define FS_FROMBOF              0  /* From beginning of file              */
    #define FS_RELATIVE             1  /* From current position               */
    #define FS_FROMEOF              2  /* From end of file                    */

/* Mode flag values for the __WStat function    */
    #define WS_TOP                  0
    #define WS_BOTTOM               1
    #define WS_LEFT                 2
    #define WS_RIGHT                3
    #define WS_HEIGHT               4
    #define WS_WIDTH                5
    #define WS_SETPORT              7

/* Mode flag values for the __WControl function */
    #define WC_CLEAR                0
    #define WC_CLOSE                1
    #define WC_HIDE                 2
    #define WC_SHOW                 3
    #define WC_SELECT               4
    #define WC_SENDBEHIND   5

/* Mode flag values for the __WAdjust function  */
    #define WA_MOVE                 0
    #define WA_SIZE                 1
    #define WA_POSCURSOR    2

/* Mode flag value for the __WPort function             */
    #define WP_ONTOP                0
    #define WP_OUTPUT               1

/* Mode flag value for the _WZoom function              */
    #define WZ_MAXIMIZED    0
    #define WZ_NORMAL               1
    #define WZ_MINIMIZED    2

/* Border strings for typical window borders.   */
    #define WO_DOUBLEBOX    "\x0cd\x0cd\x0ba\x0ba\x0c9\x0bb\x0c8\x0bc\x0cd\x0cd\x0ba\x0ba\x0c9\x0bb\x0c8\x0bc"
    #define WO_SINGLEBOX    "\x0c4\x0c4\x0b3\x0b3\x0da\x0bf\x0c0\x0d9\x0c4\x0c4\x0b3\x0b3\x0da\x0bf\x0c0\x0d9"
    #define WO_PANELBORDER  "\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db\x0db"
    #define WO_SYSTEMBORDER "\x020\x020\x020\x020\x0fe\x0f0\x020\x0f9\x020\x020\x020\x020\x020\x020\x020\x020"

    #ifndef FOXWIND_INCLUDED

/* Border string offsets.                                               */
        #define selectBorder            0
        #define deselectBorder          8

        #define topEdge                         0
        #define bottomEdge                      1
        #define leftEdge                        2
        #define rightEdge                       3
        #define topLeftCorner           4
        #define topRightCorner          5
        #define bottomLeftCorner        6
        #define bottomRightCorner       7
        #define maxBorderLen       17  /* Border string length (maximum)      */
    #endif


    #ifndef COLORS_INCLUDED

        #define BLACK_ON        0x00   /* Foreground color attributes         */
        #define BLUE_ON         0x01
        #define GREEN_ON        0x02
        #define CYAN_ON         0x03
        #define RED_ON          0x04
        #define MAGENTA_ON      0x05
        #define BROWN_ON        0x06
        #define WHITE_ON        0x07

        #define BLACK   (BLACK_ON << 4)/* Background color attributes.        */
        #define BLUE    (BLUE_ON << 4)
        #define GREEN   (GREEN_ON << 4)
        #define CYAN    (CYAN_ON << 4)
        #define RED             (RED_ON << 4)
        #define MAGENTA (MAGENTA_ON << 4)
        #define BROWN   (BROWN_ON << 4)
        #define WHITE   (WHITE_ON << 4)

        #define BRIGHT  0x08           /* Intensify foreground color          */
        #define BLINK   0x80           /* Blink this                          */


/* The following values are used in the WA_ISSHADOW column of the
   schemes to indicate whether the window casts a shadow.               */

        #define CASTSSHADOW (BRIGHT | BLACK_ON | BLACK)
        #define CASTSNOSHADOW (BRIGHT | WHITE_ON | WHITE | BLINK)


/* Color scheme numbers for use by _WOpen and others    */

        enum
            {
            USER_SCHEME,
            USERMENU_SCHEME,
            MBAR_SCHEME,
            POPUP_SCHEME,
            DIALOG_SCHEME,
            MODAL_POPUP_SCHEME,
            ALERT_SCHEME,
            WINDOW_SCHEME,
            NONMODAL_POPUP_SCHEME,
            BROWSE_SCHEME,
            REPORT_SCHEME,
            ALERT_POPUP_SCHEME
        };

/* Color index numbers used by _WSetAttr() and _WAttr() */

        enum
            {
            WA_PENCOLOR = -1,          /* CURRENT PEN COLOR                   */

            WA_NORMAL,                 /* normal text                         */
            WA_ENHANCED,               /* enhanced text                       */
            WA_BORDER,                 /* window border                       */
            WA_FOREMOST,               /* title when foremost                 */
            WA_TITLE,                  /* title otherwise                     */
            WA_SELECTED,               /* selected text                       */
            WA_HOTKEY,                 /* control hotkeys                     */
            WA_SHADOW,           /* color of shadows that fall on this window.*/
            WA_ENABLED,                /* enabled control                     */
            WA_DISABLED,               /* disabled control                    */
            WA_ISSHADOW                /* window casts a shad                 */
        };


    #endif


/* Prototypes for the API Functions */

/* Streaming output routines    */
    void _PutChr( int character );
    void _PutStr( char FAR *string );
    void _PutValue( Value FAR *val );

/* Memory management functions */
    MHANDLE _AllocHand( unsigned int hsize );
    void _FreeHand( MHANDLE hand );
    void FAR *_HandToPtr( MHANDLE hand );
    unsigned _GetHandSize( MHANDLE hand );
    int _SetHandSize( MHANDLE hand, unsigned int size );
    void _HLock( MHANDLE hand );
    void _HUnLock( MHANDLE hand );
    unsigned int _StackAvail( void );
    int _MemAvail( unsigned int );
#ifdef  __WATCOMC__
void FAR *__Alloca(unsigned int size);
void FAR *_Alloca(unsigned int size);
#pragma aux __Alloca =  0x29 0xc4 /* sub sp, ax */\
                                                0x89 0xe0 /* mov ax, sp */\
                                                0x8c 0xd2 /* mov dx, ss */\
                                                parm caller [ax] value[ax dx] modify [sp];
#define _Alloca(size)   ((_StackAvail()>(unsigned)size) ? __Alloca(((size)+1)&0xfffe): 0)
#endif

/* String handling functions    */
    int _StrLen( char FAR *string );
    void _StrCpy( char FAR *dest, char FAR *src );
    int _StrCmp( char FAR *string1, char FAR *string2 );

/* Memory management functions  */
#ifdef _MSC_VER
void FAR *  __fastcall _Alloca(unsigned int amount);

void __far * __far __cdecl _fmemmove(void __far *, const void __far *, unsigned int);
void __far * __far __cdecl _fmemset(void __far *, int, unsigned int);
int __far __cdecl _fmemcmp(const void __far *, const void __far *, unsigned int);

#define _MemCmp(dest, src, length) _fmemcmp(dest, src, length)
#define _MemMove(dest, src, length) _fmemmove(dest, src, length)
#define _MemFill(ptr, c, length) _fmemset(ptr, c, length)
#endif


#if __WATCOMC__
#pragma aux _MemFill parm [es di] [ax] [cx] modify [ax];
#pragma aux _MemCmp      parm [es di] [dx si] [ax] modify nomemory [ax si di];
void _MemMove(void FAR *dest, void FAR *src, unsigned int length);
void _MemFill(void FAR *ptr, int character, unsigned int length);
int      _MemCmp(void FAR *ptr, void FAR *ptr2, unsigned int length);
#endif

/* Functions to set the return value of a library functiion             */
    void _RetVal( Value FAR *val );
    void _RetChar( char FAR *string );
    void _RetInt( long ival, int width );
    void _RetFloat( double flt, int width, int dec );
    void _RetDateStr( char FAR *string );
    void _RetLogical( int );

/* Database Input/Output functions */
    long _DBRecNo( int workarea );
    long _DBRecCount( int workarea );
    int _DBStatus( int workarea );
    int _DBRead( int workarea, long record );
    int _DBWrite( int workarea );
    int _DBAppend( int workarea, int carryflag );
    long _DBRewind( int workarea );
    long _DBSkip( int workarea, long distance );
    long _DBUnwind( int workarea );
    int _DBReplace( Locator FAR *fld, Value FAR *val );
    long _DBSeek( Value FAR *val );
    int _DBLock( int workarea, int what );
    void _DBUnLock( int workarea );
    int _DBAppendRecords( int workarea, unsigned short nrecs, char FAR *buffer );

/* Memo Field Input/Output functions */
    FCHAN _MemoChan( int workarea );
    long _AllocMemo( Locator FAR *fld, long size );
    long _FindMemo( Locator FAR *fld );
    long _MemoSize( Locator FAR *fld );

/* Memory variable manipulation functions */
    NTI _NewVar( char FAR *name, Locator FAR *loc, int flag );
    int _Release( NTI nti );
    int _Store( Locator FAR *loc, Value FAR *val );
    int _Load( Locator FAR *loc, Value FAR *val );
    long _ALen( NTI nti, int mode );
    int _FindVar( NTI nti, int where, Locator FAR *loc );
    NTI _NameTableIndex( char FAR *name );

/* File Input/Output */

    FCHAN __FOpen( char FAR *filename, int mode, int create );

    #define _FOpen(filename, mode)                  __FOpen(filename, mode, NO)
    #define _FCreate(filename, mode)                __FOpen(filename, mode, YES)

    int __FFlush( FCHAN chan, int close );

    #define _FFlush(chan)   __FFlush(chan, NO)
    #define _FClose(chan)   __FFlush(chan, YES)

    int __FStat( FCHAN chan, int error );

    #define _FEOF(chan)             __FStat(chan, NO)
    #define _FError()               __FStat(0, YES)

    unsigned int __FIO( FCHAN chan, char FAR *buffer, unsigned int maxlen, int mode );

    #define _FGets(chan, buffer, maxlen)    __FIO(chan, buffer, maxlen, FIO_FGETS)
    #define _FRead(chan, buffer, maxlen)    __FIO(chan, buffer, maxlen, FIO_FREAD)
    #define _FPuts(chan, buffer)                    __FIO(chan, buffer, 0,          FIO_FPUTS)
    #define _FWrite(chan, buffer, maxlen)   __FIO(chan, buffer, maxlen, FIO_FWRITE)

    long _FSeek( FCHAN chan, long position, int mode );
    int _FCHSize( FCHAN chan, long length );
    int _FCopy( FCHAN dchan, long dpos, FCHAN schan, long spos, long len );

#if defined(__WINDOWS__) || defined(_WINDLL)

/* User Interface routines */

#define PIXELMODE       0
#define CHARMODE        1

unsigned __ActivateHandler(FPFI handler, short charmode);

#define _ActivateHandler(handler)               __ActivateHandler(handler, CHARMODE)
#define _ActivateHandlerP(handler)              __ActivateHandler(handler, PIXELMODE)

void    _DeActivateHandler(unsigned);
unsigned __ActivateIdle(FPFI handler, short charmode);

#define  _ActivateIdle(handler)                 __ActivateIdle(handler, CHARMODE)
#define  _ActivateIdleP(handler)                __ActivateIdle(handler, PIXELMODE)

void    _DeActivateIdle(unsigned);

int             __GetNextEvent(EventRec FAR *event, short charmode);

#define _GetNextEvent(event)                    __GetNextEventP(event, CHARMODE)
#define _GetNextEventP(event)                   __GetNextEventP(event, PIXELMODE)

void    __DefaultProcess(EventRec FAR *event, short charmode);

#define _DefaultProcess(event)                  __DefaultProcess(event, CHARMODE)
#define _DefaultProcessP(event)                 __DefaultProcess(event, PIXELMODE)

#define _MousePos(pt)                   __MousePos(pt, CHARMODE)
#define _MousePosP(pt)                  __MousePos(pt, PIXELMODE)

int             __MousePos(void FAR *pt, int);

/* Windowing routines.  */
int             __FindWindow(WHANDLE FAR *wh, Point pt, int charmode);
void    __GlobalToLocal(void FAR *pt, WHANDLE wh, int);
WHANDLE __WOpen(int top, int left, int bottom, int right, int flag, int
                           scheme_num, Scheme FAR *scheme, char FAR *bord, int pixelmode);

#define _WOpenP(top, left, bottom, right, flag, scheme_num, scheme, bord) __WOpen(top, left, bottom, right, flag, scheme_num, scheme, bord, PIXELMODE)
#define _WOpen(top, left, bottom, right, flag, scheme_num, scheme, bord) __WOpen(top, left, bottom, right, flag, scheme_num, scheme, bord, CHARMODE)

#define _GlobalToLocal(pt, wh)          __GlobalToLocal(pt, wh, CHARMODE)
#define _GlobalToLocalP(pt, wh)         __GlobalToLocal(pt, wh, PIXELMODE)

#define _FindWindow(wh, pt)                     __FindWindow(wh, pt, CHARMODE)
#define _FindWindowP(wh, pt)            __FindWindow(wh, pt, PIXELMODE)


#define _WTop(wh)               ((unsigned)__WStat(wh, WS_TOP, CHARMODE))
#define _WBottom(wh)    ((unsigned)__WStat(wh, WS_BOTTOM, CHARMODE))
#define _WLeft(wh)              ((unsigned)__WStat(wh, WS_LEFT, CHARMODE))
#define _WRight(wh)             ((unsigned)__WStat(wh, WS_RIGHT, CHARMODE))
#define _WHeight(wh)    ((unsigned)__WStat(wh, WS_HEIGHT, CHARMODE))
#define _WWidth(wh)             ((unsigned)__WStat(wh, WS_WIDTH, CHARMODE))
#define _WSetPort(wh)   ((WHANDLE)__WStat(wh, WS_SETPORT, CHARMODE))

#define _WTopP(wh)              ((unsigned)__WStat(wh, WS_TOP, PIXELMODE))
#define _WBottomP(wh)   ((unsigned)__WStat(wh, WS_BOTTOM, PIXELMODE))
#define _WLeftP(wh)             ((unsigned)__WStat(wh, WS_LEFT, PIXELMODE))
#define _WRightP(wh)    ((unsigned)__WStat(wh, WS_RIGHT, PIXELMODE))
#define _WHeightP(wh)   ((unsigned)__WStat(wh, WS_HEIGHT, PIXELMODE))
#define _WWidthP(wh)    ((unsigned)__WStat(wh, WS_WIDTH, PIXELMODE))

unsigned long __WStat(WHANDLE wh, int mode, int pixelmode);

#define _WMove(wh, pt)                  __WAdjust(wh, pt, WA_MOVE, CHARMODE)
#define _WSize(wh, pt)                  __WAdjust(wh, pt, WA_SIZE, CHARMODE)
#define _WPosCursor(wh, pt)             __WAdjust(wh, pt, WA_POSCURSOR, CHARMODE)
#define _WMoveP(wh, pt)                 __WAdjust(wh, pt, WA_MOVE, PIXELMODE)
#define _WSizeP(wh, pt)                 __WAdjust(wh, pt, WA_SIZE, PIXELMODE)
#define _WPosCursorP(wh, pt)    __WAdjust(wh, pt, WA_POSCURSOR, PIXELMODE)

void __WAdjust(WHANDLE wh, Point pt, int mode, int charmode);

void    __WScroll(WHANDLE wh, Rect r, int dv, int dh, int charmode);

#define _WScroll(wh, r, dv, dh)         __WScroll(wh, r, dv, dh, CHARMODE)
#define _WScrollP(wh, r, dv, dh)        __WScroll(wh, r, dv, dh, PIXELMODE)
#define _WClearRect(wh, r)                      __WScroll(wh, r, 0, 0, CHARMODE);
#define _WClearRectP(wh, r)                     __WScroll(wh, r, 0, 0, PIXELMODE);

#define _WGetCursor(wh)                  __WGetCursor(wh, CHARMODE)
#define _WGetCursorP(wh)                 __WGetCursor(wh, PIXELMODE)

Point   __WGetCursor(WHANDLE wh, int);

void    __SetMenuPoint(MenuId menuid, Point loc, int charmode);

#define _SetMenuPoint(menuid, loc)              __SetMenuPoint(menuid, loc, CHARMODE)
#define _SetMenuPointP(menuid, loc)             __SetMenuPoint(menuid, loc, PIXELMODE)


    #else                              // __WINDOWS__ || _WINDLL

/* User Interface routines */
        unsigned _ActivateHandler( FPFI handler );
        void _DeActivateHandler( unsigned );
        unsigned _ActivateIdle( FPFI handler );
        void _DeActivateIdle( unsigned );
        int _GetNextEvent( EventRec FAR *event );
        void _DefaultProcess( EventRec FAR *event );
        int _MousePos( Point FAR *pt );

/* Windowing routines.  */
        int _FindWindow( WHandle FAR *wh, Point pt );
        void _GlobalToLocal( Point FAR *pt, WHandle wh );
        WHandle _WOpen( int top, int left, int bottom, int right, int flag, int
                    scheme_num, Scheme FAR *scheme, char FAR *bord );

        #define _WTop(wh)               ((unsigned)__WStat(wh, WS_TOP))
        #define _WBottom(wh)    ((unsigned)__WStat(wh, WS_BOTTOM))
        #define _WLeft(wh)              ((unsigned)__WStat(wh, WS_LEFT))
        #define _WRight(wh)             ((unsigned)__WStat(wh, WS_RIGHT))
        #define _WHeight(wh)    ((unsigned)__WStat(wh, WS_HEIGHT))
        #define _WWidth(wh)             ((unsigned)__WStat(wh, WS_WIDTH))
        #define _WSetPort(wh)   ((WHandle)__WStat(wh, WS_SETPORT))

        unsigned long __WStat( WHandle wh, int mode );

        #define _WMove(wh, pt)          __WAdjust(wh, pt, WA_MOVE)
        #define _WSize(wh, pt)          __WAdjust(wh, pt, WA_SIZE)
        #define _WPosCursor(wh, pt)     __WAdjust(wh, pt, WA_POSCURSOR)

        void __WAdjust( WHandle wh, Point pt, int mode );

        void _WScroll( WHandle wh, Rect r, int dv, int dh );
        #define _WClearRect(wh, r)      _WScroll(wh, r, 0, 0);

        Point _WGetCursor( WHandle wh );
        void _SetMenuPoint( MenuId menuid, Point loc );

    #endif                             // __WINDOWS__ || _WINDLL


    int _InKey( int timeout, int flag );
    void _RefreshDisplay( void );
    void _RefreshVideo( void );

    #define _WClear(wh)             __WControl(wh, WC_CLEAR)
    #define _WClose(wh)             __WControl(wh, WC_CLOSE)
    #define _WHide(wh)              __WControl(wh, WC_HIDE)
    #define _WShow(wh)              __WControl(wh, WC_SHOW)
    #define _WSelect(wh)    __WControl(wh, WC_SELECT)
    #define _WSendBehind(w) __WControl(w, WC_SENDBEHIND)

    void __WControl( WHANDLE wh, int mode );

    #define _WOnTop()               __WPort(WP_ONTOP)
    #define _WGetPort()             __WPort(WP_OUTPUT)

    WHandle __WPort( int mode );

    void _WZoom( WHANDLE wh, int newstate );

    void __WSetTitle( WHANDLE wh, char FAR *text, int mode );

    #define WT_SETFOOTER    0
    #define WT_SETTITLE             1
    #define WT_GETFOOTER    2
    #define WT_GETTITLE             3

    #define _WSetFooter(wh, footer) __WSetTitle(wh, footer, WT_SETFOOTER)
    #define _WSetTitle(wh, title)   __WSetTitle(wh, title, WT_SETTITLE)
    #define _WFooter(wh, footer)    __WSetTitle(wh, footer, WT_GETFOOTER)
    #define _WTitle(wh, title)              __WSetTitle(wh, title, WT_GETTITLE)

    int _WAttr( WHANDLE wh, int color );
    void _WSetAttr( WHANDLE wh, int color, int attr );
    void _WPutChr( WHANDLE wh, int character );
    void _WPutStr( WHANDLE wh, char FAR *str );

/* Functions to execute FoxPro statements and evaluate FoxPro expressions */

    int _Execute( char FAR *stmt );
    int _Evaluate( Value FAR *val, char FAR *expr );

/* Menuing functions.                   */

    long __MenuStat( long x, int mode );

    #define _MenuId(literal)                                __MenuStat(literal, 0)
    #define _GetNewItemId(menuid)                   __MenuStat(menuid, 1)
    #define _CountItems(menuid)                             __MenuStat(menuid, 2)
    #define _GetNewMenuId()                                 __MenuStat(0, 3)

    int _MenuInteract( MenuId FAR *menuid, ItemId FAR *itemid );

    void __MenuAction( MenuId menuid, int mode );

    #define _ActivateMenu(menuid)                   __MenuAction(menuid, 0)
    #define _DeActivateMenu(menuid)                 __MenuAction(menuid, 1)
    #define _DisposeMenu(menuid)                    __MenuAction(menuid, 2)

    int _NewMenu( int mtype, MenuId menuid );
    void _SetMenuColor( MenuId menuid, int scheme );
    ItemId _GetItemId( MenuId menuid, long index );
    int _NewItem( MenuId menuid, ItemId itemid, ItemId beforeid, char FAR *prompt );
    void _DisposeItem( MenuId menuid, ItemId itemid );


    void _SetItemSubMenu( MenuId menuid, ItemId itemid, MenuId submenuid );
    void _SetItemColor( MenuId menuid, ItemId itemid, int scheme );
    void _GetItemText( MenuId menuid, ItemId itemid, char FAR *text );
    void _SetItemText( MenuId menuid, ItemId itemid, char FAR *text );
    int _GetItemCmdKey( MenuId menuid, ItemId itemid, char FAR *text );
    void _SetItemCmdKey( MenuId menuid, ItemId itemid, int key, char FAR *text );
    void _OnSelection( MenuId menuid, ItemId itemid, FPFI routine );

/* FoxPro Dialogs               */
    int _Dialog( int scheme, char FAR *text, char FAR *button1,
            char FAR *button2, char FAR *button3, int def, int esc );


/* Error Handling               */
    void _Error( int code );
    void _UserError( char FAR *message );
    int _ErrorInfo( int code, char FAR *message );


    #define ED_SAVENOASK    0
    #define ED_SAVEASK              1
    #define ED_SAVEAS               2

/* Mode flags for the editor functions */

/* Mode flag values for the __EdPos function */
    #define ED_SETPOS               0
    #define ED_GETLINEPOS   1
    #define ED_GETLINENUM   2
    #define ED_GETPOS               3

/* Mode flag values for the __EdScroll function */
    #define ED_SCROLLTOPOS  0
    #define ED_SCROLLTOSEL  1

/* Mode flag values for the __EdManip function */
    #define ED_SENDKEY              0
    #define ED_DELETE               1
    #define ED_INDENT               2

/* Mode flag values for the __EdClipBrd function */
    #define ED_COPY                 0
    #define ED_CUT                  1
    #define ED_PASTE                2
    #define ED_UNDO                 3
    #define ED_REDO                 4

/* Mode flag values for the __EdEnv function */
    #define ED_SETENV               0
    #define ED_GETENV               1

/* Mode flag values for the __EdRevert function */
    #define ED_REVERT               0
    #define ED_SAVE                 1

/* Mode flag values for the __WFindTitle function */
    #define WFINDTITLE      0
    #define WMAINWINDOW     1


/* Editor Functions */
    WHandle _EdOpenFile( char FAR *filename, int mode );
    int _EdCloseFile( WHandle wh, int opt );
    void __EdRevert( WHandle wh, int mode );

    #define _EdRevert(wh)                           __EdRevert(wh, ED_REVERT)
    #define _EdSave(wh)                                     __EdRevert(wh, ED_SAVE)

    #define _EdSetPos(wh, pos)                      ((long)__EdPos(wh, (EDPOS) pos, ED_SETPOS))
    #define _EdGetLinePos(wh, line)         ((EDPOS)__EdPos(wh, (EDLINE) line, ED_GETLINEPOS))
    #define _EdGetLineNum(wh, pos)          ((EDLINE)__EdPos(wh, (EDPOS) pos, ED_GETLINENUM))
    #define _EdGetPos(wh)                           ((EDLINE)__EdPos(wh, 0, ED_GETPOS))

    long __EdPos( WHandle wh, long pos, int mode );
    int _EdPosInView( WHandle wh, EDPOS pos );

    #define _EdScrollToPos(wh, pos, flag)   (__EdScroll(wh, pos, flag, ED_SCROLLTOPOS))
    #define _EdScrollToSel(wh, flag)                (__EdScroll(wh, 0, flag, ED_SCROLLTOSEL))

    void __EdScroll( WHandle wh, EDPOS pos, int flags, int mode );

    #define _EdSendKey(wh, key)                     __EdManip(wh, key, ED_SENDKEY)
    #define _EdDelete(wh)                           __EdManip(wh, 0, ED_DELETE)
    #define _EdIndent(wh, tabs)                     __EdManip(wh, tabs, ED_INDENT)

    void __EdManip( WHandle wh, int n, int mode );

    EDPOS _EdSkipLines( WHandle wh, EDPOS pos, int n );
    void _EdInsert( WHandle wh, char FAR *str, unsigned long len );
    TEXT _EdGetChar( WHandle wh, EDPOS pos );
    void _EdGetStr( WHandle wh, EDPOS pos1, EDPOS pos2, TEXT FAR *str );

    #define _EdCopy(wh)                                     __EdClipBrd(wh, ED_COPY)
    #define _EdCut(wh)                                      __EdClipBrd(wh, ED_CUT)
    #define _EdPaste(wh)                            __EdClipBrd(wh, ED_PASTE)
    #define _EdUndo(wh)                                     __EdClipBrd(wh, ED_UNDO)
    #define _EdRedo(wh)                                     __EdClipBrd(wh, ED_REDO)

    void __EdClipBrd( WHandle wh, int mode );
    void _EdSelect( WHandle wh, EDPOS pos1, EDPOS pos2 );
    void _EdUndoOn( WHandle wh, int flag );
    void _EdActive( WHandle wh, int flag );
    int _EdLastError( WHandle wh );

    #define _EdSetEnv(wh, env)                      __EdEnv(wh, env, ED_SETENV)
    #define _EdGetEnv(wh, env)                      __EdEnv(wh, env, ED_GETENV)

    #define _WFindTitle(title)                      __WFindTitle(title, WFINDTITLE)
    #define _WMainWindow()                          __WFindTitle(0, WMAINWINDOW)

    int __EdEnv( WHandle, EDENV FAR *, int );
    HWND _WhToHwnd( WHandle );
    WHandle __WFindTitle( char FAR *, int );


#if defined(I80386)
#if I80386
#undef MHANDLE
#endif
#endif

#ifdef GLOBAL_INCLUDED
#undef  WHANDLE
#endif


#if defined(_MSC_VER) && (_MSC_VER < 800)
#define MK_FP(__s,__o) ((void __far*)(((unsigned long)(__s)<<16)|(unsigned)(__o)))
#endif

#endif

#pragma pack()                         // Restore structure packing

#ifdef __cplusplus
}                                      /* End of extern "C" {                 */
#endif                                 /* __cplusplus                         */
