/* Sample application to demonstrat FT800 primitives, widgets and customized screen shots */

#include "FT_Platform.h"


#define SAMAPP_DELAY_BTW_APIS (1000)
#define SAMAPP_ENABLE_DELAY() Ft_Gpu_Hal_Sleep(SAMAPP_DELAY_BTW_APIS)
#define SAMAPP_ENABLE_DELAY_VALUE(x) Ft_Gpu_Hal_Sleep(x)

#define WRITE2CMD(a) Ft_Gpu_Hal_WrCmdBuf(phost,a,sizeof(a))

/* Global variables for display resolution to support various display panels */
/* Default is WQVGA - 480x272 */
/* Global variables for display resolution to support various display panels */
/* Default is WQVGA - 480x272 */
ft_int16_t FT_DispWidth = 480;
ft_int16_t FT_DispHeight = 272;
ft_int16_t FT_DispHCycle =  548;
ft_int16_t FT_DispHOffset = 43;
ft_int16_t FT_DispHSync0 = 0;
ft_int16_t FT_DispHSync1 = 41;
ft_int16_t FT_DispVCycle = 292;
ft_int16_t FT_DispVOffset = 12;
ft_int16_t FT_DispVSync0 = 0;
ft_int16_t FT_DispVSync1 = 10;
ft_uint8_t FT_DispPCLK = 5;
ft_char8_t FT_DispSwizzle = 0;
ft_char8_t FT_DispPCLKPol = 1;
ft_char8_t FT_DispCSpread = 1;
ft_char8_t FT_DispDither = 1;

/* Boot up for FT800 followed by graphics primitive sample cases */
/* Initial boot up DL - make the back ground green color */

const ft_uint8_t FT_DLCODE_BOOTUP[12] =
{
  255,255,255,2,//GPU instruction CLEAR_COLOR_RGB
  7,0,0,38, //GPU instruction CLEAR
  0,0,0,0,  //GPU instruction DISPLAY
};


/* Global used for buffer optimization */
Ft_Gpu_Hal_Context_t host,*phost;

ft_uint32_t Ft_CmdBuffer_Index;
ft_uint32_t Ft_DlBuffer_Index;

#ifdef BUFFER_OPTIMIZATION
ft_uint8_t  Ft_DlBuffer[FT_DL_SIZE];
ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif

ft_void_t Ft_App_WrCoCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef  BUFFER_OPTIMIZATION
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_CmdBuffer[Ft_CmdBuffer_Index];
   *pBuffcmd = cmd;
#endif
#ifdef ARDUINO_PLATFORM
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
#endif
#ifdef FT900_PLATFORM
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
#endif
   /* Increment the command index */
   Ft_CmdBuffer_Index += FT_CMD_SIZE;  
}

ft_void_t Ft_App_WrDlCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef BUFFER_OPTIMIZATION  
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_DlBuffer[Ft_DlBuffer_Index];
   *pBuffcmd = cmd;
#endif

#ifdef ARDUINO_PLATFORM
   Ft_Gpu_Hal_Wr32(phost,(RAM_DL+Ft_DlBuffer_Index),cmd);
#endif
#ifdef FT900_PLATFORM
   Ft_Gpu_Hal_Wr32(phost,(RAM_DL+Ft_DlBuffer_Index),cmd);
#endif
   /* Increment the command index */
   Ft_DlBuffer_Index += FT_CMD_SIZE;  
}

ft_void_t Ft_App_WrCoStr_Buffer(Ft_Gpu_Hal_Context_t *phost,const ft_char8_t *s)
{
#ifdef  BUFFER_OPTIMIZATION  
  ft_uint16_t length = 0;
  length = strlen(s) + 1;//last for the null termination
  
  strcpy(&Ft_CmdBuffer[Ft_CmdBuffer_Index],s);  

  /* increment the length and align it by 4 bytes */
  Ft_CmdBuffer_Index += ((length + 3) & ~3);  
#endif  
}

ft_void_t Ft_App_Flush_DL_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION    
   if (Ft_DlBuffer_Index> 0)
     Ft_Gpu_Hal_WrMem(phost,RAM_DL,Ft_DlBuffer,Ft_DlBuffer_Index);
#endif     
   Ft_DlBuffer_Index = 0;
   
}

ft_void_t Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION    
   if (Ft_CmdBuffer_Index > 0)
     Ft_Gpu_Hal_WrCmdBuf(phost,Ft_CmdBuffer,Ft_CmdBuffer_Index);
#endif     
   Ft_CmdBuffer_Index = 0;
}


/* API to give fadeout effect by changing the display PWM from 100 till 0 */
ft_void_t SAMAPP_fadeout()
{
   ft_int32_t i;
	
	for (i = 100; i >= 0; i -= 3) 
	{
		Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);

		Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
	}
}

/* API to perform display fadein effect by changing the display PWM from 0 till 100 and finally 128 */
ft_void_t SAMAPP_fadein()
{
	ft_int32_t i;
	
	for (i = 0; i <=100 ; i += 3) 
	{
		Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
		Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
	}
	/* Finally make the PWM 100% */
	i = 128;
	Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
}


/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t SAMAPP_GPU_DLSwap(ft_uint8_t DL_Swap_Type)
{
	ft_uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

	if(DL_Swap_Type == DLSWAP_LINE)
	{
		Swap_Type = DLSWAP_LINE;
	}

	/* Perform a new DL swap */
	Ft_Gpu_Hal_Wr8(phost,REG_DLSWAP,Swap_Type);

	/* Wait till the swap is done */
	while(Swap_Done)
	{
		Swap_Done = Ft_Gpu_Hal_Rd8(phost,REG_DLSWAP);

		if(DLSWAP_DONE != Swap_Done)
		{
			Ft_Gpu_Hal_Sleep(10);//wait for 10ms
		}
	}	
}


ft_void_t Ft_BootupConfig()
{
	Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);

		/* Access address 0 to wake up the FT800 */
		Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);
		Ft_Gpu_Hal_Sleep(20);

		/* Set the clk to external clock */
    #if (!defined(ME800A_HV35R) && !defined(ME810A_HV35R) && !defined(ME812AU_WH50R) && !defined(ME813AU_WH50C) && !defined(ME810AU_WH70R) && !defined(ME811AU_WH70C))
		Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
		Ft_Gpu_Hal_Sleep(10);
    #endif

		{
			ft_uint8_t chipid;
			//Read Register ID to check if FT800 is ready.
			chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
			while(chipid != 0x7C)
			{
				chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
				ft_delay(100);
			}
	#if defined(MSVC_PLATFORM) || defined (FT900_PLATFORM)
			printf("VC1 register ID after wake up %x\n",chipid);
	#endif
	}
	/* Configuration of LCD display */
#ifdef DISPLAY_RESOLUTION_QVGA
	/* Values specific to QVGA LCD display */
	FT_DispWidth = 320;
	FT_DispHeight = 240;
	FT_DispHCycle =  408;
	FT_DispHOffset = 70;
	FT_DispHSync0 = 0;
	FT_DispHSync1 = 10;
	FT_DispVCycle = 263;
	FT_DispVOffset = 13;
	FT_DispVSync0 = 0;
	FT_DispVSync1 = 2;
	FT_DispPCLK = 8;
	FT_DispSwizzle = 2;
	FT_DispPCLKPol = 0;
	FT_DispCSpread = 1;
	FT_DispDither = 1;

#endif
#ifdef DISPLAY_RESOLUTION_WVGA
	/* Values specific to QVGA LCD display */
	FT_DispWidth = 800;
	FT_DispHeight = 480;
	FT_DispHCycle =  928;
	FT_DispHOffset = 88;
	FT_DispHSync0 = 0;
	FT_DispHSync1 = 48;
	FT_DispVCycle = 525;
	FT_DispVOffset = 32;
	FT_DispVSync0 = 0;
	FT_DispVSync1 = 3;
	FT_DispPCLK = 2;
	FT_DispSwizzle = 0;
	FT_DispPCLKPol = 1;
	FT_DispCSpread = 0;
	FT_DispDither = 1;
#endif
#ifdef DISPLAY_RESOLUTION_HVGA_PORTRAIT
	/* Values specific to HVGA LCD display */

	FT_DispWidth = 320;
	FT_DispHeight = 480;
	FT_DispHCycle =  400;
	FT_DispHOffset = 40;
	FT_DispHSync0 = 0;
	FT_DispHSync1 = 10;
	FT_DispVCycle = 500;
	FT_DispVOffset = 10;
	FT_DispVSync0 = 0;
	FT_DispVSync1 = 5;
	FT_DispPCLK = 4;
	FT_DispSwizzle = 2;
	FT_DispPCLKPol = 1;
	FT_DispCSpread = 1;
	FT_DispDither = 1;

#ifdef ME810A_HV35R
	FT_DispPCLK = 5;
#endif

#endif

#if defined(ME800A_HV35R)
	/* After recognizing the type of chip, perform the trimming if necessary */
    Ft_Gpu_ClockTrimming(phost,LOW_FREQ_BOUND);
#endif

	Ft_Gpu_Hal_Wr16(phost, REG_HCYCLE, FT_DispHCycle);
	Ft_Gpu_Hal_Wr16(phost, REG_HOFFSET, FT_DispHOffset);
	Ft_Gpu_Hal_Wr16(phost, REG_HSYNC0, FT_DispHSync0);
	Ft_Gpu_Hal_Wr16(phost, REG_HSYNC1, FT_DispHSync1);
	Ft_Gpu_Hal_Wr16(phost, REG_VCYCLE, FT_DispVCycle);
	Ft_Gpu_Hal_Wr16(phost, REG_VOFFSET, FT_DispVOffset);
	Ft_Gpu_Hal_Wr16(phost, REG_VSYNC0, FT_DispVSync0);
	Ft_Gpu_Hal_Wr16(phost, REG_VSYNC1, FT_DispVSync1);
	Ft_Gpu_Hal_Wr8(phost, REG_SWIZZLE, FT_DispSwizzle);
	Ft_Gpu_Hal_Wr8(phost, REG_PCLK_POL, FT_DispPCLKPol);
	Ft_Gpu_Hal_Wr16(phost, REG_HSIZE, FT_DispWidth);
	Ft_Gpu_Hal_Wr16(phost, REG_VSIZE, FT_DispHeight);
	Ft_Gpu_Hal_Wr16(phost, REG_CSPREAD, FT_DispCSpread);
	Ft_Gpu_Hal_Wr16(phost, REG_DITHER, FT_DispDither);

#if (defined(FT_800_ENABLE) || defined(FT_810_ENABLE) ||defined(FT_812_ENABLE))
    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    Ft_Gpu_Hal_Wr16(phost, REG_TOUCH_RZTHRESH,RESISTANCE_THRESHOLD);
#endif
    #if defined(FT_81X_ENABLE)
        Ft_Gpu_Hal_Wr16(phost, REG_GPIOX_DIR, 0xffff);
        Ft_Gpu_Hal_Wr16(phost, REG_GPIOX, 0xffff);
    #else
        Ft_Gpu_Hal_Wr8(phost, REG_GPIO_DIR,0xff);
        Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);
    #endif


    /*It is optional to clear the screen here*/
    Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);


    Ft_Gpu_Hal_Wr8(phost, REG_PCLK,FT_DispPCLK);//after this display is visible on the LCD


#ifdef ENABLE_ILI9488_HVGA_PORTRAIT
	/* to cross check reset pin */
	Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);
	ft_delay(120);
	Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0x7f);
	ft_delay(120);
	Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);

	ILI9488_Bootup();
#ifdef FT900_PLATFORM
	printf("after ILI9488 bootup \n");
	//spi
	// Initialize SPIM HW
	sys_enable(sys_device_spi_master);
	gpio_function(27, pad_spim_sck); /* GPIO27 to SPIM_CLK */
	gpio_function(28, pad_spim_ss0); /* GPIO28 as CS */
	gpio_function(29, pad_spim_mosi); /* GPIO29 to SPIM_MOSI */
	gpio_function(30, pad_spim_miso); /* GPIO30 to SPIM_MISO */

	gpio_write(28, 1);
	spi_init(SPIM, spi_dir_master, spi_mode_0, 4);
#endif

#endif



	/* make the spi to quad mode - addition 2 bytes for silicon */
#ifdef FT_81X_ENABLE
	/* api to set quad and numbe of dummy bytes */
#ifdef ENABLE_SPI_QUAD
	Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_QUAD_CHANNEL,FT_GPU_SPI_TWODUMMY);
#elif ENABLE_SPI_DUAL
	Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_DUAL_CHANNEL,FT_GPU_SPI_TWODUMMY);
#else
	Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_SINGLE_CHANNEL,FT_GPU_SPI_ONEDUMMY);
#endif

#endif

#ifdef FT900_PLATFORM
    /* Change clock frequency to 25mhz */
	spi_init(SPIM, spi_dir_master, spi_mode_0, 4);

#if (defined(ENABLE_SPI_QUAD))
    /* Initialize IO2 and IO3 pad/pin for dual and quad settings */
    gpio_function(31, pad_spim_io2);
    gpio_function(32, pad_spim_io3);
    gpio_write(31, 1);
    gpio_write(32, 1);
#endif
	/* Enable FIFO of QSPI */
	spi_option(SPIM,spi_option_fifo_size,64);
	spi_option(SPIM,spi_option_fifo,1);
	spi_option(SPIM,spi_option_fifo_receive_trigger,1);
#endif

#ifdef ENABLE_SPI_QUAD
#ifdef FT900_PLATFORM
	spi_option(SPIM,spi_option_bus_width,4);
#endif
#elif ENABLE_SPI_DUAL	
#ifdef FT900_PLATFORM
	spi_option(SPIM,spi_option_bus_width,2);
#endif
#else
#ifdef FT900_PLATFORM
	spi_option(SPIM,spi_option_bus_width,1);
#endif

	
#endif


phost->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(phost,REG_CMD_WRITE);
}




/* Optimized implementation of sin and cos table - precision is 16 bit */
FT_PROGMEM ft_prog_uint16_t sintab[] = {
  0, 402, 804, 1206, 1607, 2009, 2410, 2811, 3211, 3611, 4011, 4409, 4807, 5205, 5601, 5997, 6392, 
  6786, 7179, 7571, 7961, 8351, 8739, 9126, 9511, 9895, 10278, 10659, 11038, 11416, 11792, 12166, 12539,
  12909, 13278, 13645, 14009, 14372, 14732, 15090, 15446, 15799, 16150, 16499, 16845, 17189, 17530, 17868,
  18204, 18537, 18867, 19194, 19519, 19840, 20159, 20474, 20787, 21096, 21402, 21705, 22004, 22301, 22594, 
  22883, 23169, 23452, 23731, 24006, 24278, 24546, 24811, 25072, 25329, 25582, 25831, 26077, 26318, 26556, 26789, 
  27019, 27244, 27466, 27683, 27896, 28105, 28309, 28510, 28706, 28897, 29085, 29268, 29446, 29621, 29790, 29955, 
  30116, 30272, 30424, 30571, 30713, 30851, 30984, 31113, 31236, 31356, 31470, 31580, 31684, 31785, 31880, 31970, 
  32056, 32137, 32213, 32284, 32350, 32412, 32468, 32520, 32567, 32609, 32646, 32678, 32705, 32727, 32744, 32757, 
  32764, 32767, 32764};
  
ft_int16_t qsin(ft_uint16_t a)
{
  ft_uint8_t f;
  ft_int16_t s0,s1;

  if (a & 32768)
    return -qsin(a & 32767);
  if (a & 16384)
    a = 32768 - a;
  f = a & 127;
  s0 = ft_pgm_read_word(sintab + (a >> 7));
  s1 = ft_pgm_read_word(sintab + (a >> 7) + 1);
  return (s0 + ((ft_int32_t)f * (s1 - s0) >> 7));
}

/* cos funtion */
ft_int16_t qcos(ft_uint16_t a)
{
  return (qsin(a + 16384));
}




/*****************************************************************************/
/* Example code to display few points at various offsets with various colors */




static ft_int32_t ox;
static ft_uint8_t noofch=0;

static ft_uint8_t home_star_icon[] = {0x78,0x9C,0xE5,0x94,0xBF,0x4E,0xC2,0x40,0x1C,0xC7,0x7F,0x2D,0x04,0x8B,0x20,0x45,0x76,0x14,0x67,0xA3,0xF1,0x0D,0x64,0x75,0xD2,0xD5,0x09,0x27,0x17,0x13,0xE1,0x0D,0xE4,0x0D,0x78,0x04,0x98,0x5D,0x30,0x26,0x0E,0x4A,0xA2,0x3E,0x82,0x0E,0x8E,0x82,0xC1,0x38,0x62,0x51,0x0C,0x0A,0x42,0x7F,0xDE,0xB5,0x77,0xB4,0x77,0x17,0x28,0x21,0x26,0x46,0xFD,0x26,0xCD,0xE5,0xD3,0x7C,0xFB,0xBB,0xFB,0xFD,0xB9,0x02,0xCC,0xA4,0xE8,0x99,0x80,0x61,0xC4,0x8A,0x9F,0xCB,0x6F,0x31,0x3B,0xE3,0x61,0x7A,0x98,0x84,0x7C,0x37,0xF6,0xFC,0xC8,0xDD,0x45,0x00,0xDD,0xBA,0xC4,0x77,0xE6,0xEE,0x40,0xEC,0x0E,0xE6,0x91,0xF1,0xD2,0x00,0x42,0x34,0x5E,0xCE,0xE5,0x08,0x16,0xA0,0x84,0x68,0x67,0xB4,0x86,0xC3,0xD5,0x26,0x2C,0x20,0x51,0x17,0xA2,0xB8,0x03,0xB0,0xFE,0x49,0xDD,0x54,0x15,0xD8,0xEE,0x73,0x37,0x95,0x9D,0xD4,0x1A,0xB7,0xA5,0x26,0xC4,0x91,0xA9,0x0B,0x06,0xEE,0x72,0xB7,0xFB,0xC5,0x16,0x80,0xE9,0xF1,0x07,0x8D,0x3F,0x15,0x5F,0x1C,0x0B,0xFC,0x0A,0x90,0xF0,0xF3,0x09,0xA9,0x90,0xC4,0xC6,0x37,0xB0,0x93,0xBF,0xE1,0x71,0xDB,0xA9,0xD7,0x41,0xAD,0x46,0xEA,0x19,0xA9,0xD5,0xCE,0x93,0xB3,0x35,0x73,0x0A,0x69,0x59,0x91,0xC3,0x0F,0x22,0x1B,0x1D,0x91,0x13,0x3D,0x91,0x73,0x43,0xF1,0x6C,0x55,0xDA,0x3A,0x4F,0xBA,0x25,0xCE,0x4F,0x04,0xF1,0xC5,0xCF,0x71,0xDA,0x3C,0xD7,0xB9,0xB2,0x48,0xB4,0x89,0x38,0x20,0x4B,0x2A,0x95,0x0C,0xD5,0xEF,0x5B,0xAD,0x96,0x45,0x8A,0x41,0x96,0x7A,0x1F,0x60,0x0D,0x7D,0x22,0x75,0x82,0x2B,0x0F,0xFB,0xCE,0x51,0x3D,0x2E,0x3A,0x21,0xF3,0x1C,0xD9,0x38,0x86,0x2C,0xC6,0x05,0xB6,0x7B,0x9A,0x8F,0x0F,0x97,0x1B,0x72,0x6F,0x1C,0xEB,0xAE,0xFF,0xDA,0x97,0x0D,0xBA,0x43,0x32,0xCA,0x66,0x34,0x3D,0x54,0xCB,0x24,0x9B,0x43,0xF2,0x70,0x3E,0x42,0xBB,0xA0,0x95,0x11,0x37,0x46,0xE1,0x4F,0x49,0xC5,0x1B,0xFC,0x3C,0x3A,0x3E,0xD1,0x65,0x0E,0x6F,0x58,0xF8,0x9E,0x5B,0xDB,0x55,0xB6,0x41,0x34,0xCB,0xBE,0xDB,0x87,0x5F,0xA9,0xD1,0x85,0x6B,0xB3,0x17,0x9C,0x61,0x0C,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xED,0xC9,0xFC,0xDF,0x14,0x54,0x8F,0x80,0x7A,0x06,0xF5,0x23,0xA0,0x9F,0x41,0xF3,0x10,0x30,0x4F,0x41,0xF3,0x18,0x30,0xCF,0xCA,0xFC,0xFF,0x35,0xC9,0x79,0xC9,0x89,0xFA,0x33,0xD7,0x1D,0xF6,0x5E,0x84,0x5C,0x56,0x6E,0xA7,0xDA,0x1E,0xF9,0xFA,0xAB,0xF5,0x97,0xFF,0x2F,0xED,0x89,0x7E,0x29,0x9E,0xB4,0x9F,0x74,0x1E,0x69,0xDA,0xA4,0x9F,0x81,0x94,0xEF,0x4F,0xF6,0xF9,0x0B,0xF4,0x65,0x51,0x08};






static void polarxy(ft_int32_t r, ft_uint16_t th, ft_int32_t *x, ft_int32_t *y)
{
  *x = (16 * (FT_DispWidth/(2*noofch)) + (((long)r * qsin(th)) >> 11) + 16 * ox);
  *y = (16 * 300 - (((long)r * qcos(th)) >> 11));
}


void vertex(ft_int32_t x, ft_int32_t y)
{
  Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x,y));
}


static void polar(ft_int32_t r, ft_uint16_t th)
{
  ft_int32_t x, y;
  polarxy(r, th, &x, &y);
  vertex(x, y);
}

ft_uint16_t da(ft_int32_t i)
{
  return (i - 45) * 32768L / 360;
}

static void cs(ft_uint8_t i)
{
  switch (i)
  {
    case  0:  Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(200,255,200)); break;
    case 60: Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,0)); break;
    case 80: Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,0,0)); break;
  }
}

 
char *info[] = { "FT800 Signals Application",

				"APP to demonstrate drawing Signals,",
				"using Strips, Points",
				"& Blend function."
               };
ft_void_t home_setup()
{
  Ft_Gpu_Hal_WrCmd32(phost,CMD_INFLATE);
  Ft_Gpu_Hal_WrCmd32(phost,250*1024L);
  Ft_Gpu_Hal_WrCmdBuf(phost,home_star_icon,sizeof(home_star_icon));
  
  Ft_Gpu_CoCmd_Dlstart(phost);        // start
  Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
  Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255, 255, 255));
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_HANDLE(13));    // handle for background stars
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_SOURCE(250*1024L));      // Starting address in gram
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT(L4, 16, 32));  // format 
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE(NEAREST, REPEAT, REPEAT, 512, 512  ));
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_HANDLE(14));    // handle for background stars
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_SOURCE(250*1024L));      // Starting address in gram
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT(L4, 16, 32));  // format 
  Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE(NEAREST, BORDER, BORDER, 32, 32  ));
  Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
  Ft_Gpu_CoCmd_Swap(phost);
  Ft_App_Flush_Co_Buffer(phost);
  Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
}

ft_void_t Play_Sound(ft_uint16_t sound,ft_uint8_t volume)
{
  Ft_Gpu_Hal_Wr8(phost,REG_VOL_SOUND,volume);
  Ft_Gpu_Hal_Wr16(phost,REG_SOUND,sound);
  Ft_Gpu_Hal_Wr8(phost,REG_PLAY,1);
}	
static ft_uint8_t sk=0;
ft_uint8_t Read_keys()
{
  static ft_uint8_t Read_tag=0,temp_tag=0,ret_tag=0;	
  Read_tag = Ft_Gpu_Hal_Rd8(phost,REG_TOUCH_TAG);
  ret_tag = NULL;
  if(Read_tag!=NULL && temp_tag!=Read_tag)
  {
     temp_tag = Read_tag;											// Load the Read tag to temp variable	
     Play_Sound(0x51,100);
	 sk = Read_tag;											// Load the Read tag to temp variable	
  } 
  if(Read_tag==NULL)
  {
     ret_tag =  temp_tag;
     temp_tag = 0;
	 sk = 0;
  }
  return ret_tag;
}


ft_void_t Info()
{
  ft_uint16_t dloffset = 0,z;
  Ft_CmdBuffer_Index = 0;
  

  
  Ft_Gpu_CoCmd_Dlstart(phost); 
  Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
  Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,255));
  Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,FT_DispHeight/2,26,OPT_CENTERX|OPT_CENTERY,"Please tap on a dot");
  Ft_Gpu_CoCmd_Calibrate(phost,0);
  Ft_App_Flush_Co_Buffer(phost);
  Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
  
  Ft_Gpu_CoCmd_Logo(phost);
  Ft_App_Flush_Co_Buffer(phost);
  Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
  while(0!=Ft_Gpu_Hal_Rd16(phost,REG_CMD_READ)); 
  dloffset = Ft_Gpu_Hal_Rd16(phost,REG_CMD_DL);
  dloffset -= 4;

#ifdef FT_81X_ENABLE
  dloffset -= 2*4;//remove two more instructions in case of 81x
#endif


  Ft_Gpu_Hal_WrCmd32(phost,CMD_MEMCPY);
  Ft_Gpu_Hal_WrCmd32(phost,100000L);
  Ft_Gpu_Hal_WrCmd32(phost,RAM_DL);
  Ft_Gpu_Hal_WrCmd32(phost,dloffset);
  do
  {
    Ft_Gpu_CoCmd_Dlstart(phost);   
    Ft_Gpu_CoCmd_Append(phost,100000L,dloffset);
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_A(256));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_A(256));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_B(0));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_C(0));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_D(0));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_E(256));
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_F(0));  
    Ft_App_WrCoCmd_Buffer(phost,SAVE_CONTEXT());	
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(219,180,150));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_A(220));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(EDGE_STRIP_A));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(0,FT_DispHeight*16));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(FT_DispWidth*16,FT_DispHeight*16));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_A(255));
    Ft_App_WrCoCmd_Buffer(phost,RESTORE_CONTEXT());	
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0,0,0));
   // INFORMATION 
    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,20,28,OPT_CENTERX|OPT_CENTERY,info[0]);
    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,60,26,OPT_CENTERX|OPT_CENTERY,info[1]);
    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,90,26,OPT_CENTERX|OPT_CENTERY,info[2]);  
    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,120,26,OPT_CENTERX|OPT_CENTERY,info[3]);  
    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,FT_DispHeight-30,26,OPT_CENTERX|OPT_CENTERY,"Click to play");
    if(sk!='P')
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,255));
    else
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(100,100,100));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(FTPOINTS));   
    Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(20*16));
    Ft_App_WrCoCmd_Buffer(phost,TAG('P'));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((FT_DispWidth/2)*16,(FT_DispHeight-60)*16));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(180,35,35));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(BITMAPS));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2II((FT_DispWidth/2)-14,(FT_DispHeight-75),14,4));
    Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
    Ft_Gpu_CoCmd_Swap(phost);
    Ft_App_Flush_Co_Buffer(phost);
    Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
  }while(Read_keys()!='P');
    Play_Sound(0x50,255);
}

#ifdef FT900_PLATFORM
ft_void_t FT900_Config()
{
	sys_enable(sys_device_uart0);
		    gpio_function(48, pad_uart0_txd); /* UART0 TXD */
		    gpio_function(49, pad_uart0_rxd); /* UART0 RXD */
		    uart_open(UART0,                    /* Device */
		              1,                        /* Prescaler = 1 */
		              UART_DIVIDER_115200_BAUD,  /* Divider = 1302 */
		              uart_data_bits_8,         /* No. Data Bits */
		              uart_parity_none,         /* Parity */
		              uart_stop_bits_1);        /* No. Stop Bits */

		    /* Print out a welcome message... */
		    uart_puts(UART0,

		        "(C) Copyright 2014-2015, Future Technology Devices International Ltd. \r\n"
		        "--------------------------------------------------------------------- \r\n"
		        "Welcome to Signal Example ... \r\n"
		        "\r\n"
		        "--------------------------------------------------------------------- \r\n"
		        );

		    //init_printf(UART0,myputc);

	#ifdef ENABLE_ILI9488_HVGA_PORTRAIT
		/* asign all the respective pins to gpio and set them to default values */
		gpio_function(34, pad_gpio34);
		gpio_dir(34, pad_dir_output);
	    gpio_write(34,1);

		gpio_function(27, pad_gpio27);
		gpio_dir(27, pad_dir_output);
	    gpio_write(27,1);

		gpio_function(29, pad_gpio29);
		gpio_dir(29, pad_dir_output);
	    gpio_write(29,1);

	    gpio_function(33, pad_gpio33);
	    gpio_dir(33, pad_dir_output);
	    gpio_write(33,1);


	    gpio_function(30, pad_gpio30);
	    gpio_dir(30, pad_dir_output);
	    gpio_write(30,1);

		gpio_function(28, pad_gpio28);
		gpio_dir(28, pad_dir_output);
	    gpio_write(28,1);


	  	gpio_function(43, pad_gpio43);
	  	gpio_dir(43, pad_dir_output);
	    gpio_write(43,1);
		gpio_write(34,1);
		gpio_write(28,1);
		gpio_write(43,1);
		gpio_write(33,1);
		gpio_write(33,1);

	#endif
		/* useful for timer */
		ft_millis_init();
		interrupt_enable_globally();
		//printf("ft900 config done \n");
	}
#endif

static ft_uint8_t rate = 2;
static ft_int16_t x,y,tx;
static ft_uint8_t beats[10];
static ft_char8_t beats_Incr[10] = {-10,10,5,-5,-20,20,12,-12,-5,5};
static ft_uint16_t add2write = 0;

static byte istouch()
{
  return !(Ft_Gpu_Hal_Rd16(phost,REG_TOUCH_RAW_XY) & 0x8000);
}

void Sine_wave(ft_uint8_t amp)
{
  static ft_uint8_t played = 0,change=0;
  x+=rate;
  if(x>FT_DispWidth) x  = 0;
  y = (FT_DispHeight/2) + ((ft_int32_t)amp*qsin(-65536*x/(25*rate))/65536);
  if(played==0 &&  change < y){ 
  played = 1;
  Play_Sound((108<<8 | 0x10),100); }
  if(change > y)
  played = 0;
  change = y;
  Ft_Gpu_Hal_Wr16(phost,RAM_G+(x/rate)*4,VERTEX2F(x*16,y*16));	
}



void Sawtooth_wave(ft_uint8_t amp)
{
  static ft_uint16_t temp=0;
  static ft_uint8_t pk = 0;
  x+=rate;
  if(x>FT_DispWidth){ x  = 0;}
  temp+=2; if(temp>65535L) temp = 0;
  y = (temp % amp);
  pk = y/(amp-2);
  if(pk) Play_Sound((108<<8 | 0x10),100);				 
  y = (FT_DispHeight/2)-y;
  Ft_Gpu_Hal_Wr16(phost,RAM_G+(x/rate)*4,VERTEX2F(x*16,y*16));	
}

void Triangle_wave(ft_uint8_t amp)
{
  static ft_uint16_t temp=0;
 static ft_uint8_t pk = 0,dc=0,p=0;
  x+=rate;
  if(x>FT_DispWidth){ x  = 0;}
  temp+=2; if(temp>65535L) temp = 0;
  y = (temp % amp);
  pk = (y/(amp-2))%2;
  dc = (temp / amp)%2;  
  if(pk) { if(p==0){ p=1; Play_Sound((108<<8 | 0x10),100); } else  p=0;}
  if(dc) y = (FT_DispHeight/2) -(amp-y);  else
  y = (FT_DispHeight/2) - y;
  Ft_Gpu_Hal_Wr16(phost,RAM_G+(x/rate)*4,VERTEX2F(x*16,y*16));	
}
static ft_uint16_t temp_x,temp_p,temp_y,en;

void Heartbeat()
{	 ft_uint16_t tval;
	  y = FT_DispHeight/2;
	  for(tval=0;tval<10;tval++)
	  {
	    y = y+(beats_Incr[tval]*5);
	    beats[tval] = y;
	    //printf(" \n tval=%d   beats=   %d",tval,beats[tval]);
	   }
	   x+=rate; if(x>FT_DispWidth){ x  = 0;temp_p = 0;temp_y=0;
					y=FT_DispHeight/2;
					en=0;temp_x=0;}
	   tx = 5*rate;
	   tx = ((temp_p+1)*tx) + temp_p*temp_x;
	   if(tx<=x){ if(0==en)	  en = 1;}
	  if(en==1){
	  if(y!=beats[temp_y])
	  {
		  //printf("\n y=%d beats[temp_y]=%d",y,beats[temp_y]);
		  y += beats_Incr[temp_y] * 5;
		  temp_y++;
	    if(y==(FT_DispHeight/2)+beats_Incr[4] * 5)
			Play_Sound((108<<8 | 0x10),100);
	  }
	  else
	  {
	    temp_y++;
	    if(temp_y>9) {
	    temp_y = 0;   temp_p++;
	    en = 0;  temp_x = x - tx;

	    }
	  }}
	  Ft_Gpu_Hal_Wr32(phost,RAM_G+(x/rate)*4,VERTEX2F(x*16,y*16));
	  //printf("\n x=%d, y=%d",x,y);

}

ft_void_t Ft_Gpu_Radiobutton(ft_int16_t x,ft_int16_t y,ft_uint32_t bgcolor,ft_uint32_t fgcolor,ft_uint8_t psize,ft_uint8_t tag,ft_uint8_t option)
{
  ft_uint8_t check_size = psize/2;
  Ft_App_WrCoCmd_Buffer(phost,SAVE_CONTEXT());
  Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB((bgcolor >> 16 & 0xff),(bgcolor >> 8 & 0xff),(bgcolor & 0xff)));
  Ft_App_WrCoCmd_Buffer(phost,TAG(tag));
  Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(psize*16));
  Ft_App_WrCoCmd_Buffer(phost,BEGIN(FTPOINTS));
  Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,y*16));
  if(tag == option)
  {
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB((fgcolor >> 16 & 0xff),(fgcolor >> 8 & 0xff),(fgcolor & 0xff)));
    Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(check_size*16));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,y*16));
  }
  Ft_App_WrCoCmd_Buffer(phost,RESTORE_CONTEXT());  
}


ft_void_t Waves()
{
  ft_int16_t i,xx=0;
  ft_uint16_t tval,th_to,amp;
  ft_uint8_t  temp[480],hide_x=0,fg=1,tag,opt=3;
// ========  Bg =========================================================
  for(tval=0;tval<=FT_DispHeight/2;tval++)	
  {
    temp[FT_DispHeight-tval] = temp[tval] = (tval*0.90);		
  }
  Ft_Gpu_Hal_WrMem(phost,FT_DispWidth*4L,temp,sizeof(temp));
  
  
  y = FT_DispHeight/2;
  for(tval=0;tval<10;tval++)
  {
    y = y+(beats_Incr[tval]*5);
    beats[tval] = y;
  }
  
  for(tval=0;tval<FT_DispWidth;tval+=rate)
  {
    Ft_Gpu_Hal_Wr32(phost,RAM_G+((tval/rate)*4),VERTEX2F(tval*16,y*16));			
  }
  add2write = 0;
  
  while(1)
  {
    

// ========  Menu  =========================================================
    if(istouch())   fg = 1;
    if(fg){ th_to=0; if(hide_x>0)hide_x=0; else
      fg = 0;  }
    else 
    { th_to++;
      if(th_to > 250) {
       if( hide_x < 85) hide_x++;   else
       th_to = 0;     }
    }
//==========Option =========================================================    
    tag = Read_keys();
    if(tag!=0)
    {
      x = 0;   temp_p = 0;en = 0; temp_x = 0; temp_y = 0; //reset
      if(tag>2)  opt = tag;
      if(tag==1)if(rate>1)rate--;	
      if(tag==2)if(rate<6)rate++;			
      y = (FT_DispHeight/2);
      for(tval=0;tval<FT_DispWidth;tval+=rate)
      {
	Ft_Gpu_Hal_Wr32(phost,RAM_G+((tval/rate)*4),VERTEX2F(tval*16,y*16));			
      }
      add2write = 0;
    }    
//========= Signals ========================================================
  amp = 100;
  switch(opt)
  {
    case 5:

      Triangle_wave(amp);
    break;
    
    case 4:
        Sawtooth_wave(amp);
    break;
    
    case 3:
          Sine_wave(amp);
    break;
    
    case 6:
      amp = 50;
      Heartbeat();					
    break;
 }
//=========Display list start===================================================    
    Ft_Gpu_CoCmd_Dlstart(phost); 
    Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0x12,0x4A,0x26)); 
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_SOURCE(FT_DispWidth*4L));	
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT(L8,1,FT_DispHeight));	
#ifdef DISPLAY_RESOLUTION_WVGA
   Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT_H(0,FT_DispHeight>>9));
#endif
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE(NEAREST, REPEAT, BORDER, FT_DispWidth, FT_DispHeight));
#ifdef DISPLAY_RESOLUTION_WVGA
    Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE_H( FT_DispWidth>>9, FT_DispHeight>>9));
#endif
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(BITMAPS));
    Ft_App_WrCoCmd_Buffer(phost,TAG(0));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(0,0));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0x1B,0xE0,0x67));	

    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0x1B,0xE0,0x67));
    Ft_App_WrCoCmd_Buffer(phost,LINE_WIDTH(2*16));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(LINE_STRIP));
    Ft_Gpu_CoCmd_Append(phost,RAM_G,(x/rate)*4);
    Ft_App_WrCoCmd_Buffer(phost,END());
    
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(LINE_STRIP));  
    if((x/rate)<(FT_DispWidth/rate)-(50/rate))            // else it screw up
    Ft_Gpu_CoCmd_Append(phost,RAM_G+(x/rate)*4+((50/rate)*4),((FT_DispWidth/rate)*4)-((x/rate)*4)-((50/rate)*4));
    
    Ft_App_WrCoCmd_Buffer(phost,END());	

    Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(6*16));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(FTPOINTS));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,y*16));	
    Ft_App_WrCoCmd_Buffer(phost,END());
    Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0xff,0xff,0xff));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_A(100));	
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(EDGE_STRIP_R));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((hide_x+FT_DispWidth-80)*16,0));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((hide_x+FT_DispWidth-80)*16,FT_DispHeight*16));
    Ft_App_WrCoCmd_Buffer(phost,COLOR_A(255));
    Ft_Gpu_Radiobutton(hide_x+FT_DispWidth-70,FT_DispHeight-48,0xffffff,0,8,3,opt);		
    Ft_Gpu_Radiobutton(hide_x+FT_DispWidth-70,FT_DispHeight-28,0xffffff,0,8,4,opt);
    Ft_Gpu_Radiobutton(hide_x+FT_DispWidth-70,FT_DispHeight-8 ,0xffffff,0,8,5,opt);
    Ft_Gpu_Radiobutton(hide_x+FT_DispWidth-70,FT_DispHeight-68,0xffffff,0,8,6,opt);
    Ft_Gpu_CoCmd_Text(phost,hide_x+FT_DispWidth-60,FT_DispHeight-48,26,OPT_CENTERY,"Sine");
    Ft_Gpu_CoCmd_Text(phost,hide_x+FT_DispWidth-60,FT_DispHeight-28,26,OPT_CENTERY,"Sawtooth");
    Ft_Gpu_CoCmd_Text(phost,hide_x+FT_DispWidth-60,FT_DispHeight-8 ,26,OPT_CENTERY,"Triangle");
    Ft_Gpu_CoCmd_Text(phost,hide_x+FT_DispWidth-60,FT_DispHeight-68,26,OPT_CENTERY,"ECG");
    Ft_Gpu_CoCmd_Text(phost,(hide_x+FT_DispWidth-60),20,30,OPT_CENTERY|OPT_CENTERX,"-");
    Ft_Gpu_CoCmd_Text(phost,(hide_x+FT_DispWidth-20),20,30,OPT_CENTERY|OPT_CENTERX,"+");
    Ft_Gpu_CoCmd_Text(phost,(hide_x+FT_DispWidth-80),50,28,0,"Rate:");
    Ft_Gpu_CoCmd_Number(phost,(hide_x+FT_DispWidth-30),50,28,0,rate);		
    Ft_Gpu_CoCmd_Text(phost,(hide_x+FT_DispWidth-80),80,28,0,"Pk:");
    Ft_Gpu_CoCmd_Number(phost,(hide_x+FT_DispWidth-40),80,28,0,amp);		
    Ft_App_WrCoCmd_Buffer(phost,COLOR_A(50));
    Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(15*16));
    Ft_App_WrCoCmd_Buffer(phost,BEGIN(FTPOINTS));
    Ft_App_WrCoCmd_Buffer(phost,TAG(1));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((hide_x+FT_DispWidth-60)*16,20*16));		
    Ft_App_WrCoCmd_Buffer(phost,TAG(2));
    Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((hide_x+FT_DispWidth-20)*16,20*16));	
   
    Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
    Ft_Gpu_CoCmd_Swap(phost);
    Ft_App_Flush_Co_Buffer(phost);
    Ft_Gpu_Hal_WaitCmdfifo_empty(phost); 
//==========================End==================================================    
  }
}  


#if defined MSVC_PLATFORM || defined FT900_PLATFORM
/* Main entry point */
ft_int32_t main(ft_int32_t argc,ft_char8_t *argv[])
#endif
#if  defined(ARDUINO_PLATFORM) || defined(MSVC_FT800EMU)
ft_void_t setup()
#endif
{

#ifdef FT900_PLATFORM
	FT900_Config();
#endif

	Ft_Gpu_HalInit_t halinit;
	
	halinit.TotalChannelNum = 1;

              
	Ft_Gpu_Hal_Init(&halinit);
	host.hal_config.channel_no = 0;
	host.hal_config.pdn_pin_no = FT800_PD_N;
	host.hal_config.spi_cs_pin_no = FT800_SEL_PIN;
    #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)
	    host.hal_config.spi_clockrate_khz = 12000; //in KHz
    #elif defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
        host.hal_config.spi_clockrate_khz = 20000; //in KHz
    #endif
#ifdef ARDUINO_PLATFORM_SPI
	host.hal_config.spi_clockrate_khz = 4000; //in KHz
#endif
        Ft_Gpu_Hal_Open(&host);
            
	phost = &host;

    Ft_BootupConfig();

#if ((defined FT900_PLATFORM) || defined(MSVC_PLATFORM))
	printf("\n reg_touch_rz =0x%x ", Ft_Gpu_Hal_Rd16(phost, REG_TOUCH_RZ));
	printf("\n reg_touch_rzthresh =0x%x ", Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_RZTHRESH));
    printf("\n reg_touch_tag_xy=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG_XY));
	printf("\n reg_touch_tag=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG));
#endif


    /*It is optional to clear the screen here*/	
    Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);
    
    Ft_Gpu_Hal_Sleep(1000);//Show the booting up screen. 



	home_setup();
	Info();
	Waves();   
	/* Close all the opened handles */
    Ft_Gpu_Hal_Close(phost);
    Ft_Gpu_Hal_DeInit();
#ifdef MSVC_PLATFORM
	return 0;
#endif
}

void loop()
{
}



/* Nothing beyond this */













