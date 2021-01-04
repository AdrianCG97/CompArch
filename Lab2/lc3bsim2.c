/*
    Adrian Cantu Garza
    Enrique Antunano
    UTEID 1: ac73538
    UTEID 2: era668
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
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
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

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

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

void process_ADD(uint16_t inst){
    uint16_t DR = (inst & 0x0E00) >> 9;
    uint16_t SR1 = (inst & 0x01C0) >> 6;
    uint16_t num1 = CURRENT_LATCHES.REGS[SR1];
    uint16_t num2 = 0;

    uint16_t A = inst & 0x0020;
    if(!A){//SR2  
      uint16_t SR2 = inst & 0x0007;
      num2 = CURRENT_LATCHES.REGS[SR2];
    }
    else{//Imm5
      num2 = inst & 0x001F;
      if(num2 & 0x0010){
        num2 = num2 | 0xFFF0; //sign extend if needed
      }
    }
    uint16_t sum = num1 + num2;
    NEXT_LATCHES.REGS[DR] = sum;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;

    if((sum & 0x8000)) NEXT_LATCHES.N = 1;
    else if(sum == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1; 

    NEXT_LATCHES.PC +=2;
    return;
}

void process_AND(uint16_t inst){
    uint16_t DR = (inst & 0x0E00) >> 9;
    uint16_t SR1 = (inst & 0x01C0) >> 6;
    uint16_t num1 = CURRENT_LATCHES.REGS[SR1];
    uint16_t num2 = 0;

    uint16_t A = inst & 0x0020;
    if(!A){//SR2  
      uint16_t SR2 = inst & 0x0007;
      num2 = CURRENT_LATCHES.REGS[SR2];
    }
    else{
      num2 = inst & 0x001F;
      if(num2 & 0x0010){
        num2 = num2 | 0xFFF0; //sign extend if needed
      }
    }

    uint16_t and = num1 & num2;
    NEXT_LATCHES.REGS[DR] = and;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;

    if((and & 0x8000)) NEXT_LATCHES.N = 1;
    else if(and == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1;
    NEXT_LATCHES.PC +=2;
    return;
}

void process_BR(uint16_t inst){
  int n = inst & 0x0800;
  int z = inst & 0x0400;
  int p = inst & 0x0200;

  if((n && CURRENT_LATCHES.N) || (z && CURRENT_LATCHES.Z) || (p && CURRENT_LATCHES.P)){
    uint16_t offset = inst & 0x01FF;

    if(offset & 0x0100){ //Sign extend
        offset = offset | 0xFF00;
    }

    offset = (offset << 1) + CURRENT_LATCHES.PC + 2;

    NEXT_LATCHES.PC = offset;
  }
  else{
     NEXT_LATCHES.PC +=2;
  }

  return;
}

void process_JMP(uint16_t inst){
  uint16_t BaseR = (inst & 0x01C0) >> 6;
  uint16_t jmpTo = CURRENT_LATCHES.REGS[BaseR];
  NEXT_LATCHES.PC = jmpTo;
  return;
}

void process_JSR(uint16_t inst){
  uint16_t A = inst & 0x0800;
  uint16_t adds = 0;
  NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC + 2;
  //JSR
  if(A){
    uint16_t PCoffset11 = inst & 0x07FF;
    if(inst & 0x0400){
      PCoffset11 = PCoffset11 | 0xF800; //Sign Extend
    }
    PCoffset11 = PCoffset11 << 1;
    adds = CURRENT_LATCHES.PC + PCoffset11 + 2;
  }//JSRR 
  else{
    uint16_t BaseR = (inst & 0x01C0) >> 6;
    adds = CURRENT_LATCHES.REGS[BaseR];
  }
  NEXT_LATCHES.PC = adds;
}

void process_LDB(uint16_t inst){
  uint16_t DR = (inst & 0x0E00) >> 9;
  uint16_t BaseR = (inst & 0x01C0) >> 6;
  uint16_t boffset6 = inst & 0x003F;
  uint16_t adds = 0;

  if(boffset6 & 0x0020){
    boffset6 = boffset6 | 0xFFC0; 
  }  

  adds = CURRENT_LATCHES.REGS[BaseR] + boffset6;
  uint16_t data = MEMORY[adds >> 1][adds & 0x0001];

  if(data & 0x0080){
    data = data | 0xFF00;
  }
  NEXT_LATCHES.REGS[DR] = data;
  //printf("Loading data %x at %x to register %d\n",data, adds,DR);
  //Set Conditon codes
  NEXT_LATCHES.N = 0;
  NEXT_LATCHES.Z = 0;
  NEXT_LATCHES.P = 0;

  if((data & 0x8000)) NEXT_LATCHES.N = 1;
  else if(data == 0) NEXT_LATCHES.Z = 1;
  else NEXT_LATCHES.P = 1;
  NEXT_LATCHES.PC +=2;
}

void process_LDW(uint16_t inst){
  uint16_t DR = (inst & 0x0E00) >> 9;
  uint16_t BaseR = (inst & 0x01C0) >> 6;
  uint16_t offset6 = inst & 0x003F;
  uint16_t adds = 0;

  if(offset6 & 0x0020){  //Sign extend offset
    offset6 = offset6 | 0xFFC0; 
  }  

  adds = CURRENT_LATCHES.REGS[BaseR] + (offset6 << 1);
  uint16_t data = (MEMORY[adds >> 1][1] << 8) + MEMORY[adds >> 1][0];
  NEXT_LATCHES.REGS[DR] = data;
  //printf("Offset is : %d\n", offset6);
  //printf("Loading data %x at %x to register %d\n",data, adds,DR);
  //Set Conditon codes
  NEXT_LATCHES.N = 0;
  NEXT_LATCHES.Z = 0;
  NEXT_LATCHES.P = 0;

  if((data & 0x8000)) NEXT_LATCHES.N = 1;
  else if(data == 0) NEXT_LATCHES.Z = 1;
  else NEXT_LATCHES.P = 1;
  NEXT_LATCHES.PC +=2;
}

void process_LEA(uint16_t inst){
  uint16_t DR = (inst & 0x0E00) >> 9;
  uint16_t PCoffset9 = inst & 0x01FF;

  if(PCoffset9 & 0x0100){
    PCoffset9 = PCoffset9 & 0xFE00;
  }

  NEXT_LATCHES.PC += 2;
  uint16_t data = NEXT_LATCHES.PC + (PCoffset9 << 1);
  NEXT_LATCHES.REGS[DR] = data;
}

void process_TRAP(uint16_t inst){
  uint16_t trap = inst & 0x00FF;
  NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC + 2;
  NEXT_LATCHES.PC = (MEMORY[trap][1] << 8) + MEMORY[trap][0];
}

void process_SHF(uint16_t inst){
    uint16_t DR = (inst & 0x0E00) >> 9;
    uint16_t SR = (inst & 0x01C0) >> 6;
    uint16_t SRcont = CURRENT_LATCHES.REGS[SR];
    int16_t sSRcont = CURRENT_LATCHES.REGS[SR];
    uint16_t Amt = (inst & 0x000F);
    uint16_t SHF;

    uint16_t A = (inst & 0x0030) >> 4;

    if(A == 0){
        SHF = SRcont << Amt;
    } else if(A==1){
        SHF = SRcont >> Amt;
    } else if(A==3){
        SHF = sSRcont >> Amt;
    } else {                      // Should not occur
        SHF = SRcont;
    }

    NEXT_LATCHES.REGS[DR] = SHF;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;

    if (SHF & 0x8000) NEXT_LATCHES.N = 1;
    else if  (SHF == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1;

    NEXT_LATCHES.PC += 2;
    return;
}

void process_STB(uint16_t inst){
    uint16_t SR = (inst & 0x0E00) >> 9;
    uint16_t SRdata = CURRENT_LATCHES.REGS[SR];
    uint16_t BaseR = (inst & 0x01C0) >> 6;
    uint16_t Basedata = CURRENT_LATCHES.REGS[BaseR];
    uint16_t bOffset6 = (inst & 0x003F);

    if (bOffset6 & 0x0020) bOffset6 = bOffset6 | 0xFFC0;    // Sign Extending

    Basedata = Basedata + bOffset6;
    SRdata = SRdata & 0x00FF;                               // Bit masking

    MEMORY[Basedata/2][Basedata & 0x0001] = SRdata;

    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;

    if (SRdata & 0x0080) NEXT_LATCHES.N = 1;
    else if  (SRdata == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1;

    NEXT_LATCHES.PC += 2;
}

void process_STW(uint16_t inst){
    uint16_t SR = (inst & 0x0E00) >> 9;
    uint16_t SRdata = CURRENT_LATCHES.REGS[SR];
    uint16_t BaseR = (inst & 0x01C0) >> 6;
    uint16_t Basedata = CURRENT_LATCHES.REGS[BaseR];
    uint16_t bOffset6 = (inst & 0x003F);

    if (bOffset6 & 0x0020) bOffset6 = bOffset6 | 0xFFC0;    // Sign Extending

    uint16_t adds = Basedata + (bOffset6 * 2);
    MEMORY[adds/2][0] = SRdata & 0x00FF;
    MEMORY[adds/2][1] = (SRdata & 0xFF00) >> 8;

     NEXT_LATCHES.N = 0;
     NEXT_LATCHES.Z = 0;
     NEXT_LATCHES.P = 0;

    if (SRdata & 0x8000) NEXT_LATCHES.N = 1;
    else if  (SRdata == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1;

    NEXT_LATCHES.PC += 2;
}

void process_XORandNot(uint16_t inst){
    uint16_t DR = (inst & 0x0E00) >> 9;
    uint16_t DRdata = CURRENT_LATCHES.REGS[DR];
    uint16_t SR1 = (inst & 0x01C0)
 >> 6;
    uint16_t SR1data = CURRENT_LATCHES.REGS[SR1];

    uint16_t A = (inst & 0x0020);

    if (A == 0) {                                   // XOR[0]
        uint16_t SR2 = (inst & 0x0007);
        uint16_t SR2data = CURRENT_LATCHES.REGS[SR2];
        DRdata = SR1data ^ SR2data;
    } else {
        uint16_t imm5 = (inst & 0x001F);
        if (imm5 != 0x001F) {                       // XOR[1]
            if (!(imm5 & 0x0010)) DRdata = SR1data ^ imm5;
            else {
                DRdata = DRdata | 0xFFE0;
                DRdata = SR1data ^ imm5;
            }
        } else DRdata = ~SR1data;                   // NOT
    }

    NEXT_LATCHES.REGS[DR] = DRdata;

    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;

    if (DRdata & 0x8000) NEXT_LATCHES.N = 1;
    else if  (DRdata == 0) NEXT_LATCHES.Z = 1;
    else NEXT_LATCHES.P = 1;

    NEXT_LATCHES.PC += 2;
}


void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */    
  //FETCH
  uint16_t inst = (MEMORY[CURRENT_LATCHES.PC >> 1][1] << 8) + MEMORY[CURRENT_LATCHES.PC >> 1][0];

  //DECODE
  uint16_t opCode = (inst & 0xF000)>> 12;

  if(opCode == 1)       process_ADD(inst);   //ADD
  else if(opCode == 5)  process_AND(inst);   //AND
  else if(opCode == 0)  process_BR(inst);    //BR
  else if(opCode == 12) process_JMP(inst);   //JMP and RET
  else if(opCode == 4)  process_JSR(inst);   //JSR and JSRR
  else if(opCode == 2)  process_LDB(inst);   //LDB
  else if(opCode == 6)  process_LDW(inst);   //LDW
  else if(opCode == 14) process_LEA(inst);   //LEA
  else if(opCode == 15) process_TRAP(inst);  //TRAP
  else if (opCode == 13) process_SHF(inst);  //LSHF, RSHFL, RSHFA
  else if (opCode == 3) process_STB(inst);   // STB
  else if (opCode == 7) process_STW(inst);   // STW
  else if (opCode == 9) process_XORandNot(inst); // XOR, NOT
  
}