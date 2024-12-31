/****************************************************************************/
/*  boot   V3.20                                                            */
/*  Copyright (c) 1988, 1989 Texas Instruments Inc.                         */
/****************************************************************************/

/****************************************************************************/
/*  BOOT.C                                                                  */
/*                                                                          */
/*  This file contains the C source to the run-time system initialization   */
/*  for GSP C.  This is the first code executed at system reset.  The code  */
/*  has several responsibilities :                                          */
/*                                                                          */
/*       1) Set up the system stack.  This is implemented as the static     */
/*          array SYS_STACK, whose size is given by STACK_SIZE.             */
/*       2) Process the runtime initialization table, if required           */
/*       3) Disable interrupts and call the user's "main" function.         */
/*       4) At program termination, call exit.                              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* DEFINITION OF SYSTEM STACK.                                              */
/* TO CUSTOMIZE STACK SIZE, MODIFY THE "STACK_SIZE" MACRO.                  */
/****************************************************************************/
#define STACK_SIZE 2000
int sys_stack[STACK_SIZE];


/****************************************************************************/
/*                                                                          */
/*  C_INT00 - Initialize system upon system reset, then call user.          */
/*            NOTE : This function may not have ANY local variables, since  */
/*            the stack doesn't exist yet when it is called.                */
/*                                                                          */
/****************************************************************************/

int c_int00()

   {
    register int *init;              /* NOTE : INIT WILL BE IN REGISTER A9  */
    register int table;

    /************************************************************************/
    /* BEFORE ANYTHING ELSE CAN BE DONE, WE MUST INITIALIZE THE STACK.      */
    /************************************************************************/
    asm("		DINT");
    asm("       SETF    32,0,1");       /* FORCE FIELD SIZE 1 TO BE 32    */
    init = &sys_stack[1];
    asm("       MOVE    A9,FP");        /* SET UP FRAME POINTER.          */
    asm("       MOVE    A9,STK");       /* SET UP PROGRAM STACK POINTER   */

    init = &sys_stack[STACK_SIZE - 1];
    asm("       MOVE    A9,SP");        /* SET UP SYSTEM STACK POINTER    */

    /*************************************************************************/
    /* MOVE ADDRESS OF BEGINNING AND END OF INIT TABLE TO POINTERS.          */
    /* IF POINTER TO INITIALIZATION TABLE IS NOT -1, PROCESS TABLES.         */
    /*************************************************************************/
/*
    asm("       .global cinit");
    asm("       MOVI    cinit,A10");

    if (table != -1) var_init(table);*/   /* PROCESS INITIALIZATION TABLES. */

    main();                             /* CALL USER'S PROGRAM            */
/*
    exit(1);*/                            /* IF NOT ALREADY DONE, EXIT      */
   }

/*** end of boot.c ***/
