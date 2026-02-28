
#include "user.h"

SPL06 spl06;
SPL06_Calib_Param spl06_calib_param;


/***************************************************************************
*	函 数 名:	spl06_write_reg
*	功能说明:  IIC写
*	形   参: 无
*	返 回 值:	无
**************************************************************************/
uint8_t spl06_write_reg(uint8_t reg_addr,uint8_t reg_val)
{
	IIC_Start();
	IIC_Send_Byte((SP06_Advice_Address<<1)|0);//发送器件地址+写命令
	if(IIC_Wait_Ack()) //等待应答
	{
	IIC_Stop();
	return 1;
	}
	IIC_Send_Byte(reg_addr); //写寄存器地址
	IIC_Wait_Ack(); //等待应答
	IIC_Send_Byte(reg_val);//发送数据
	if(IIC_Wait_Ack()) //等待ACK
	{
	IIC_Stop();
	return 1;
	}
	IIC_Stop();
	return 0;
}

//IIC读SPL06寄存器
uint8_t spl06_read_reg(uint8_t reg_addr)
{
	uint8_t res;
	IIC_Start();
	IIC_Send_Byte((SP06_Advice_Address<<1)|0);//发送器件地址+写命令
	IIC_Wait_Ack(); //等待应答

	IIC_Send_Byte(reg_addr); //写寄存器地址
	IIC_Wait_Ack(); //等待应答

	IIC_Start();
	IIC_Send_Byte((SP06_Advice_Address<<1)|1);//发送器件地址+读命令
	IIC_Wait_Ack(); //等待应答

	res=IIC_Read_Byte(0);//读取数据,发送nACK
	IIC_Stop(); //产生一个停止条件
	return res;
}

//连续读SPL06寄存器
uint8_t spl06_read_buffer(uint8_t reg_addr,uint8_t *buffer,uint16_t len)
{
	IIC_Start();
	IIC_Send_Byte((SP06_Advice_Address<<1)|0);//发送器件地址+写命令
	if(IIC_Wait_Ack()) //等待应答
	{
	IIC_Stop();
	return 1;
	}
	 IIC_Send_Byte(reg_addr); //写寄存器地址
	 IIC_Wait_Ack(); //等待应答
	 IIC_Start();
	IIC_Send_Byte((SP06_Advice_Address<<1)|1);//发送器件地址+读命令
	 IIC_Wait_Ack(); //等待应答
	while(len)
	{
	if(len==1)*buffer=IIC_Read_Byte(0);//读数据,发送nACK
	else *buffer=IIC_Read_Byte(1); //读数据,发送ACK
	len--;
	buffer++;
	}
	 IIC_Stop(); //产生一个停止条件
	return 0;
}

//
void spl06_start(uint8_t mode)
{
 spl06_write_reg(SP06_MEAS_CFG, mode);//测量模式配置
}


void spl06_config_temperature(uint8_t rate,uint8_t oversampling)
{
	uint8_t data;
	data = (1<<7)|rate|oversampling;
	if(oversampling > TMP_PRC_8)//过采样次数大于EMPERATURE_RATE_8_TIMES，应当允许数据被新的数据覆盖，内部拥有气压和温度共32级的FIFO，在大于8次（也就是大于或等于16次过采样）的时候需要被新的数据覆盖，否则数据就会丢失
	{
		uint8_t data;
		data = spl06_read_reg(SP06_CFG_REG);//读取原寄存器值
		data |= SPL06_CFG_T_SHIFT;//P-SHIFT位置1
		spl06_write_reg(SP06_CFG_REG, data);//重新写回寄存器
	}

	 switch(oversampling)
	 {
		case TMP_PRC_1:
			spl06.i32kT = 524288;
			break;
		case TMP_PRC_2:
			spl06.i32kT = 1572864;
			break;
		case TMP_PRC_4:
			spl06.i32kT = 3670016;
			break;
		case TMP_PRC_8:
			spl06.i32kT = 7864320;
			break;
		case TMP_PRC_16:
			spl06.i32kT = 253952;
			break;
		case TMP_PRC_32:
			spl06.i32kT = 516096;
			break;
		case TMP_PRC_64:
			spl06.i32kT = 1040384;
			break;
		case TMP_PRC_128:
			spl06.i32kT = 2088960;
			break;
	 }
	 spl06_write_reg(SP06_TMP_CFG, data);//重新写回寄存器
}



void spl06_config_pressure(uint8_t rate,uint8_t oversampling)//设置补偿系数及采样速率
{

	uint8_t data;
	data = rate|oversampling;
	if(oversampling > PM_PRC_8)//过采样次数大于EMPERATURE_RATE_8_TIMES，应当允许数据被新的数据覆盖，内部拥有气压和温度共32级的FIFO，在大于8次（也就是大于或等于16次过采样）的时候需要被新的数据覆盖，否则数据就会丢失
	{
		uint8_t data;
		data = spl06_read_reg(SP06_CFG_REG);//读取原寄存器值
		data |= SPL06_CFG_P_SHIFT;//P-SHIFT位置1
		spl06_write_reg(SP06_CFG_REG, data);//重新写回寄存器
	}

	 switch(oversampling)
	 {
		case PM_PRC_1:
			spl06.i32kP = 524288;
			break;
		case PM_PRC_2:
			spl06.i32kP = 1572864;
			break;
		case PM_PRC_4:
			spl06.i32kP = 3670016;
			break;
		case PM_PRC_8:
			spl06.i32kP = 7864320;
			break;
		case PM_PRC_16:
			spl06.i32kP = 253952;
			break;
		case PM_PRC_32:
			spl06.i32kP = 516096;
			break;
		case PM_PRC_64:
			spl06.i32kP = 1040384;
			break;
		case PM_PRC_128:
			spl06.i32kP = 2088960;
			break;
	 }
	 spl06_write_reg(SP06_PSR_CFG, data);   //写入配置
}


int32_t spl06_get_pressure_adc(void)//获取压力ADC值
{
	 uint8_t buf[3];
	 int32_t adc;
	 buf[0] = spl06_read_reg(SP06_PSR_B2);
	 buf[1] = spl06_read_reg(SP06_PSR_B1);
	 buf[2] = spl06_read_reg(SP06_PSR_B0);
	 adc = (int32_t)buf[0]<<16 | (int32_t)buf[1]<<8 | (int32_t)buf[2];
	 adc = (adc&0x800000)?(0xFF000000|adc):adc;
	 return adc;
}


int32_t spl06_get_temperature_adc(void)//获取温度ADC值
{
	 uint8_t buf[3];
	 int32_t adc;
	 buf[0] = spl06_read_reg(SP06_TMP_B2);
	 buf[1] = spl06_read_reg(SP06_TMP_B1);
	 buf[2] = spl06_read_reg(SP06_TMP_B0);
	 adc = (int32_t)buf[0]<<16 | (int32_t)buf[1]<<8 | (int32_t)buf[2];
	 adc = (adc&0x800000)?(0xFF000000|adc):adc;
	 return adc;
}


uint8_t spl06_update(int32_t *Temp,int32_t *Press)//获取并计算出温度值、气压值
{
	float Praw_src=0,Traw_src=0;
	 float qua2=0, qua3=0;
	 Traw_src = spl06_get_temperature_adc()/(float_t)spl06.i32kT;
	 Praw_src = spl06_get_pressure_adc()/(float_t)spl06.i32kP ;
	//计算温度
	 *Temp = 0.5*spl06_calib_param.c0 + spl06_calib_param.c1* Traw_src;
	 //计算气压
	 qua2 =  spl06_calib_param.c10 + Praw_src * (spl06_calib_param.c20 + Praw_src*spl06_calib_param.c30);
	 qua3 = Traw_src * Praw_src * (spl06_calib_param.c11 + Praw_src * spl06_calib_param.c21);
	 *Press = spl06_calib_param.c00 + Praw_src * qua2 + Traw_src * spl06_calib_param.c01 + qua3;
	 return 0;
}

//获取传感器数据就位状态，传感器就绪状态
uint8_t spl06_get_measure_status(void)
{
	return spl06_read_reg(SP06_MEAS_CFG);
}

//设置读取模式+读取方式
void spl06_set_measure_mode(uint8_t mode)  //参数为模式值
{
	 spl06_write_reg(SP06_MEAS_CFG, mode);
}
//启动命令模式读取温度值
void spl06_start_temperature(void)
{
	spl06_write_reg(SP06_MEAS_CFG, MEAS_CTRL_TempMeasure);
}
//启动命令模式读取气压值
void spl06_start_pressure(void)
{
	spl06_write_reg(SP06_MEAS_CFG, MEAS_CTRL_PressMeasure);
}
//进入待机模式，进入后停止采集数据直到再次切换模式
void spl06_enter_standby(void)
{
	spl06_write_reg(SP06_MEAS_CFG, MEAS_CTRL_Standby);
}


void spl06_get_calib_param()
{
	unsigned long h;
	unsigned long m;
	unsigned long l;
	h =  spl06_read_reg(0x10);
	l  =  spl06_read_reg(0x11);
	spl06_calib_param.c0 = (int16_t)h<<4 | l>>4;
	spl06_calib_param.c0 = (spl06_calib_param.c0&0x0800)?(0xF000|spl06_calib_param.c0):spl06_calib_param.c0;

	h =  spl06_read_reg(0x11);
	l  =  spl06_read_reg(0x12);
	spl06_calib_param.c1 = (int16_t)(h&0x0F)<<8 | l;
	spl06_calib_param.c1 = (spl06_calib_param.c1&0x0800)?(0xF000|spl06_calib_param.c1):spl06_calib_param.c1;
	h =  spl06_read_reg(0x13);
	m =  spl06_read_reg(0x14);
	l =  spl06_read_reg(0x15);
	spl06_calib_param.c00 = (int32_t)h<<12 | (int32_t)m<<4 | (int32_t)l>>4;
	spl06_calib_param.c00 = (spl06_calib_param.c00&0x080000)?(0xFFF00000|spl06_calib_param.c00):spl06_calib_param.c00;
	h =  spl06_read_reg(0x15);
	m =  spl06_read_reg(0x16);
	l =  spl06_read_reg(0x17);
	spl06_calib_param.c10 = (int32_t)h<<16 | (int32_t)m<<8 | l;
	spl06_calib_param.c10 = (spl06_calib_param.c10&0x080000)?(0xFFF00000|spl06_calib_param.c10):spl06_calib_param.c10;
	h =  spl06_read_reg(0x18);
	l  =  spl06_read_reg(0x19);
	spl06_calib_param.c01 = (int16_t)h<<8 | l;
	h =  spl06_read_reg(0x1A);
	l  =  spl06_read_reg(0x1B);
	spl06_calib_param.c11 = (int16_t)h<<8 | l;
	h =  spl06_read_reg(0x1C);
	l  =  spl06_read_reg(0x1D);
	spl06_calib_param.c20 = (int16_t)h<<8 | l;
	h =  spl06_read_reg(0x1E);
	l  =  spl06_read_reg(0x1F);
	spl06_calib_param.c21 = (int16_t)h<<8 | l;
	h =  spl06_read_reg(0x20);
	l  =  spl06_read_reg(0x21);
	spl06_calib_param.c30 = (int16_t)h<<8 | l;

}



uint8_t spl06_init(void)
{

	 uint8_t spl06_start_status;
	 if(spl06_write_reg(SP06_RESET,0x89))
	 {
		 return 1;
	 }
	 delay_ms(100); //复位后系数准备好需要至少40ms

	 do
		 spl06_start_status = spl06_get_measure_status();    		//读取气压计启动状态
	 while((spl06_start_status&MEAS_COEF_RDY)!=MEAS_COEF_RDY);

	 //读取内部校准值
	 spl06_get_calib_param();

	 do
		 spl06_start_status = spl06_get_measure_status();//读取气压计启动状态
	 while((spl06_start_status&MEAS_SENSOR_RDY)!= MEAS_SENSOR_RDY);

	 spl06.chip_id = spl06_read_reg(SP06_ID);
	 if((spl06.chip_id & 0xf0) != SP06_COEF )
	 {
		 return 2;
	 }

	 spl06_config_pressure(PM_RATE_128,PM_PRC_8);
	 spl06_config_temperature(TMP_RATE_128,TMP_PRC_8);

	 spl06_start(MEAS_CTRL_ContinuousPressTemp); //启动连续的气压温度测量
	 return 0;
}


//通过气压计算高度
int32_t Caculate_height(int32_t GasPress)
{
	int32_t Altitude=0;
	Altitude =(44330.0 *(1.0-pow((float)(GasPress) / 101325.0,1.0/5.255)))*1000;//单位mm
	return Altitude;
}
