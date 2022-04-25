// prototype
void blink(void);

#define SLAVE_ADDRESS_LCD 0x3F<<1 // adres przesunięty dla MasterTransmit()

// lcd library
int lcd_send_cmd (char cmd){
	HAL_Delay(1);
	uint8_t data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0 1100
	data_t[1] = data_u|0x08;  //en=0, rs=0 1000
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	return HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

int lcd_checkCon(void){
	return HAL_I2C_IsDeviceReady(&hi2c1, SLAVE_ADDRESS_LCD, 1 , 10);
}

void lcd_clear(){
	lcd_send_cmd(0x01);
}

void lcd_line(int n){
	n %= 4;
	int addr;
	switch(n){
		case 0: addr = 0x00;
		  break;
		case 1: addr = 0x40;
		  break;
		case 2: addr = 0x14;
		  break;
		case 3: addr = 0x54;
	}
	lcd_send_cmd(0x80+addr);
}

// 3bit: D,C,B
//D - display on/off
//C - cursor on/off
//B - cursor blink on/off
void lcd_display( int set ){
	lcd_send_cmd(0x08+set);
}

int  lcd_send_data (char data)
{
	HAL_Delay(1);
	uint8_t data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	return HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

void lcd_init (void)
{
	HAL_GPIO_WritePin(Test_LED_GPIO_Port, Test_LED_Pin, 0);

	// check connection
	if( HAL_I2C_IsDeviceReady(&hi2c1, SLAVE_ADDRESS_LCD, 1, 100 ) ){
		HAL_GPIO_TogglePin(Test_LED_GPIO_Port, Test_LED_Pin);
		return;
	}

	// 0x30 00,0011,0000 Function set --> 8-bit mode
	// 0x02 00,0010,0000 Function set --> 4-bit mode
	// 0x28 00,0010,1000 Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	// 0x08 00,0000,1000 Display on/off control --> D=0,C=0, B=0  ---> display off
	// 0x01 00,0000,0001 clear display
	// 0x06 00,0000,0110 Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	// 0x0C 00,0000,1100 Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	// 0x0F 00,0000,1111 Display on/off control --> blinking cursor

	// wait for lcd init
	HAL_Delay(200);

	int tmp;
	int i=0;
	uint8_t commands[] = {0x30,0x20,0x28,0x08,0x01,0x06,0x0F		,0x00};

	while( commands[i] > 0 ){

		if ( (tmp = lcd_send_cmd (commands[i++])) )
		{
			while( tmp-- > 0 ) blink();
			HAL_GPIO_TogglePin(Test_LED_GPIO_Port, Test_LED_Pin);
			return;
		}
	}
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
