/*
**      loadcoff.c :    Load and Execute GSP file (COFF format)
*/

#include        "hrcap.h"
#include        "nuvista.h"
#include "stdfox.h"

#define         true    1
#define         false   0

#define DATA_BUFFER_SIZE        (60*1024U)


#define INTENB  0xC0000110
#define INTPEND 0xC0000120


/* host control register bit mask */
#define         HLT                     0x8000/* halt gsp                     */
#define         CF                      0x4000/* cache flush                  */
#define         LBL                     0x2000/* lower byte lost              */
#define         INCR            0x1000 /* increment on read cycle             */
#define         INCW            0x0800 /* increment on write cycle            */
#define         NMIMODE         0x0200 /* NMI mode (no stack usage)           */
#define         NMI                     0x0100/* NMI int                      */
#define         INTOUT          0x0080 /* MSG interrupt out                   */
#define         MSGOUT          0x0070 /* message out                         */
#define         INTIN           0x0008 /* MSG interrupt in                    */
#define         MSGIN           0x0007 /* message in                          */

static unsigned int gprog[] =
                    {
                        0x0360,        /* dint                                */
                        0x0550,        /* setf 16,0,0                         */
                        0x09e8,        /* movi >1000,a8                       */
                        0x0001, 0x0000,
                        0x4d0e,        /* move a8,stk                         */
                        0x4d0f,        /* move a8,sp                          */
                        0x09e8,        /* movi >5a5a5a5a,a8 ;ack to host      */
                        0x5a5a, 0x5a5a,
                        0x0588,        /* move a8,@HSTADRL,0                  */
                        0x00d0, 0xc000,
                        0x2e08,        /* srl  16,a8                          */
                        0x0588,        /* move a8,@HSTADRH,0                  */
                        0x00e0, 0xc000,
                        0xc0ff         /* jruc $ ;spin                        */
                    };

/*
**      Copyright (c) 1988, 1989
**      Truevision Inc.
**      All Rights Reserved
**
**              Modified for PC/AT  11/26 1990  by T.Morikawa
**
*/

#define maxSections 30                 /* max sections we can handle          */
#define tiGSPMagic      ((int)(0x0090))/* magic # for TI COFF files           */

/*
**      NAME
**              GSPExec -- Begin GSP program execution at specified address
**
**      SYNOPSIS
**              void GSPExec (execAddress)
**
**              unsigned long           execAddress;    // address to start executing
**
**      DESCRIPTION
**              Starts the GSP executing at a specified address.  Typically used
**              after new code has been downloaded.
**
**      RETURN VALUE
**              None.
**
**      NOTES
*/

void GSPExec( unsigned long execAddress )
{
    /*
     **      We assume that the GSP hardware is in the state effected
     **      by a previous call to the GSPInit() routine.  This routine
     **      simply releases the GSP from its halt state after having
     **      stuffed the execute address into the GSP reset and NMI trap
     **      vectors.  We stuff the address into both vectors because:
     **              1) at power-up the chip will start execution at the reset
     **                 trap address
     **              2) we execute an NMI here which will start execution at the
     **                 NMI trap addr.
     **              3) we also set the NMIM bit which disables saving the return
     **                 address on interrupt (no stack exists at this point so we
     **                 could'nt save anyway).
     **
     **      We also ensure that the GSP's host interface is set to
     **      auto-increment on reads from & writes to the GSP's memory.
     */
    WriteGSPLong(( unsigned long ) 0xFFFFFFE0L, execAddress );/* gsp RESET addr*/
    WriteGSPLong(( unsigned long ) 0xFFFFFEE0L, execAddress );/* gsp NMI address*/
    SetGSPCtl(( int ) 0x1B00 );        /* remove halt and let her rip         */
}



/*
**      NAME
**              GSPInit -- Perform minimal initialization of the GSP.
**
**      SYNOPSIS
**              void GSPInit (void)
**
**      DESCRIPTION
**              Initializes the GSP to a default state, with nothing executing.
**
**      RETURN VALUE
**              None.
**
**      NOTES
*/

void GSPInit( )
{
    /*
     **      Set the GSP's host interface control register:
     **              - halt the GSP
     **              - flush the GSP's instruction cache
     **              - lower byte first when host accesses GSP's memory
     **              - auto-increment on reads from & writes to GSP's memory
     */
    SetGSPCtl(( int ) ( HLT + CF + INCR + INCW ) );
    /*
     **      Disable all GSP interrupts, and clear any pending GSP interrupts.
     */
    WriteGSPWord(( unsigned long ) INTENB, 0 );
    WriteGSPWord(( unsigned long ) INTPEND, 0 );
}

#define relocAbs                        0x0000/* absolute, no relocation      */
#define relocPcrWord            0x0010 /* word direct                         */
#define relocRelLong            0x0011 /* 32-bit direct                       */
#define relocOcrLong            0x0018 /* 1's complement 32-bit direct        */
#define relocGspPcr16           0x0019 /* 16-bit relative (in words)          */


int RelocateItem( relInfo, relocOffset )
coffRelocInfo *relInfo;
unsigned long relocOffset;
{
    unsigned long relocAddress;
    unsigned long relocEntry;

    /* do relocation here */
    switch ( relInfo->type )
        {
        case relocAbs :
            break;
        case relocOcrLong :
            relocAddress = relocOffset + relInfo->virtAddr;
            relocEntry = ~ReadGSPWord( relocAddress );
            relocEntry += relocOffset;
            WriteGSPLong( relocAddress, ~relocEntry );
            break;
        case relocRelLong :
            relocAddress = relocOffset + relInfo->virtAddr;
            relocEntry = ReadGSPLong( relocAddress );
            relocEntry += relocOffset;
            WriteGSPLong( relocAddress, relocEntry );
            break;
        case relocPcrWord :
            relocAddress = relocOffset + relInfo->virtAddr;
            relocEntry = ReadGSPWord( relocAddress );
            relocEntry += relocOffset;
            WriteGSPWord( relocAddress, ( int ) relocEntry );
            break;
        case relocGspPcr16 :
#ifdef DBUG
                printf("Bad reloc entry\n");
#endif
            break;
        }
    return ( 0 );
}

/* initialize record struct */
typedef struct
    {
    int sizeInWords;
    unsigned long variablePtr;
} cinitRecord;

int CinitCopy( refNum, relocOffset, curSect, dataBuffer )
int refNum;                            /* file descriptor                     */
unsigned long relocOffset;             /* relocation offset                   */
coffSectionHeader *curSect;            /* current section                     */
char *dataBuffer;
{
    cinitRecord crecord;
    int error, ret, cnt;

    error = 0;

    /* change size to words */
    curSect->size >>= 4;

    while ( curSect->size )
        {
        unsigned wordSize;             // ### int wordSize
        unsigned offset;

        offset = 0L;
        cnt = sizeof ( cinitRecord );
        ret = xread( refNum, ( char * ) &crecord, cnt );
        if ( ret != cnt )
            {
            error = ldBadDataRead;
            goto cinitCopyExit;
            }
        if ( crecord.sizeInWords == 0 )
            goto cinitCopyExit;

        while ( crecord.sizeInWords )
            {
            wordSize = crecord.sizeInWords;
            if ( wordSize >= ( DATA_BUFFER_SIZE >> 1 ) )
                {
                wordSize = DATA_BUFFER_SIZE >> 1;
                }
            /* might as well update bytes left */
            curSect->size -= wordSize;
            crecord.sizeInWords -= wordSize;

            /* read a buffer off of the file */
            cnt = wordSize << 1;
            ret = xread( refNum, ( char * ) dataBuffer, cnt );
            if ( ret != cnt )
                {
                error = ldBadDataRead;
                goto cinitCopyExit;
                }
            /* dump code to gsp */
            WriteGSPBuffer(( int * ) dataBuffer,
                relocOffset + crecord.variablePtr + offset, wordSize );
            offset += wordSize << 4;
            }
        }
cinitCopyExit :
    return error;
}
int RelocateSection( refNum, relocOffset, curSect )
int refNum;                            /* file descriptor                     */
unsigned long relocOffset;             /* relocation offset                   */
coffSectionHeader *curSect;            /* current section                     */
{
    int error, ret;
    coffRelocInfo relocInfo;
    unsigned cnt;                      // ### long cnt;

    /* Do section relocation */
    error = 0;

    if ( curSect->nReloc == 0 )        /* skip if not relocatable             */
        return ( error );

    /* seek to relocation entry     */
    if ( xlseek( refNum, curSect->relPtr, 0 ) == -1L )
        {
        return ( ldBadRelocSeek );
        }
    else
        {
        while ( curSect->nReloc )
            {
            curSect->nReloc--;
            /* read in relocation info */
            cnt = ( long ) sizeof ( coffRelocInfo );
            ret = xread( refNum, ( char * ) &relocInfo, cnt );
            if ( ret != cnt )
                {
                return ldBadRelocRead;
                }
            RelocateItem( &relocInfo, relocOffset );
            }
        }
    return ( error );
}

/*
**      regular section (allocated, relocated, loaded)'
*/
#define ldSectTypeReg           0x0000L

/*
**      dummy section (not allocated, relocated, not loaded)
*/
#define ldSectTypeDummy         0x0001L

/*
**      no load section (allocated, relocated, not loaded)
*/
#define ldSectTypeNoLoad        0x0002L

/*
**      grouped section (formed from severl input sections)
*/
#define ldSectTypeGrouped       0x0004L

/*
**      padding section (not allocated, not relocated, loaded)
*/
#define ldSectTypePad           0x0008L


/*
**      copy section (for decision function in updating fields)
**      (not allocated, relocated, loaded; relocation and line number
**      entries are processed normally)
**
*/
#define ldSectTypeCopy          0x0010L

/*
**      text section (section contains executable code)
*/
#define ldSectTypeText          0x0020L

/*
**      data section (section contains initialized data)
*/
#define ldSectTypeData          0x0040L

/*
**      bss section (section contains uninitialized data)
*/
#define ldSectTypeBSS           0x0080L

/*
**      align section (section is aligned on cache boundry)
*/
#define ldSectTypeAlign         0x0100L

/*
**      NAME
**              ReadSections -- Read and process sections in a COFF file
*/

static int
ReadSections( refNum, relocOffset, totalSize, sectionHeader,
    curSect, fileHeader, relocate )
int refNum;                            /* coff file                           */
long relocOffset;                      /* relocation offset                   */
unsigned long *totalSize;              /* variable to accumulate total size
                                          of sections loaded                  */
coffSectionHeader *sectionHeader;      /* section headers                     */
coffSectionHeader *curSect;            /* current section header              */
coffFileHeader *fileHeader;            /* file header buffer                  */
int relocate;                          /* relocation flag                     */
{
    unsigned long offset;              /* used for seeks into input file      */
    int error;                         /* error return                        */
    unsigned long relocAddress;        /* relocation address                  */
    unsigned long bssOffset = 0L;      /* bss offset address                  */
    int sectionsLeft;                  /* # sections left to process          */
    unsigned cnt;                      // ### long cnt;
    unsigned int *dataBuffer;          /* data buffer for disk I/O            */
    int ret;

    error = 0;

    /* allocate the buffer for disk I/O */
    dataBuffer = ( unsigned int * ) xmalloc( DATA_BUFFER_SIZE );
    if ( dataBuffer == 0L )
        {
        error = ldNotEnoughHeapSpace;
        dataBuffer = 0;                /* paranoia                            */
        goto readSectExit;
        }

    /* for each section to be read */
    curSect = &sectionHeader[0];

    /* Read sections */
    *totalSize = 0;                    /* total size of all loaded sections   */
    sectionsLeft = fileHeader->numSections--;

    for ( offset = ( ( long ) sizeof ( coffFileHeader ) + ( long ) fileHeader->numOptBytes );
            sectionsLeft--;
            offset += sizeof ( coffSectionHeader ),
            curSect++
        )
        {
        /* seek to next section header */
        if ( xlseek( refNum, offset, 0 ) == -1L )
            {
            error = ldBadSectionSeek;
            goto readSectExit;
            }

        /* and read it in */
        cnt = ( long ) sizeof ( coffSectionHeader );
        ret = xread( refNum, ( char * ) curSect, cnt );
        if ( ret != cnt )
            {
            error = ldBadSectionRead;
            goto readSectExit;
            }

        if ( xlseek( refNum, curSect->sectionPtr, 0 ) == -1L )
            {
            error = ldBadSectionDataSeek;
            goto readSectExit;
            }

        /* check flags for no load sections; */
        /* setting the size zero will prevent a load */
        if ( curSect->flags == ldSectTypeDummy ||
                curSect->flags == ldSectTypeNoLoad ||
                curSect->flags == ldSectTypePad )
            {
            curSect->size = 0L;
            }
        /* check bss or cinit sections */
        if ( curSect->flags == ldSectTypeBSS )
            {
            bssOffset = relocOffset + curSect->virtAddr;
            }
        else if ( curSect->flags & ldSectTypeCopy && curSect->name[1] == 'c' )
            {
            if ( error = CinitCopy( refNum, relocOffset, curSect, dataBuffer ) )
                goto readSectExit;
            else
                {
                curSect->size = 0L;
                relocOffset = bssOffset;
                }
            }

        /* Check to see if it is a bit address, we cannot load */
        /* executables on bit boundries. */
        if ( ( curSect->virtAddr + relocOffset ) & 0xfL )
            {
            /* is it a text section ?? */
            if ( curSect->name[1] == 't' )
                {
                error = ldExecutableOnBitAddress;
                goto readSectExit;
                }
            }

        /* normal word aligned data or bit aligned data */
        relocAddress = relocOffset + curSect->virtAddr;

        /* change size to words */
        curSect->size >>= 4;

        if ( curSect->sectionPtr == 0L )
            {
            curSect->size = 0;
            }

        *totalSize += curSect->size;
        while ( curSect->size )
            {
            /* # bytes to write to gsp */
            register int byteSize;

            /* get # words to write to gsp,
             ** read blocks of DATA_BUFFER_SIZE, till
             ** end, then read remaining */
            if ( curSect->size < ( DATA_BUFFER_SIZE >> 1 ) )
                {
                byteSize = ( int ) curSect->size << 1;
                }
            else
                {
                byteSize = DATA_BUFFER_SIZE;
                }

            /* might as well update bytes left */
            curSect->size -= ( byteSize >> 1 );

            /* read a buffer off of the file */
            cnt = ( long ) byteSize;
            ret = xread( refNum, ( char * ) dataBuffer, cnt );
            if ( ret != cnt )
                {
                error = ldBadDataRead;
                goto readSectExit;
                }
            /* dump code to gsp */
            WriteGSPBuffer( dataBuffer, relocAddress, byteSize >> 1 );
            relocAddress += ( ( long ) byteSize ) << 3;
            }
        if ( relocate && curSect->name[1] == 't' )
            {
            /* locate this section */
            if ( RelocateSection( refNum, relocOffset, curSect ) < 0 )
                {
                error = ldBadRelocateSection;
                return ( error );
                }
            }
        }

readSectExit :
    /* get rid of the buffer used for disk I/O */
    if ( dataBuffer != ( unsigned int * ) 0 )
        {
        xfree( dataBuffer );
        }
    return ( error );
}
/*
**      NAME
**              FdLoadGSPProg -- Load a program on the GSP and optionally execute it
**
**      SYNOPSIS
**              int FdLoadGSPProg(refNum,loadAddr,execFlg,loadFlg,size,start)
**              short                   refNum;                 // file descriptor of object module
**              unsigned long   loadAddr;               // address to start loading module
**              int                             execFlag;               // exec flag, 0=no exec, 1=exec
**              int                             loadFlag;               // load flag, 0=no load, 1=load
**              unsigned long   *size;                  // required memory size of module
**              unsigned long   *start;                 // resulting start address after reloc
**
**      DESCRIPTION
**              Load relocatable or non-relocatable gsp COFF file to the VISTA.
**              By specifying a loadAddress, the module can be relocated in
**              memory.  The loadFlag determines whether to load or not.  This is
**              useful in determining the memory size required for the download by
**              making a call with 'loadFlag == FALSE' and then examine size and start
**              on return.
**              The execFlag determines whether the downloaded object module will be
**              executed.  Size is the number of bytes required on the VISTA to hold
**              the program.  Start is the resulting starting execution address of
**              the module after relocation.
**
**      RETURN VALUE
**              Error status, or 0 if successful.
**
**      NOTES
**
*/

int FdLoadGSPProg( refNum, loadAddress, execFlag, loadFlag, size, start )
int refNum;                            /* file descriptor                     */
unsigned long loadAddress;             /* address of program load             */
int execFlag;                          /* exec after load flag                */
int loadFlag;                          /* load, or just get info flag         */
unsigned long *size;                   /* memory required in words            */
unsigned long *start;                  /* start address                       */
{
    int error;                         /* local error holder                  */
    unsigned long ip;                  /* instruction ptr for GSP reset       */
    unsigned long relocOffset;         /* relocation offset                   */
    int err;
    int cnt;
    int ret;
    unsigned long totalSize;           /* totalSize of the loaded sections    */
    coffSectionHeader *sectionHeader;  /* section headers                     */
    coffSectionHeader *curSect;        /* current section header              */
    coffFileHeader fileHeader;         /* file header buffer                  */
    coffOptFileHeader optFileHeader;   /* optional header buffer              */
    int relocate;                      /* relocation flag                     */

    error = 0;

    #ifndef FOXPRO
        printf( "In FdLoadGSPProg()\n" );
    #endif

    /*
     ** allocate some buffer for the sections
     */
    sectionHeader = ( coffSectionHeader * ) xmalloc( maxSections * sizeof ( coffSectionHeader ) );
    if ( sectionHeader == 0L )
        {
        error = ldNotEnoughHeapSpace;
        sectionHeader = 0;             /* paranoia                            */
        goto ERROR_EXIT;
        }

    GSPInit( );                        /* halt graphics processor             */

    /*
     ** On entry to this routine we assume that the GSP is in the
     ** state effected by GSPInit(). If it is, we then search
     ** for a file of the passed in filename to load into the GSP
     ** address space. The file is assumed to be COFF relocatable
     ** file format.
     */

    relocate = ( loadAddress == 0L ) ? false : true;

    /* read COFF file header */
    cnt = ( long ) sizeof ( coffFileHeader );
    ret = xread( refNum, ( char * ) &fileHeader, cnt );
    if ( ret != cnt )
        {
        error = ldBadCoffHeader;
        goto ERROR_EXIT;
        }
    /* check magic */
    if ( fileHeader.magicNumber != tiGSPMagic )
        {
        error = ldBadMagicNumber;
        goto ERROR_EXIT;
        }
    /* check if trying to relocate an absolute file */
    if ( ( fileHeader.flags & 0x1 ) && relocate )
        {
        error = ldFileIsAbsoulte;
        goto ERROR_EXIT;
        }
    /* if no optional header, then we don't have a fully resolved file */
    if ( fileHeader.numOptBytes == 0 )
        {
        error = ldUnlinkedCoffFile;
        /* this is where symbolic re-linking code would go */
        goto ERROR_EXIT;
        }
    else
        {
        /* see if optional header is correct size */
        if ( fileHeader.numOptBytes != sizeof ( coffOptFileHeader ) )
            {
            goto ERROR_EXIT;
            }
        /* read optional file header */
        cnt = sizeof ( coffOptFileHeader );
        ret = xread( refNum, ( char * ) &optFileHeader, cnt );
        if ( ret != cnt )
            {
            error = ldBadOptHeader;
            goto ERROR_EXIT;
            }

#ifdef DBUG
                printf( "Code size in bytes = %ld\n", optFileHeader.tSize / 8);
                printf( "Data size in bytes = %ld\n", optFileHeader.dSize / 8);
                printf( "BSS size in bytes = %ld\n", optFileHeader.bSize / 8);
                printf( "Entry = 0x%lx\n", optFileHeader.pEntry);
                printf( "Text Start = 0x%lx\n", optFileHeader.textStart);
                printf( "Data Start = 0x%lx\n", optFileHeader.dataStart);
#endif

        if ( loadAddress == 0L )
            {
            loadAddress = optFileHeader.textStart;
            }
        /* calc relocation offset, and initial ip */
        if ( relocate )
            {
            relocOffset = loadAddress - optFileHeader.textStart;
            if ( relocOffset == 0 )
                relocate = false;
            ip = optFileHeader.pEntry + relocOffset;
            }
        else
            {
            ip = optFileHeader.pEntry;
            relocOffset = 0;
            }
        }
    /* check room for all section headers */
    if ( fileHeader.numSections > maxSections )
        {
        error = ldNotEnoughSections;
        goto ERROR_EXIT;
        }
    if ( loadFlag )
        {
        ret = ReadSections( refNum, relocOffset, &totalSize, sectionHeader,
            curSect, &fileHeader, relocate );

        if ( ret < 0 )
            {
            error = ldBadReadSections;
            goto ERROR_EXIT;
            }
        }
    error = 0;                         /* no error if we got here             */

ERROR_EXIT :

    if ( loadFlag )
        {
        if ( ( ! error ) && execFlag )
            {
            GSPExec( ip );
            }
        }
    *start = ip;
    *size = totalSize;

    /* get rid of the buffer used for sections      */
    if ( sectionHeader != ( coffSectionHeader * ) 0 )
        {
        xfree( sectionHeader );
        }
    return ( error );
}


#define         PHSTADRH        0x22a  /* host address high                   */

int LoadCoff( char *fname )
{
    int err = -1;
    unsigned long address;
    int cnt, timeout, ack;
    int fh;
    unsigned long progSize, progStart, data;

    ack = false;
    timeout = 18;

    while ( ( ack == false ) && ( timeout-- ) )
        {
        GSPInit( );                    /* minimal initialize GSP              */

        /* load check program into memory */
        WriteGSPBuffer( gprog, VistaEnv.vistaStr, 18 );

        /* execute the program */
        GSPExec( VistaEnv.vistaStr );

        /* a bit of delay needs to wake up GSP program */
        Delay( 2 );
        data = GetCommAddr( );
        if ( data == 0x5A5A5A5AL )
            {                          /* check the ack from AtVista          */
            ack = true;                /* set breaking loop flag              */
            if ( ( fh = xopen( fname, O_RDONLY | O_BINARY ) ) == -1 )
                {
                err = -1;
                }
            else
                {
                err = FdLoadGSPProg( fh, VistaEnv.vistaStr, true, true,
                    &progSize, &progStart );
                xclose( fh );
                }
            }
        }
    if ( ( ack == false ) || ( err != 0 ) )
        return - 1;
    else
        return 0;
}



/*** end of loadcoff.c ***/
