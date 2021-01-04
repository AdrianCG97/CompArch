#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(){
	uint16_t x = 10;
	uint16_t y = x & 0x0010;
	
	printf("The num is %x\n",y);

	return 0;
}