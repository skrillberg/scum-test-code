#include <stdint.h>

#define USER_BANK_SEL	(0x7F)
#define USER_BANK_0		(0x00)
#define USER_BANK_1		(0x10)
#define USER_BANK_2		(0x20)
#define USER_BANK_3		(0x30)

#define PWR_MGMT_1 		(0x06)
#define PWR_MGMT_2		(0x07)
#define GYRO_CONFIG_1	(0x01)


#define CLK_BEST_AVAIL	(0x01)
#define GYRO_RATE_250	(0x00)
#define GYRO_LPF_17HZ 	(0x29)

void ICM_SelectBank(uint8_t bank);
void ICM_ReadMag(int16_t magn[3]);
uint8_t ICM_Mag_Read(uint8_t reg);
void i2c_Mag_write(uint8_t reg,uint8_t value);
void init_compass(void);