/*
    Name 1: Adrian Cantu Garza
    UTEID 1: ac73538
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
//////////////////////////////////////////////////////////
//  HELPER FUNCTIONS

int *mic; // current microinstruction

int powr(int n, int p){
    int num = 1;
    for(int i = 0; i < p; i++){
        num = num*n;
    }
    return num;
}

void get_Binary(int* bin, int b, int s, int num){
    int size = b - s + 1;
    int n = powr(2,size -1);
    num = (num % (powr(2,b+1)));
    num = num >> s;
    for(int i = 0; i < size; i++){
        if((num / n) == 1){
            bin[i] = 1;
            num = num % n;
        } 
        else bin[i] = 0;

        n = n / 2;
    }
}

int eval_adder(){
    int ir = CURRENT_LATCHES.IR;
    uint16_t lsh = 0;
    uint16_t a2m = GetADDR2MUX(mic);
    if(a2m == 0) lsh = 0;
    else if(a2m == 1) {
        lsh = ir & 0x003F;
        if(ir & 0x0020) lsh = lsh | 0xFFC0; 
    }
    else if(a2m == 2){
        lsh = ir & 0x01FF;
        if(ir & 0x0100) lsh = lsh | 0xFE00;
    }
    else if(a2m == 3){
        lsh = ir & 0x07FF;
        if(ir & 0x0400) lsh = lsh | 0xF800;
    }

    //LSHF
    if(GetLSHF1(mic) == 1){
        lsh = lsh << 1;
    } 
    //ADDER1MUX
    uint16_t a1m = 0;
    if(GetADDR1MUX(mic)){   //Base
        if(GetSR1MUX(mic)){
            a1m = CURRENT_LATCHES.REGS[(ir & 0x01C0) >> 6];
        }
        else{
            a1m = CURRENT_LATCHES.REGS[(ir & 0x0E00) >> 9];
        }
    }
    else{                   //PC
        a1m = CURRENT_LATCHES.PC;
    }
    //adder
    uint16_t num = a1m + lsh;
    return num;
}

int getG_MARMUX(){
    int ir = CURRENT_LATCHES.IR;
    if(GetMARMUX(mic)){ //ADDER
        return eval_adder();
    }
    else{               //7.0
        int val = (ir & 0x00FF) << 1;
        return val;
    }
}

int getG_PC(){
    return CURRENT_LATCHES.PC;
}

int getG_ALU(){
    int A;
    int B;
    int sr1;
    //Get A
    if(GetSR1MUX(mic) == 0){
        sr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    }
    else{
        sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
    }
    A = CURRENT_LATCHES.REGS[sr1];
    //Get B
    if(CURRENT_LATCHES.IR & 0x0020){
        B = (CURRENT_LATCHES.IR & 0x001F);
        if(B & 0x0010) B = B | 0xFFE0;
    }
    else{
        int sr2 = CURRENT_LATCHES.IR & 0x0007;
        B = CURRENT_LATCHES.REGS[sr2];
    }
    //Execute operation
    int aluk = GetALUK(mic);
    if(aluk == 0){
        uint16_t sum = A + B;
        return sum;
    }
    else if(aluk == 1){
        return (A & B);
    }
    else if(aluk == 2){
        return (A ^ B);
    }
    else{
        return A;
    }
}

int getG_SHF(){
    int num = CURRENT_LATCHES.IR & 0x000F;
    int sel = (CURRENT_LATCHES.IR & 0x0030) >> 4;
    int sr1;
    if(GetSR1MUX == 0){
        sr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    }
    else{
        sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;   
    }
    uint16_t val = CURRENT_LATCHES.REGS[sr1];

    if(sel == 0){
        return (val << num);
    } 
    else if(sel == 1){
        return (val >> num);
    }
    else{
        int bit15 = val & 0x8000;
        for(int i = 0; i < num;i++){
            val = val >> 1;
            val = val | (0x8000 & bit15);
        }
        return val;
    }
}

int getG_MDR(){
    int mdr = CURRENT_LATCHES.MDR;
    int size = GetDATA_SIZE(mic);
    int mar_0 = CURRENT_LATCHES.MAR & 0x0001;
    if(size == 0){   //BYTE
        int val = 0;
        if(mar_0 == 0){

            val =  (mdr & 0x00FF);
        }
        else{
            val = ((mdr & 0xFF00)>>8);
        }
        if(val & 0x0080){
            val = val | 0xFF00;
        }
        return val;
    }
    else{           //WORD
        return mdr;
    }

}

/////LATCHES//////
void MAR_latch(){
    if(GetLD_MAR(mic) == 1){
        NEXT_LATCHES.MAR = BUS;
    }
    else{
        NEXT_LATCHES.MAR = CURRENT_LATCHES.MAR;
    }
}

void MDR_latch(){
    if(GetLD_MDR(mic) == 1){
        if(GetMIO_EN(mic) == 1){
            int mar = CURRENT_LATCHES.MAR;
            NEXT_LATCHES.MDR = (MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0];
        }
        else{
            if(GetDATA_SIZE(mic) == 1){ //word
                NEXT_LATCHES.MDR = BUS;
            }
            else{                       //Byte
                int temp = BUS & 0x00FF;
                NEXT_LATCHES.MDR = temp + (temp << 8);
            }
        }
    }
    else{
        NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
    }
}

void IR_latch(){
    if(GetLD_IR(mic) == 1){
        NEXT_LATCHES.IR = BUS;
    }
    else{
        NEXT_LATCHES.IR = CURRENT_LATCHES.IR;
    }
}

void BEN_latch(){

    if(GetLD_BEN(mic) == 1){
        int ir = CURRENT_LATCHES.IR;

        if((CURRENT_LATCHES.N && (ir & 0x0800)) || (CURRENT_LATCHES.Z && (ir & 0x0400)) || (CURRENT_LATCHES.P && (ir & 0x0200))){
            NEXT_LATCHES.BEN = 1;
        }
        else{
            NEXT_LATCHES.BEN = 0;
        }
    }
    else{
        NEXT_LATCHES.BEN = CURRENT_LATCHES.BEN;
    }
}

void REG_latch(){
    if(GetLD_REG(mic) == 1){
        int dr = 0;
        if(GetDRMUX(mic) == 0){ //DR
            dr = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
        }
        else dr = 7;
        NEXT_LATCHES.REGS[dr] = BUS;
    }
    else{
        for(int i = 0; i <=7 ; i++){
            NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.REGS[i];
        }
    }
}

void CC_latch(){
    if(GetLD_CC(mic) == 1){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
        if(BUS >= 0x8000 && BUS <= 0xFFFF){
            NEXT_LATCHES.N = 1;
        }
        else if(BUS == 0){
            NEXT_LATCHES.Z = 1;
        }
        else{
            NEXT_LATCHES.P = 1;
        }
    }
    else{
        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
    }
}

void PC_latch(){
    if(GetLD_PC(mic) == 1){
        int mux = GetPCMUX(mic);
        if(mux == 0){
             NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
        }
        else if(mux == 1){
             NEXT_LATCHES.PC = BUS;
        }
        else{
            NEXT_LATCHES.PC = eval_adder();
        }
    }
    else{
        NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    }
}
//////////////////////////////////////////////////////////


void eval_micro_sequencer() {
  /* 
   * Evaluate the address of the NEXT STATE according to the 
   * micro sequencer logic. Latch the next microinstruction.
   * Uses first 9 bits of microcoded instruction
   */
    mic = CURRENT_LATCHES.MICROINSTRUCTION;

    int cond = GetCOND(mic);
    int ird = GetIRD(mic);
    int j = GetJ(mic);

    if(ird == 1){
        int ns = CURRENT_LATCHES.IR >> 12;
        NEXT_LATCHES.STATE_NUMBER = ns;
    }
    else if(cond == 0){ //Unconditional
        NEXT_LATCHES.STATE_NUMBER = j;
    }
    else if(cond == 1){ //Memory Ready
        if(CURRENT_LATCHES.READY){
            NEXT_LATCHES.STATE_NUMBER = j + 2;
        }
        else{
            NEXT_LATCHES.STATE_NUMBER = j;
        }
    }
    else if(cond == 2){ //Branch
        if(CURRENT_LATCHES.BEN){
            NEXT_LATCHES.STATE_NUMBER = j + 4;
        }
        else{
            NEXT_LATCHES.STATE_NUMBER = j;
        }
    }
    else{               //Addressing Mode
        int IR_b[1];
        get_Binary(IR_b,11,11,CURRENT_LATCHES.IR);
        if(IR_b[0]){
            NEXT_LATCHES.STATE_NUMBER = j + 1;
        }
        else{
            NEXT_LATCHES.STATE_NUMBER = j;
        }
    }
    for(int i = 0; i < CONTROL_STORE_BITS; i++){
        NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
    }
    

}

//To count number of memory cycles
int Cmem_cyc = 1;
int Nmem_cyc = 1;

void cycle_memory() {
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
    Cmem_cyc = Nmem_cyc;
    int cond = GetCOND(mic);

    if(cond == 1){
        if(Cmem_cyc != 4){
            NEXT_LATCHES.READY = 0;
        }
        else if(Cmem_cyc == 4){
            //if R_W is 1 write to memory
            //Read is handled by MDR_latch()
            if(GetR_W(mic) == 1){
                int mar = CURRENT_LATCHES.MAR;
                int mdr = CURRENT_LATCHES.MDR;
                if(GetDATA_SIZE(mic) == 1){     //Word
                    MEMORY[mar/2][0] = mdr & 0x00FF;
                    MEMORY[mar/2][1] = (mdr & 0xFF00)>>8;
                }
                else{                           //Byte
                    if(mar & 0x0001){
                        MEMORY[mar/2][1] = (mdr & 0xFF00)>>8;
                    }
                    else{
                        MEMORY[mar/2][0] = mdr & 0x00FF;
                    }
                }
            }

            NEXT_LATCHES.READY = 1;
        }
        Nmem_cyc++;
        if(Cmem_cyc == 5) Nmem_cyc = 1;
    }
    else{
        NEXT_LATCHES.READY = 0;
        Nmem_cyc = 1;
    }
}


uint16_t Gmar,Gpc,Galu,Gshf,Gmdr; //values in gates 

void eval_bus_drivers() {
  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */  
    Gmar = getG_MARMUX();
    Gpc = getG_PC();
    Galu = getG_ALU();
    Gshf = getG_SHF();
    Gmdr = getG_MDR();
}


void drive_bus() {
  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       
    if(GetGATE_MARMUX(mic)){
        BUS = Gmar;
    }
    else if(GetGATE_PC(mic)){
        BUS = Gpc;
    }
    else if(GetGATE_ALU(mic)){
        BUS = Galu;
    }
    else if(GetGATE_SHF(mic)){
        BUS = Gshf;
    }
    else if(GetGATE_MDR(mic)){
        BUS = Gmdr;
    }
    else{
        BUS = 0;
    }

}


void latch_datapath_values() {
  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */
    MAR_latch();
    MDR_latch();
    IR_latch();
    BEN_latch();
    REG_latch();
    CC_latch();
    PC_latch();

}
