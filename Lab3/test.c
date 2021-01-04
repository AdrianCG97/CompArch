#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main(){
	int num = 0x1F21F42F;
	printf("num: %x \n",num);
	printf("num: %x \n",num & 0xFFFF);
	return 0;
}