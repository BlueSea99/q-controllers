//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��st7565�����ͼ򵥵�gui���ɱ���������������stm32��Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//
#include "Drivers.h"

#define LCD_DAT_GPIOx GPIOA

//IOOUT_LCD_RD,GPI_B, GPin0,
#define LCD_RD_GPIOx GPIOB
#define LCD_RD_PIN GPIO_Pin_0

//IOOUT_LCD_WR,GPI_B,  GPin1, 
#define LCD_WR_GPIOx GPIOB
#define LCD_WR_PIN GPIO_Pin_1

//IOOUT_LCD_RS,GPI_B,  GPin13, 
#define LCD_RS_GPIOx GPIOB
#define LCD_RS_PIN GPIO_Pin_13

//IOOUT_LCD_RESET,GPI_B,  GPin14, 
#define LCD_REST_GPIOx GPIOB
#define LCD_REST_PIN GPIO_Pin_14

//IOOUT_LCD_CS,GPI_B,  GPin15,
#define LCD_CS_GPIOx GPIOB
#define LCD_CS_PIN GPIO_Pin_15

#define PinSet(g,p) {g->BSRR = (p);}
#define PinClr(g,p) {g->BRR = (p);}

#define LCMD_DISP_ON 0xAF
#define LCMD_DISP_OFF 0xAE
#define LCMD_START_LINE_ADDR 0x40 //0x40-0x7f
#define LCMD_VC_ON 0x2C       //1010  
#define LCMD_VR_ON 0x2A  
#define LCMD_VF_ON 0x29      //1001  

#define PAGE_DISP 8
#define WIDTH_DISP 128
#define HEIGHT_DISP 64

#define DISP_BUF_BYTES (PAGE_DISP*WIDTH_DISP)
static volatile u8 gDispBuf1[PAGE_DISP][WIDTH_DISP];//��ʾ����
static volatile u8 gDispBuf2[PAGE_DISP][WIDTH_DISP];//��ʾ����
static volatile u8 (*gpDisp)[WIDTH_DISP]=gDispBuf1;

//��d0-d7��������
static void WriteData(u8 Data)
{
	u8 i=0;

	for(i=0;i<8;i++)
	{
		if(ReadBit(Data,i)) 
		{
			PinSet(LCD_DAT_GPIOx,1<<i);
		}
		else
		{
			PinClr(LCD_DAT_GPIOx,1<<i);
		}
	}

	PinSet(LCD_RS_GPIOx,LCD_RS_PIN);
	PinSet(LCD_RD_GPIOx,LCD_RD_PIN);
	PinClr(LCD_WR_GPIOx,LCD_WR_PIN);
	
	PinClr(LCD_CS_GPIOx,LCD_CS_PIN);
	__NOP();__NOP();__NOP();
	PinSet(LCD_CS_GPIOx,LCD_CS_PIN);
}

//��d0-d7��������
static void WriteCmd(u8 Cmd)
{
	u8 i=0;

	for(i=0;i<8;i++)
	{
		if(ReadBit(Cmd,7-i)) 
		{
			PinSet(LCD_DAT_GPIOx,1<<i);
		}
		else
		{
			PinClr(LCD_DAT_GPIOx,1<<i);
		}
	}

	PinClr(LCD_RS_GPIOx,LCD_RS_PIN);
	PinSet(LCD_RD_GPIOx,LCD_RD_PIN);
	PinClr(LCD_WR_GPIOx,LCD_WR_PIN);
	
	PinClr(LCD_CS_GPIOx,LCD_CS_PIN);
	__NOP();__NOP();__NOP();
	PinSet(LCD_CS_GPIOx,LCD_CS_PIN);
}

static u8 ReadStatus(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u8 Reg=0,Status=0;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	PinClr(LCD_RS_GPIOx,LCD_RS_PIN);
	PinSet(LCD_WR_GPIOx,LCD_WR_PIN);

	PinClr(LCD_CS_GPIOx,LCD_CS_PIN);

	PinClr(LCD_RD_GPIOx,LCD_RD_PIN);
	__NOP();__NOP();__NOP();
	Reg=LCD_DAT_GPIOx->IDR;
	PinSet(LCD_RD_GPIOx,LCD_RD_PIN);
	
	PinSet(LCD_CS_GPIOx,LCD_CS_PIN);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	if(ReadBit(Reg,0)) SetBit(Status,7);
	if(ReadBit(Reg,1)) SetBit(Status,6);
	if(ReadBit(Reg,2)) SetBit(Status,5);
	if(ReadBit(Reg,3)) SetBit(Status,4);
	
	return Status;
}

//page 0-7
//col 0-127
static void __inline SetDispAddr(u8 Page,u8 Col)
{
     WriteCmd(0xB0|(7-Page)); 
     WriteCmd(0x10|(Col>>4)); 
     WriteCmd(0x00|(Col&0x0f)); 
}

//pData�Ĵ�С�������[H][(W-1)/8+1]
void LCD_DrawRegion(u16 X,u16 Y,u16 W,u16 H,const u8 *pData)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//���»���
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //��ֹԽ��
			{
				if(ReadBit(pData[j*RowBytes+(i>>3)],7-(i&0x07)))	
				{
					SetBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}

	//����ռ�˼���page��ռ�˼���col������һpage����һcol��ʼˢ����Ļ
	{
		u16 StartPage=Y>>3;
		u16 StartCol=X;
		u16 PageTotal=((Y+H)>>3) - (Y>>3) +1;//����ʽ���ܺϲ�ΪH/8 +1������ʧЧ

		for(j=0;j<PageTotal;j++)
		{
			if((StartPage+j) < PAGE_DISP) //��ֹԽ��
			{
				SetDispAddr(StartPage+j,StartCol);
				for(i=0;i<W;i++)
				{	
					if((StartCol+i) < WIDTH_DISP)  //��ֹԽ��
						WriteData(gpDisp[StartPage+j][StartCol+i]); 
				}
			}
		}
	}
}

//��������䵥һ����
void LCD_Fill(u16 X,u16 Y,u16 W,u16 H,u8 Data)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//���»���
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //��ֹԽ��
			{
				if(ReadBit(Data,7-(i&0x07)))	
				{
					SetBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}

	//����ռ�˼���page��ռ�˼���col������һpage����һcol��ʼˢ����Ļ
	{
		u16 StartPage=Y>>3;
		u16 StartCol=X;
		u16 PageTotal=((Y+H)>>3) - (Y>>3) +1;//����ʽ���ܺϲ�ΪH/8 +1������ʧЧ

		for(j=0;j<PageTotal;j++)
		{
			if((StartPage+j) < PAGE_DISP) //��ֹԽ��
			{
				SetDispAddr(StartPage+j,StartCol);
				for(i=0;i<W;i++)
				{	
					if((StartCol+i) < WIDTH_DISP)  //��ֹԽ��
						WriteData(gpDisp[StartPage+j][StartCol+i]); 
				}
			}
		}
	}
}

//pData�Ĵ�С�������[H][(W-1)/8+1]
void LCD_DrawRegion2(u16 X,u16 Y,u16 W,u16 H,const u8 *pData)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//���»���
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //��ֹԽ��
			{
				if(ReadBit(pData[j*RowBytes+(i>>3)],7-(i&0x07)))	
				{
					SetBit(gDispBuf2[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gDispBuf2[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}
}

//��������䵥һ����
void LCD_Fill2(u16 X,u16 Y,u16 W,u16 H,u8 Data)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//���»���
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //��ֹԽ��
			{
				if(ReadBit(Data,7-(i&0x07)))	
				{
					SetBit(gDispBuf2[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gDispBuf2[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}
}

static void LCD_Refresh(void)
{
	u32 i,j;
	
	for(j=0;j<PAGE_DISP;j++)
	{
		SetDispAddr(j,0);
		for(i=0;i<WIDTH_DISP;i++)
		{	
			WriteData(gpDisp[j][i]); 
		}
	}
}

//���������ڶ��������ݻ�������һ��
#define LCD_SLIDE_NUM 16
void LCD_Slide(bool ToRight)
{
	u32 i,j,n;
	
	if(ToRight)
	{
		for(j=0;j<WIDTH_DISP;j+=LCD_SLIDE_NUM)//����8�Σ�ÿ���ƶ�16��
		{
			for(n=0;n<PAGE_DISP;n++)
			{
				for(i=0;i<(128-LCD_SLIDE_NUM);i++) gDispBuf1[n][127-i]=gDispBuf1[n][127-LCD_SLIDE_NUM-i];
				for(i=0;i<LCD_SLIDE_NUM;i++) gDispBuf1[n][i]=gDispBuf2[n][(128-LCD_SLIDE_NUM)-j+i];
			}
			LCD_Refresh();
			DelayMs(120);
		}		
	}
	else
	{
		for(j=0;j<WIDTH_DISP;j+=LCD_SLIDE_NUM)//����8�Σ�ÿ���ƶ�16��
		{
			for(n=0;n<PAGE_DISP;n++)
			{
				for(i=0;i<(128-LCD_SLIDE_NUM);i++) gDispBuf1[n][i]=gDispBuf1[n][LCD_SLIDE_NUM+i];
				for(i=0;i<LCD_SLIDE_NUM;i++) gDispBuf1[n][i+(128-LCD_SLIDE_NUM)]=gDispBuf2[n][j+i];
			}
			LCD_Refresh();
			DelayMs(120);
		}	
	}
}

void LCD_Slide2(bool ToRight)
{
	u32 i,j,n;
	
	if(ToRight)
	{
		for(j=0;j<WIDTH_DISP;j+=16)//����8�Σ�ÿ���ƶ�16��
		{
			for(n=0;n<PAGE_DISP;n++)
			{
				for(i=0;i<112;i++) gDispBuf1[n][127-i]=gDispBuf1[n][127-16-i];
				for(i=0;i<16;i++) gDispBuf1[n][i]=gDispBuf2[n][112-j+i];
			}
			LCD_Refresh();
			DelayMs(100);
		}		
	}
	else
	{
		for(j=0;j<WIDTH_DISP;j+=16)//����8�Σ�ÿ���ƶ�16��
		{
			for(n=0;n<PAGE_DISP;n++)
			{
				for(i=0;i<112;i++) gDispBuf1[n][i]=gDispBuf1[n][16+i];
				for(i=0;i<16;i++) gDispBuf1[n][i+112]=gDispBuf2[n][j+i];
			}
			LCD_Refresh();
			DelayMs(100);
		}	
	}
}

void LCD_Test(void) 
{ 
	u16 x=0,y=0;
	bool xt=TRUE,yt=TRUE;
	
	while(1)
	{
		
		DelayMs(500);

		if(xt)
		{
			x++;
			if(x==WIDTH_DISP-97) xt=FALSE;
		}
		else
		{
			x--;
			if(x==0) xt=TRUE;
		}

		if(yt)
		{
			y++;
			if(y==HEIGHT_DISP-17) yt=FALSE;
		}
		else
		{
			y--;
			if(y==0) yt=TRUE;
		}
	}
} 

void LCD_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	PinSet(LCD_REST_GPIOx,LCD_REST_PIN);
	PinSet(LCD_CS_GPIOx,LCD_CS_PIN);

	LCD_Reset(); 
	Debug("Lcd Reset\n\r");

#if 0
	WriteCmd(0xA2);    //LCD Bias selection(1/65 Duty,1/9Bias)  
	WriteCmd(0xA0);    //ADC selection(SEG0->SEG128)  
	WriteCmd(0xC0);    //SHL selection(COM0->COM64)  
	WriteCmd(0x26);    //Regulator Resistor Selection  
	DelayMs(5); 
	WriteCmd(0x81);    //Electronic Volume  
	WriteCmd(0x20);    //Reference Register selection  Vo=(1+Rb/Ra)(1+a)*2.1=10  
	WriteCmd(LCMD_VC_ON);    //Power Control(Vc=1;Vr=0;Vf=0)  
	DelayMs(10); 
	WriteCmd(LCMD_VC_ON|LCMD_VR_ON); 
	DelayMs(10); 
	WriteCmd(LCMD_VC_ON|LCMD_VR_ON|LCMD_VF_ON); 
	DelayMs(10); 
	//WriteCmd(0xF8); 
	//WriteCmd(0x00); 
	DelayMs(5); 
	WriteCmd(0xAF);    //Display on  
	//WriteCmd(0xAE);    //Display off 
#else 
	WriteCmd(LCMD_DISP_ON);                             /*��ʾ��*/
	WriteCmd(LCMD_START_LINE_ADDR);                             /*��ʼ�е�ַ*/
	WriteCmd(0xa0);                             /*�Ƿ�ת��������ʾ*/
	WriteCmd(0xa6);                             /*�Ƿ�����ʾ*/
	WriteCmd(0xa4);                             /*��ȫ����ʾ*/
	WriteCmd(0xa2);                             /*1/9 bias*/
	WriteCmd(0xC0);                             /*com0-com64*/
	WriteCmd(0x2f);                             /*��Դȫ��*/
	WriteCmd(0x81);                             /*�������ȵ��ڼĴ���*/
	WriteCmd(0x1a);                             /*����ֵ*/
#endif

	MemSet(gpDisp,0,DISP_BUF_BYTES);
	LCD_Fill(0,0,128,64,0);

  	Debug("LCD Init Finish\n\r");
}

void LCD_Reset(void) 
{ 
	PinClr(LCD_REST_GPIOx,LCD_REST_PIN);
	DelayMs(50); 
	PinSet(LCD_REST_GPIOx,LCD_REST_PIN);
	DelayMs(50); 
} 





