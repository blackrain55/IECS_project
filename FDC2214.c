
//FDC2214.c
#include "fdc2214.h"
#include "delay.h"
#include "led.h"

u32 Data_FDC;
void FDC_IIC_Delay(void)
{
    delay_us(2);
}

void FDC_GPIO_Init(void)
{

}

void FDC_IIC_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//First enable peripheral IO portc clock

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;//Port configuration
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //Push-pull output
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        //IO velocity is 50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);                        //Initialize GPIO according to the set parameters

  GPIO_SetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);                  //PC.4, PC.5 output high
}

//Generate IIC start signal
void FDC_IIC_Start(void)
{
    FDC_SDA_OUT();     //SDA line output
    FDC_IIC_SDA=1;
    FDC_IIC_SCL=1;
    FDC_IIC_Delay();
     FDC_IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    FDC_IIC_Delay();
    FDC_IIC_SCL=0;//Celese I2C bus, prepare to send or receive data
}
//Generate IIC stop signal
void FDC_IIC_Stop(void)
{
    FDC_SDA_OUT();//SDA line output
    FDC_IIC_SCL=0;
    FDC_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
     FDC_IIC_Delay();
    FDC_IIC_SCL=1;
    FDC_IIC_SDA=1;//Send I2C bus end signal
    FDC_IIC_Delay();
}
//Waiting for the response signal arrival
//Return value: 1. Receive response failed
//                0, the reception response is successful
u8 FDC_IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    FDC_SDA_IN();      //SDA is set to input
    FDC_IIC_SDA=1;FDC_IIC_Delay();
    FDC_IIC_SCL=1;FDC_IIC_Delay();
    while(FDC_READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            FDC_IIC_Stop();
            return 1;
        }
    }
    FDC_IIC_SCL=0;//Clock output 0
    return 0;
}


//Produce an ACK response
//void FDC_IIC_Ack(void)
//{
//    FDC_IIC_SCL=0;
//    FDC_SDA_OUT();
//    FDC_IIC_SDA=0;
//    FDC_IIC_Delay();
//    FDC_IIC_SCL=1;
//    FDC_IIC_Delay();
//    FDC_IIC_SCL=0;
//}

//debug
void FDC_IIC_Ack(void)
{
    FDC_IIC_SCL=0;
    FDC_IIC_SDA=0;
    FDC_IIC_Delay();
    FDC_IIC_SCL=1;
    FDC_IIC_Delay();
    FDC_IIC_SCL=0;
    FDC_IIC_Delay();
    FDC_IIC_SDA=1;
}


//Do not produce an ACK response
void FDC_IIC_NAck(void)
{
    FDC_IIC_SCL=0;
    FDC_SDA_OUT();
    FDC_IIC_SDA=1;
    FDC_IIC_Delay();
    FDC_IIC_SCL=1;
    FDC_IIC_Delay();
    FDC_IIC_SCL=0;
}
//IIC sends a byte
//Back to the slave has no answer
//1, have a response
//0, no answer
void FDC_IIC_Send_Byte(u8 txd)
{
  u8 t;
    FDC_SDA_OUT();
    FDC_IIC_SCL=0;//Pull down clock start data transmission
    for(t=0;t<8;t++)
    {
        FDC_IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
            FDC_IIC_SCL=1;
            FDC_IIC_Delay();
            FDC_IIC_SCL=0;
            FDC_IIC_Delay();
    }
}
//Read 1 byte, when ACK = 1, send ACK, ACK = 0, send NACK
u8 FDC_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    FDC_SDA_IN();//SDA is set to input
    for(i=0;i<8;i++ )
    {
        FDC_IIC_SCL=0;
        FDC_IIC_Delay();
        FDC_IIC_SCL=1;
        receive<<=1;
        if(FDC_READ_SDA)receive++;
        FDC_IIC_Delay();
    }
    if (!ack)
        FDC_IIC_NAck();//Send NACK
    else
        FDC_IIC_Ack(); //Send ACK
    return receive;
}

u8 Set_FDC2214(u8 reg,u8 MSB,u8 LSB)
{
    FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|0);//Send device address + write command
    if(FDC_IIC_Wait_Ack())    //Waiting for an answer
    {
        FDC_IIC_Stop();
        return 1;
    }
    FDC_IIC_Send_Byte(reg);//Write register address
    FDC_IIC_Wait_Ack();         //Waiting for an answer
    FDC_IIC_Send_Byte(MSB);  //Send data 1
    if(FDC_IIC_Wait_Ack())     //Waiting for ACK
    {
        FDC_IIC_Stop();
        return 1;
    }
    FDC_IIC_Send_Byte(LSB);  //Send data 2
    if(FDC_IIC_Wait_Ack())     //Waiting for ACK
    {
        FDC_IIC_Stop();
        return 1;
    }
    FDC_IIC_Stop();
    return 0;
}

u16 FDC_Read(u8 reg)
{
    u16 res;
   FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|0);//Send device address + write command
    FDC_IIC_Wait_Ack();        //Waiting for an answer
    FDC_IIC_Send_Byte(reg);    //Write register address
    FDC_IIC_Wait_Ack();        //Waiting for an answer
    FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|1);//Send device address + read command
    FDC_IIC_Wait_Ack();        //Waiting for an answer
    res=FDC_IIC_Read_Byte(1)<<8;//Read data, send ACK

//    FDC_IIC_Ack();
    res|=FDC_IIC_Read_Byte(0);//Read data, send NACK
    FDC_IIC_Stop();            //Generate a stop condition
    return res;
}

u32 FCD2214_ReadCH(u8 index)
{
    u32 result;
    switch(index)
    {
        case 0:
          result = FDC_Read(DATA_CH0)&0x0FFF;
          result = (result<<16)|(FDC_Read(DATA_LSB_CH0));
            break;
        case 1:
            result = FDC_Read(DATA_CH1)&0x0FFF;
          result = (result<<16)|(FDC_Read(DATA_LSB_CH1));
            break;
        case 2:
            result = FDC_Read(DATA_CH2)&0x0FFF;
          result = (result<<16)|(FDC_Read(DATA_LSB_CH2));
            break;
        case 3:
            result = FDC_Read(DATA_CH3)&0x0FFF;
          result = (result<<16)|(FDC_Read(DATA_LSB_CH3));
            break;
        default:break;
    }
    result =result&0x0FFFFFFF;
    return result;
}

/*FDC2214 initialization function
  * Return value: 0: Initialization is normal
  * 1: Not normal
 */
u8 FDC2214_Init(void)
{
    u16 res,ID_FDC2214;
    FDC_GPIO_Init();
    FDC_IIC_Init();
    res=FDC_Read(MANUFACTURER_ID);//Is the test device read is normal?
    ID_FDC2214 = FDC_Read(DEVICE_ID);

    printf("DEVICE_ID:0X%x\r\n",ID_FDC2214);
    if(res==0x5449)
    {
        //Set SET_FDC2214 Register
        Set_FDC2214(RCOUNT_CH0,0x34,0xFB);//Reference count conversion interval (t = (rcount_ch0 * 16) / frefx)
        Set_FDC2214(RCOUNT_CH1,0x34,0xFB);
        Set_FDC2214(RCOUNT_CH2,0x34,0xFB);
        Set_FDC2214(RCOUNT_CH3,0x34,0xFB);

        Set_FDC2214(SETTLECOUNT_CH0,0x00,0x1B);//The stable time before the conversion (t = (settlecount_chx * 16) / frefx)
        Set_FDC2214(SETTLECOUNT_CH1,0x00,0x1B);
        Set_FDC2214(SETTLECOUNT_CH2,0x00,0x1B);
        Set_FDC2214(SETTLECOUNT_CH3,0x00,0x1B);

        Set_FDC2214(CLOCK_DIVIDERS_C_CH0,0x20,0x02);//Select sensor frequency at 0.01 MHz ~ 10MHz
        Set_FDC2214(CLOCK_DIVIDERS_C_CH1,0x20,0x02);//Frefx = fclk = 43.4MHz / 2 (2 Division)
        Set_FDC2214(CLOCK_DIVIDERS_C_CH2,0x20,0x02);//CHx_REF_DIVIDER=2;CHx_FIN_SEL=2
        Set_FDC2214(CLOCK_DIVIDERS_C_CH3,0x20,0x02);//CHx_REF_DIVIDER=2;CHx_FIN_SEL=2

        Set_FDC2214(DRIVE_CURRENT_CH0,0x78,0x00);//0.146mA (sensor clock establishment + conversion time drive current)
        Set_FDC2214(DRIVE_CURRENT_CH1,0x78,0x00);
        Set_FDC2214(DRIVE_CURRENT_CH2,0x78,0x00);
        Set_FDC2214(DRIVE_CURRENT_CH3,0x78,0x00);

        Set_FDC2214(ERROR_CONFIG,0x00,0x00);//All misunderstandings

        Set_FDC2214(MUX_CONFIG,0xC2,0x0D);//Channel 0, 1, 2, 3; Select 10 MHz to the minimum setting of the oscillation frequency exceeding the oscillation slot, multi-channel, four channels

        Set_FDC2214(CONFIG,0x14,0x01);//Activation mode, use the internal oscillator to do the reference frequency, the INTB pin will be set with the status register
    }
    else return 1;

    return 0;
}

float Cap_Calculate(u8 chx)
{
    float Cap;
    Data_FDC = FCD2214_ReadCH(chx);
    Cap = 232021045.248/(Data_FDC);
    return (Cap*Cap);
}