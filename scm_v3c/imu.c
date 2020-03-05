#include "spi.h"
#include "imu.h"
#include <stdint.h>


void ICM_SelectBank(uint8_t bank) {
	write_imu_register(USER_BANK_SEL, bank);
}

void init_compass(){
		int i;
		write_imu_register(0x7F, 0x00); // Select user bank 0
		for(i=0; i<10000; i++);
		write_imu_register(0x0F, 0x30); //INT Pin / Bypass Enable Configuration
		for(i=0; i<10000; i++);
		write_imu_register(0x03, 0x20); //I2C_MST_EN
		for(i=0; i<10000; i++);
		write_imu_register(0x7F, 0x30); //select user bank 3
		for(i=0; i<10000; i++);
		write_imu_register(0x01, 0x4D); // I2C Master mode and Speed 400 kHz
		for(i=0; i<10000; i++);
		write_imu_register(0x02, 0x01); // I2C_SLV0 _DLY_ enable
		for(i=0; i<10000; i++);
		write_imu_register(0x05, 0x81); // enable IIC	and EXT_SENS_DATA==1 Byte
		for(i=0; i<10000; i++);

		// Initialize magnetometer
		i2c_Mag_write(0x32, 0x01); // Reset AK8963
		for(i=0; i<10000; i++);
		i2c_Mag_write(0x31, 0x02); // use i2c to set AK8963 working on Continuous measurement mode1 & 16-bit output
}

void i2c_Mag_write(uint8_t reg,uint8_t value)
  {
		int i;
  	write_imu_register(0x7F, 0x30);

		for(i=0; i<1000; i++);
  	write_imu_register(0x03 ,0x0C);//mode: write

		for(i=0; i<1000; i++);
  	write_imu_register(0x04 ,reg);//set reg addr

		for(i=0; i<1000; i++);
  	write_imu_register(0x06 ,value);//send value

		for(i=0; i<1000; i++);
  }

 uint8_t ICM_Mag_Read(uint8_t reg)
  {
  	uint8_t  Data;
		int i;
  	write_imu_register(0x7F, 0x30);
		for(i=0; i<1000; i++);
  	write_imu_register(0x03 ,0x0C|0x80);
		for(i=0; i<1000; i++);
  	write_imu_register(0x04 ,reg);// set reg addr
		for(i=0; i<1000; i++);
  	write_imu_register(0x06 ,0xff);//read
		for(i=0; i<1000; i++);
  	write_imu_register(0x7F, 0x00);
  	Data = read_imu_register(0x3B);
		for(i=0; i<1000; i++);
  	return Data;
  }

	/*
 *
 * Read magnetometer
 *
 */
void ICM_ReadMag(int16_t magn[3]) {
	uint8_t mag_buffer[10];

	      mag_buffer[0] =ICM_Mag_Read(0x01);

	      mag_buffer[1] =ICM_Mag_Read(0x11);
	  	  mag_buffer[2] =ICM_Mag_Read(0x12);
	  	  magn[0]=mag_buffer[1]|mag_buffer[2]<<8;
	    	mag_buffer[3] =ICM_Mag_Read(0x13);
	      mag_buffer[4] =ICM_Mag_Read(0x14);
	    	magn[1]=mag_buffer[3]|mag_buffer[4]<<8;
	  	 	mag_buffer[5] =ICM_Mag_Read(0x15);
	      mag_buffer[6] =ICM_Mag_Read(0x16);
	  		magn[2]=mag_buffer[5]|mag_buffer[6]<<8;

	     	i2c_Mag_write(0x31,0x01);
}




