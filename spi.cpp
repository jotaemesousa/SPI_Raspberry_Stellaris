#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

using namespace std;
 
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
 
typedef struct __attribute__((__packed__)ROSCASDataToSTELLARIS_
{
    int8_t v_linear;
    int8_t v_angular;
    
    uint8_t cmd;		// 0b <blinky_bit> <ask_data_bit> <HBridgeDriveMode> <ask_firmware_bit> <0> <0> <0> <start_stop_car_bit>
}ROSCASDataToSTELLARIS;


typedef struct __attribute__((__packed__))ROSCASDataFromSTELLARIS_
{
    int32_t left_encoder_count;
    int32_t right_encoder_count;
    uint8_t battery_voltage;
    uint8_t battery_current;

    uint8_t cmd_back;
    
}ROSCASDataFromSTELLARIS;
 
int main(int argc, char **argv) 
{
	int i,fd;
	char wr_buf[]={0xf1,0xf2,0xf3,0xf4};
	char rd_buf[10];
	
	fd = open("/dev/spidev0.0", O_RDWR);
		
	int cmd = 0, l = -1, a = -128;
	if(argc == 4)
	{
	    sscanf(argv[1], "%d", &l);
	    sscanf(argv[2], "%d", &a);
	    sscanf(argv[3], "%d", &cmd);
	}
	uint8_t t[]={0x20,0x21,0x22,0x23};
	
//          write(fd,t,1);
//          write(fd,t+1,1);
//         write(fd,t+2,1);
//         write(fd,t+3,1);
// // 	write(fd,t+4,1);
// 	
// 	usleep(200);
// 	uint8_t p_rec[20];
// 	
// 	for(int i = 0; i < 12; i++)
// 	{
// 	    read(fd, p_rec + i, 1);
// 	    printf("byte = %x\n" , p_rec[i]);
// 	}

    ROSCASDataToSTELLARIS cmd_vel;
    cmd_vel.v_linear = l;
    cmd_vel.v_angular = a;
    cmd_vel.cmd = cmd;
    uint8_t *p_valor = (uint8_t *)&cmd_vel;
    
    cout << sizeof(ROSCASDataToSTELLARIS) << endl;

    for(int i = 0; i < sizeof(ROSCASDataToSTELLARIS); i++)
    {
        write(fd,p_valor + i,1);
    }

    //usleep(100);
    
    ROSCASDataFromSTELLARIS received;
    received.left_encoder_count = 0;
    received.right_encoder_count = 0;
    received.battery_voltage = 0;
    received.battery_current = 0;
    received.cmd_back = 0;
    
    uint8_t *p_rec = (uint8_t *)&received;
    
    cout << sizeof(ROSCASDataFromSTELLARIS) << endl;
    
    if(cmd_vel.cmd != 0)
    {
      cout << "A receber .. " << endl;
      for(int i = 0; i < (int)sizeof(ROSCASDataFromSTELLARIS); i++)
      { 
	  read(fd, p_rec + i, 1);
      }
      cout << "left_encoder_count = " << (int)received.left_encoder_count << "\n right_encoder_count = " << (int)received.right_encoder_count << endl;
      cout << "battery_voltage = " << (int)received.battery_voltage << "\n battery_current = " << (int)received.battery_current << endl;
      cout << "cmd back = " << (int)received.cmd_back << endl;
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
