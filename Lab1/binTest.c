#include<stdlib.h>
#include<string.h>
#include<stdio.h>

int numBits;

int printBin(int*bin){
  printf("\nOffset is:  \n");
      for(int i = numBits-1; i>= 0; i--){
        printf("%d ",bin[i]);
      }
}

int validateOffset( int* bin){
  //TO DO return binary offset
  int isPos = 1;
  int offset;
  int num;


    numBits = 5;
    int range = 32;

    offset = -10;
    num = offset;
    
    if((offset >= -range) && (offset <= (range -1) )){
      if(num < 0){
        isPos = 0;
        num = num * (-1);
      }

      //Get positive binary number into array
      int index = numBits-1;
      for(int i = (range/2); i > 1; i = i/2){
        if(num / i == 1){
          bin[index] = 1;
          num = num - i;
        }
        printf("Num = %d\n", num);
        index--;
      }

      if(num == 1){
        bin[0] = 1;
      }
      printBin(bin);
      //Get 2's complement
      if(!isPos){

        //Flip all bits
        for(int i = numBits; i >= 0; i--){
          bin[i] = !bin[i];
        }
        printBin(bin);
        //Increas by 1
        int Cin = 1;
        for(int i = 0; i < numBits; i++){
          int temp = bin[i] + Cin;
          if(temp == 2){
            bin[i] = 0;
            Cin = 1;
          }
          else if(temp == 1){
            bin[i] = 1;
            Cin = 0;
          }
          else{
            bin[i] = 0;
            Cin = 0;
          }
          //printf("Bit %d is %d with Cout of %d",i,bin[i],Cin);
        }
      }
      //Print binary
      printBin(bin);
      printf("\n");
      return 1;
    }
    else{
      exit(4);
    }
}


int main(){

  int arr[] = {0,0,0,0,0,0};
  validateOffset(arr);

  return 1;  
}