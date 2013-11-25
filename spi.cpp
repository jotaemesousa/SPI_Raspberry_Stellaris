#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

using namespace std;
 
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
 
 
int main(int argc, char **argv) 
{
	int i,fd;
	char wr_buf[]={0xf1,0xf2,0xf3,0xf4};
	char rd_buf[10];;
 
	if (argc<2) {
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}
   	
	fd = open(argv[1], O_RDWR);
	if (fd<=0) { 
		printf("%s: Device %s not found\n", argv[0], argv[1]);
		exit(1);
	}
	int t[]={0x20,0x21,0x33,0x45};

	uint16_t valor = 256;

	uint8_t tab[5];
	memset(tab,0,5);
	memcpy(tab,&valor,2);
	write(fd, tab, 1);
	write(fd, tab + 1, 1);
	//write(fd,t,1);
	//write(fd,t+1,1);
	//write(fd,t+2,1);
	//write(fd,t+3,1);
	

	/*write(fd,wr_buff+1,1);
	write(fd,wr_buff+2,1);
	write(fd,wr_buff+3,1);*/
	
	/*
	if (write(fd, wr_buf, ARRAY_SIZE(wr_buf)) != ARRAY_SIZE(wr_buf))
		perror("Write Error");
	if (read(fd, rd_buf, ARRAY_SIZE(rd_buf)) != ARRAY_SIZE(rd_buf))
		perror("Read Error");
	else
		for (i=0;i<ARRAY_SIZE(rd_buf);i++) {
		printf("0x%02X ", rd_buf[i]);
		if (i%2)
			printf("\n");
	}*/
 
	close(fd);
	return 0;
}
