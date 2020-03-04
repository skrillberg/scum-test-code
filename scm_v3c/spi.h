unsigned int read_gyro_x(void);
unsigned int read_gyro_y(void);
unsigned int read_gyro_z(void);

void spi_write(unsigned char writeByte);

unsigned char spi_read(void);

void spi_chip_select(void);

void spi_chip_deselect(void);

unsigned int read_acc_y(void);

unsigned int read_acc_z(void);

void test_imu_life(void);

unsigned char read_imu_register(unsigned char reg);

void write_imu_register(unsigned char reg, unsigned char data);
