//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��ILI9320�����ͼ򵥵�gui���ɱ���������������stm32��Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//

#include "Drivers.h"
#include "Lcd.h"
#if LCD_BGLIGHT_PWM_MODE
#include "Time.h"
#endif

#define LCD_ILI9320_On() GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define LCD_ILI9320_Off() GPIO_ResetBits(GPIOC,GPIO_Pin_8)

#define Bank1_LCD_R    ((uint32_t)0x60000000)    //disp Reg ADDR
#define Bank1_LCD_D    ((uint32_t)0x60020000)	//disp Data ADDR 

static u8 gLcdScale=100;

//9320���ܼĴ�����ַ
#define WINDOW_XADDR_START	0x0050 // ˮƽ�Ŀ�ʼ��ַ��
#define WINDOW_XADDR_END		0x0051 // ˮƽ�Ľ�����ַ��
#define WINDOW_YADDR_START	0x0052 // ��ֱ�Ŀ�ʼ��ַ��
#define WINDOW_YADDR_END		0x0053 // ��ֱ�Ľ�����ַ��
#define GRAM_XADDR		    		0x002A // GRAM ˮƽ�ĵ�ַ��
#define GRAM_YADDR		    		0x002B // GRAM ��ֱ�ĵ�ַ��
#define GRAMWR 			    			0x002C // Write GRAM
#define GRAMRD								0x002E // Read GRAM

/**********************************************
��������LCD_DelayMs
���ܣ�����LCD������ʱ
��ڲ�������ʱ��
����ֵ����
***********************************************/
static void LCD_DelayMs(u32 Ms)
{
  u32 i;
	for(; Ms; Ms--)
		for(i=1000;i;i--);
}

/*************************************************
��������LCD_WriteIndex
���ܣ�����Ĵ�����ַ
��ڲ������Ĵ�����ַ
����ֵ����
*************************************************/
static void LCD_WriteIndex(u16 index)
{
	*(__IO u16 *) (Bank1_LCD_R)= index;	
}

/*************************************************
��������LCD_WR_Reg
���ܣ���lcd�Ĵ�����д����
��ڲ������Ĵ�����ַ������
����ֵ���Ĵ���ֵ
*************************************************/
static void LCD_WriteReg(u16 index,u16 val)
{	
	*(__IO u16 *) (Bank1_LCD_R)= index;	
	*(__IO u16 *) (Bank1_LCD_D)= val;	
}

/*************************************************
��������LCD_RD_Reg
���ܣ���lcd�Ĵ�������ֵ
��ڲ������Ĵ�����ַ
����ֵ���Ĵ���ֵ
*************************************************/
static u16 LCD_ReadReg(u16 index)
{	
	*(__IO u16 *) (Bank1_LCD_R)= index;	
	return (*(__IO u16 *) (Bank1_LCD_D));
}

/*************************************************
��������LCD_WR_Data
���ܣ���lcdд����
��ڲ���������ֵ
����ֵ����
*************************************************/
static void LCD_WriteData(u16 val)
{   
	*(__IO u16 *) (Bank1_LCD_D)= val; 	
}

/*************************************************
��������LCD_RD_Data
���ܣ���lcd����
��ڲ�������
����ֵ������
*************************************************/
static u16 LCD_ReadData(void)
{
	return(*(__IO u16 *) (Bank1_LCD_D));	
}

/*************************************************
��������LCD_Power_On
���ܣ�LCD��������
��ڲ�������
����ֵ����
*************************************************/
static void LCD_PowerOn(void)
{
	u8 iddata[4];
	
    LCD_Reset();
    
    LCD_WriteIndex(0xD3);
    iddata[0] = LCD_ReadData();
    iddata[1] = LCD_ReadData();
    iddata[2] = LCD_ReadData();
    iddata[3] = LCD_ReadData();
    Debug("Lcd driver ic:%x%x%x%x\n\r",iddata[0],iddata[1],iddata[2],iddata[3]);
    
	LCD_WriteIndex(0xCF);
	LCD_WriteData(0x00);
	LCD_WriteData(0x81);
	LCD_WriteData(0x30);

	LCD_WriteIndex(0xED);
	LCD_WriteData(0x64);
	LCD_WriteData(0x03);
	LCD_WriteData(0x12);
	LCD_WriteData(0x81);

	LCD_WriteIndex(0xE8);
	LCD_WriteData(0x85);
	LCD_WriteData(0x10);
	LCD_WriteData(0x78);

	LCD_WriteIndex(0xCB);
	LCD_WriteData(0x39);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x00);
	LCD_WriteData(0x34);
	LCD_WriteData(0x02);

	LCD_WriteIndex(0xF7);
	LCD_WriteData(0x20);

	LCD_WriteIndex(0xEA);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);

	LCD_WriteIndex(0xB1);
	LCD_WriteData(0x00);
	LCD_WriteData(0x1B);

	LCD_WriteIndex(0xB6);
	LCD_WriteData(0x0A);
	LCD_WriteData(0xA2);

	LCD_WriteIndex(0xC0);
	LCD_WriteData(0x35);

	LCD_WriteIndex(0xC1);
	LCD_WriteData(0x11);

	LCD_WriteIndex(0xC5);
	LCD_WriteData(0x45);
	LCD_WriteData(0x45);

	LCD_WriteIndex(0xC7);
	LCD_WriteData(0xA2);

	LCD_WriteIndex(0xF2);
	LCD_WriteData(0x00);

	LCD_WriteIndex(0x26);
	LCD_WriteData(0x01);

	LCD_WriteIndex(0xE0); //Set Gamma
	LCD_WriteData(0x0F);
	LCD_WriteData(0x26);
	LCD_WriteData(0x24);
	LCD_WriteData(0x0B);
	LCD_WriteData(0x0E);
	LCD_WriteData(0x09);
	LCD_WriteData(0x54);
	LCD_WriteData(0xA8);
	LCD_WriteData(0x46);
	LCD_WriteData(0x0C);
	LCD_WriteData(0x17);
	LCD_WriteData(0x09);
	LCD_WriteData(0x0F);
	LCD_WriteData(0x07);
	LCD_WriteData(0x00);
	LCD_WriteIndex(0XE1); //Set Gamma
	LCD_WriteData(0x00);
	LCD_WriteData(0x19);
	LCD_WriteData(0x1B);
	LCD_WriteData(0x04);
	LCD_WriteData(0x10);
	LCD_WriteData(0x07);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x47);
	LCD_WriteData(0x39);
	LCD_WriteData(0x03);
	LCD_WriteData(0x06);
	LCD_WriteData(0x06);
	LCD_WriteData(0x30);
	LCD_WriteData(0x38);
	LCD_WriteData(0x0F);

	LCD_WriteIndex(0x36); //rbg order
	LCD_WriteData(0x08);

	LCD_WriteIndex(0X2A); 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0xEF);

	LCD_WriteIndex(0X2B); 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x01);
	LCD_WriteData(0x3F);

	LCD_WriteIndex(0x3a); // Memory Access Control
	LCD_WriteData(0x55);
	LCD_WriteIndex(0x11); //Exit Sleep
	
	LCD_DelayMs(120);
	LCD_WriteIndex(0x29); //display on
	LCD_WriteIndex(0x2c); 

    return;
}





/*************************************************
��������LCD_Power_Off
���ܣ�LCD�ر�����
��ڲ�������
����ֵ����
*************************************************/
static void LCD_PowerOff(void)
{
	return;
}

/*************************************************
��������LCD_WR_Data_Start
���ܣ�LCD��ʼ����������ǰ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_BlukWriteDataStart(void)
{	
    LCD_WriteIndex(GRAMWR);
}

/*************************************************
��������LCD_ReadDataStart
���ܣ�LCD��ʼ����������ǰ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_BulkReadDataStart(void)
{	
    LCD_WriteIndex(GRAMRD);
}

/*************************************************
��������LCD_BulkWriteData
���ܣ���lcd����д����
��ڲ���������ֵ
����ֵ����
*************************************************/
void LCD_BulkWriteData(u16 val)
{   
	*(__IO u16 *) (Bank1_LCD_D)= val; 	
}

/*************************************************
��������LCD_BulkReadData
���ܣ�������lcd����
��ڲ�������
����ֵ������
*************************************************/
u16 LCD_BulkReadData(void)//4???
{
  register u16 Data;
  LCD_ReadData();//���������ֽ�
  Data=LCD_ReadData();
  Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB����
  LCD_WriteData(Data);
  return Data;
}

/*************************************************
��������LCD_Set_XY
���ܣ�����lcd��ʾ��ʼ��
��ڲ�����xy����
����ֵ����
*************************************************/
void LCD_SetXY(u16 x,u16 y)
{
  LCD_WriteIndex(GRAM_XADDR);
  LCD_WriteData(x>>8);
  LCD_WriteData(x&0xFF);
  LCD_WriteIndex(GRAM_YADDR);
  LCD_WriteData(y>>8);
  LCD_WriteData(y&0xFF);
  //LCD_WriteIndex(GRAMWR);
}

/*************************************************
��������LCD_Set_Region
���ܣ�����lcd��ʾ�����ڴ�����д�������Զ�����
��ڲ�����xy�����յ�,Y_IncMode��ʾ������y������x
����ֵ����
*************************************************/
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist)
{		
	LCD_WriteIndex(0X2A);  //column address set 
	LCD_WriteData(x_start >> 8);     //set the head of column address
	LCD_WriteData(x_start & 0xff);
	LCD_WriteData(x_end >> 8);     //set the end of column address
	LCD_WriteData(x_end & 0xff);
	LCD_WriteIndex(0X2B);  //column address set 
	LCD_WriteData(y_start >> 8);     //set the head of column address
	LCD_WriteData(y_start & 0xff);
	LCD_WriteData(y_end >> 8);     //set the end of column address
	LCD_WriteData(y_end & 0xff);
  
  	if(0){//4???
		u16 ModeReg=LCD_ReadReg(0x0003);

		if(yIncFrist)
			ModeReg|=(0x8);
		else
			ModeReg&=(~0x8);
		LCD_WriteReg(0x0003, ModeReg);
	}
}

/*************************************************
��������LCD_Set_XY_Addr_Direction
���ܣ�����lcd����д�����ķ���
��ڲ�����0:��0���ߣ�1:�ɸߵ�0
����ֵ����
*************************************************/
static void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection,bool yIncFirst)
{
	u16 Mode;

	LCD_WriteIndex(RDDMADCTL);
	LCD_ReadData();
	Mode = LCD_ReadData();
	Mode &=0x1F;
	
	switch(xyDirection)
	{
		case xInc_yInc:
			Mode|=0x00;
			break;
		case xInc_yDec:
			Mode|=0x80;
			break;
		case xDec_yInc:
			Mode|=0x40;
			break;
		case xDec_yDec:
			Mode|=0xC0;
			break; 
	}
	
	if(yIncFirst)
		Mode |=0x20;
		
	LCD_WriteIndex(MADCTL);
	LCD_WriteData(Mode); 
}


/*************************************************
��������LCD_BGR_Mode
���ܣ�����lcd RGB˳��
��ڲ�����0:RGB   1:BGR
����ֵ����
*************************************************/
void LCD_BgrMode(bool UseBGR)
{
	u8 Reg;
	
	LCD_WriteIndex(0x36);

	Reg=LCD_ReadData();
	//Debug("Mo:%x\n\r",Reg);
	
	if(UseBGR)
		LCD_WriteData(0x08|Reg);
	else
		LCD_WriteData(0x00|Reg);

		//Debug("Mo:%x\n\r",LCD_ReadData());
}

/*************************************************
��������LCD_Addr_Inc
���ܣ���ַ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_AddrInc(void)//4????
{
	register u16 Color16;
	//LCD_WriteIndex(GRAMRD);
	LCD_ReadData();
	Color16=LCD_ReadData();
	//LCD_WriteIndex(GRAMWR);
	LCD_WriteData((((Color16>>11)&0x001f)|(Color16&0x07e0)|((Color16<<11)&0xf800)));//��16λRGB(565)ɫ�ʻ����16λBGR(565)ɫ��
}

/*************************************************
��������LCD_DrawPoint
���ܣ���һ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 Data)
{
	LCD_WriteIndex(GRAM_XADDR);
	LCD_WriteData(x>>8);
	LCD_WriteData(x&0xFF);
	LCD_WriteIndex(GRAM_YADDR);
	LCD_WriteData(0x00);
	LCD_WriteData(y&0xFF);
	LCD_WriteIndex(GRAMWR);   
	LCD_WriteData(Data);  
}

/*************************************************
��������LCD_DrawPoint
���ܣ���һ����
��ڲ�������
����ֵ����
*************************************************/
u16 LCD_ReadPoint(u16 x,u16 y)
{
	register u16 Data;
	LCD_WriteIndex(GRAM_XADDR);
	LCD_WriteData(x>>8);
	LCD_WriteData(x&0xFF);
	LCD_WriteIndex(GRAM_YADDR);
	LCD_WriteData(0x00);
	LCD_WriteData(y&0xFF);
	LCD_WriteIndex(GRAMRD);   
	LCD_ReadData();
	Data=LCD_ReadData();
	Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB����
	//4???? LCD_WriteData(Data);//�����������������������˾�
	return Data;
}

/*************************************************
��������LCD_Light_Set
���ܣ�LCD���ñ�������
��ڲ�����Scale:0-100��0ΪϨ��100����
����ֵ����
*************************************************/
void LCD_Light_Set(u8 Scale)
{
	Debug("LCD Light Set:%d\n\r",Scale);
#if LCD_BGLIGHT_PWM_MODE
	Tim3_PWM(Scale);
#else
	if(Scale) GPIO_SetBits(GPIOC,GPIO_Pin_6);
	else GPIO_ResetBits(GPIOC,GPIO_Pin_6);
#endif
	gLcdScale=Scale;
}

/*************************************************
��������LCD_Light_State
����:��ѯ��ǰ��������
��ڲ�������
����ֵ��0-100��0ΪϨ��100����
*************************************************/
u8 LCD_Light_State(void)
{
	return gLcdScale;
}

/**********************************************
��������FSMC_LCD_Init
���ܣ�����FSMC����
��ڲ�������
����ֵ����
***********************************************/
static void LCD_FSMC_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE); 
  
  p.FSMC_AddressSetupTime = 0x02;
  p.FSMC_AddressHoldTime = 0x00;
  p.FSMC_DataSetupTime = 0x05;
  p.FSMC_BusTurnAroundDuration = 0x00;
  p.FSMC_CLKDivision = 0x00;
  p.FSMC_DataLatency = 0x00;
  p.FSMC_AccessMode = FSMC_AccessMode_B;
    
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;	  

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}

/**********************************************
��������LCD_Configuration
���ܣ�����LCD��IO����
��ڲ�������ʱ��
����ֵ����
***********************************************/
static void LCD_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE, ENABLE); 	

  //����ic�������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //LCD Reset
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

#if LCD_BGLIGHT_PWM_MODE //LCD �������,	  
  /*����PC6 PWM*/
  //pc6����tim3 ch1��remap
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //TIM3 ��ȫ��ӳ�� 
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

  //FSMC��GPIOD�ܽ�
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_14 	//D0
  												| GPIO_Pin_15 		//D1
  												| GPIO_Pin_0		//D2
  												| GPIO_Pin_1		//D3
  												| GPIO_Pin_8 		//D13
  												| GPIO_Pin_9 		//D14
  												| GPIO_Pin_10 		//D15  		
  												| GPIO_Pin_7		//NE1
  												| GPIO_Pin_11		//RS
  												| GPIO_Pin_4		//nRD
  												| GPIO_Pin_5;		//nWE
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  //FSMC��GPIOE�ܽ�
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_7		//D4
  												| GPIO_Pin_8 		//D5
  												| GPIO_Pin_9 		//D6
  												| GPIO_Pin_10 		//D7
  												| GPIO_Pin_11 		//D8
  												| GPIO_Pin_12 		//D9
  												| GPIO_Pin_13		//D10
  												| GPIO_Pin_14 		//D11
  												| GPIO_Pin_15; 	//D12  												
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

/*************************************************
��������LCD_Init
���ܣ���ʼ������lcd
��ڲ�������
����ֵ����
*************************************************/
void LCD_Init(void)
{
	LCD_FSMC_Init();
	LCD_DelayMs(100);	 
	LCD_Configuration();
	LCD_ILI9320_On();
	LCD_DelayMs(100);
	LCD_Light_Set(100);
	LCD_PowerOn();
	LCD_BgrMode(FALSE);

	LCD_SetAddrIncMode(xDec_yInc,TRUE);
}

void LCD_DeInit(void)
{
	LCD_PowerOff();
}

/**********************************************
��������LCD_Reset
���ܣ�LCD��λ
��ڲ�������ʱ��
����ֵ����
***********************************************/ 
void LCD_Reset(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
    LCD_DelayMs(50);					   
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 	 
	LCD_DelayMs(500);	
}


