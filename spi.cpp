#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

using namespace std;
 
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
 
struct ROSCASDataToSTELLARIS
{
    int8_t v_linear;
    int8_t v_angular;
    uint8_t cmd;
};

struct ROSCASDataFromSTELLARIS
{
    uint8_t var1;
    uint8_t var2;
};

 
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
	
	int cmd = 0, l = -1, a = -128;
	if(argc == 5)
	{
	    sscanf(argv[2], "%d", &l);
	    sscanf(argv[3], "%d", &a);
	    sscanf(argv[4], "%d", &cmd);
	}
	int t[]={0x20,0x21,0x33,0x45};
	
	

    struct ROSCASDataToSTELLARIS cmd_vel;
    cmd_vel.v_linear = l;
    cmd_vel.v_angular = a;
    cmd_vel.cmd = cmd;
    uint8_t *p_valor = (uint8_t *)&cmd_vel;
    
    cout << sizeof(struct ROSCASDataToSTELLARIS) << endl;

    for(int i = 0; i < sizeof(struct ROSCASDataToSTELLARIS); i++)
    {
        write(fd,p_valor + i,1);
    }

    //usleep(100);
    
    struct ROSCASDataFromSTELLARIS received;
    received.var1 = 0;
    received.var2 = 0;
    uint8_t *p_rec = (uint8_t *)&received;
    
    cout << sizeof(struct ROSCASDataFromSTELLARIS) << endl;
    
    if(cmd_vel.cmd == 1)
    {
      cout << "A receber .. " << endl;
      for(int i = 0; i < (int)sizeof(struct ROSCASDataFromSTELLARIS); i++)
      { 
	  read(fd, p_rec + i, 1);
	  cout << "cenas\n" << endl;
	  printf("byte = %x\n" , p_rec[i]);
	
	  
	  cout << i  << endl;
      }
      
      
      
      
      cout << "recebeu var1 = " << (int)received.var1 << " e var2 = " << (int)received.var2 << endl;
    }
	
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
