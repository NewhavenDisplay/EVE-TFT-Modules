/*

* Copyright (c) Future Technology Devices International 2015

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.
 */


#include "FT_Platform.h"
#ifdef FT900_PLATFORM
#include "ff.h"
#endif

#define REPORT_FRAMES   0
#if defined(DISPLAY_RESOLUTION_WQVGA) || defined(DISPLAY_RESOLUTION_QVGA) || defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
	#define SUBDIV  4
	#define YY      (480 / SUBDIV)
#elif defined(DISPLAY_RESOLUTION_WVGA)
	#define SUBDIV  12
	#define YY      ((800 +SUBDIV)/ SUBDIV)
	#define MIN_PIXELS_PER_DIV 55
#endif

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

/* Initial boot up DL - make the back ground green color */
const ft_uint8_t FT_DLCODE_BOOTUP[12] = 
{
  255,255,255,2,//GPU instruction CLEAR_COLOR_RGB
  7,0,0,38, //GPU instruction CLEAR
  0,0,0,0,  //GPU instruction DISPLAY
};

ft_float_t transform_m,transform_c;

#if defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
ft_float_t m_min = 13.0/65536;

#define SUBDIV  4
#define YY      (480 / SUBDIV)
#endif
#if defined(DISPLAY_RESOLUTION_WQVGA) || defined(DISPLAY_RESOLUTION_QVGA)
	ft_float_t m_min = 13.0/65536;
#elif defined(DISPLAY_RESOLUTION_WVGA)
	ft_float_t m_min = 26.0/65536;
#endif


/* Global used for buffer optimization */
Ft_Gpu_Hal_Context_t host,*phost;

#ifdef FT900_PLATFORM


FATFS            FatFs;				// FatFs work area needed for each volume
FIL 			 CurFile;
FRESULT          fResult;
SDHOST_STATUS    SDHostStatus;
#endif

ft_uint32_t Ft_CmdBuffer_Index;
ft_uint32_t Ft_DlBuffer_Index;

#ifdef BUFFER_OPTIMIZATION
ft_uint8_t  Ft_DlBuffer[FT_DL_SIZE];
ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif

#if defined(FT900_PLATFORM)
DWORD get_fattime(void){
	return 0;
}
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
        "Welcome to Graph Example ... \r\n"
        "\r\n"
        "--------------------------------------------------------------------- \r\n"
        );

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
//printf("ft900 config done \n");}
}
#endif

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
				printf("VC1 register ID after wake up %x\n",chipid);
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

	/* Reconfigure the SPI */
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

/* Boot up for FT800 */

ft_void_t Info()
{
  ft_uint16_t dloffset = 0,z;
  Ft_CmdBuffer_Index = 0;
  Ft_Gpu_CoCmd_Dlstart(phost); 
  Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
  Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,255));
  Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,FT_DispHeight/2,28,OPT_CENTERX|OPT_CENTERY,"Please tap on a dot");
  Ft_Gpu_CoCmd_Calibrate(phost,0);

  Ft_App_Flush_Co_Buffer(phost);
  Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
}

ft_void_t read_extended(ft_int16_t sx[5], ft_int16_t sy[5])
{
  ft_uint32_t sxy0, sxyA, sxyB, sxyC;
  sxy0 = Ft_Gpu_Hal_Rd32(phost,REG_CTOUCH_TOUCH0_XY);
  sxyA = Ft_Gpu_Hal_Rd32(phost,REG_CTOUCH_TOUCH1_XY);
  sxyB = Ft_Gpu_Hal_Rd32(phost,REG_CTOUCH_TOUCH2_XY);
  sxyC = Ft_Gpu_Hal_Rd32(phost,REG_CTOUCH_TOUCH3_XY);

  sx[0] = sxy0 >> 16;
  sy[0] = sxy0;
  sx[1] = sxyA >> 16;
  sy[1] = sxyA;
  sx[2] = sxyB >> 16;
  sy[2] = sxyB;
  sx[3] = sxyC >> 16;
  sy[3] = sxyC;

  sx[4] = Ft_Gpu_Hal_Rd16(phost,REG_CTOUCH_TOUCH4_X);
  sy[4] = Ft_Gpu_Hal_Rd16(phost,REG_CTOUCH_TOUCH4_Y);
}
/* API used for single touch usecases */
ft_void_t read_compatible(ft_int16_t sx[5], ft_int16_t sy[5])
{
  ft_uint32_t sxy0, sxyA, sxyB, sxyC;
  sxy0 = Ft_Gpu_Hal_Rd32(phost,REG_TOUCH_SCREEN_XY);
  sxyA = 0x80008000;
  sxyB = 0x80008000;
  sxyC = 0x80008000;

  sx[0] = sxy0 >> 16;
  sy[0] = sxy0;
  sx[1] = sxyA >> 16;
  sy[1] = sxyA;
  sx[2] = sxyB >> 16;
  sy[2] = sxyB;
  sx[3] = sxyC >> 16;
  sy[3] = sxyC;

  sx[4] = 0x8000;
  sy[4] = 0x8000;
}

ft_void_t set(ft_int32_t x0, ft_int16_t y0,
           ft_int32_t x1, ft_int16_t y1) {
    ft_int32_t xd = x1 - x0;
    ft_int16_t yd = y1 - y0;
    transform_m = yd / (ft_float_t)xd;
    if (transform_m < m_min)
       transform_m = m_min; 
    transform_c = y0 - transform_m * x0;
  }
ft_void_t sset(ft_int32_t x0, ft_int16_t y0) 
{
    transform_c = (ft_float_t)y0 - transform_m * x0;
}
ft_int16_t m2s(ft_int32_t x) 
{
    return (ft_int16_t)(transform_m * x + transform_c);
}
ft_int32_t s2m(ft_int16_t y) 
{
    return (ft_int32_t)(y - transform_c) / transform_m;
}


FT_PROGMEM ft_prog_uint16_t sintab[257] = {
0, 402, 804, 1206, 1608, 2010, 2412, 2813, 3215, 3617, 4018, 4419, 4821, 5221, 5622, 6023, 6423, 6823, 7223, 7622, 8022, 8421, 8819, 9218, 9615, 10013, 10410, 10807, 11203, 11599, 11995, 12390, 12785, 13179, 13573, 13966, 14358, 14750, 15142, 15533, 15923, 16313, 16702, 17091, 17479, 17866, 18252, 18638, 19023, 19408, 19791, 20174, 20557, 20938, 21319, 21699, 22078, 22456, 22833, 23210, 23585, 23960, 24334, 24707, 25079, 25450, 25820, 26189, 26557, 26924, 27290, 27655, 28019, 28382, 28744, 29105, 29465, 29823, 30181, 30537, 30892, 31247, 31599, 31951, 32302, 32651, 32999, 33346, 33691, 34035, 34378, 34720, 35061, 35400, 35737, 36074, 36409, 36742, 37075, 37406, 37735, 38063, 38390, 38715, 39039, 39361, 39682, 40001, 40319, 40635, 40950, 41263, 41574, 41885, 42193, 42500, 42805, 43109, 43411, 43711, 44010, 44307, 44603, 44896, 45189, 45479, 45768, 46055, 46340, 46623, 46905, 47185, 47463, 47739, 48014, 48287, 48558, 48827, 49094, 49360, 49623, 49885, 50145, 50403, 50659, 50913, 51165, 51415, 51664, 51910, 52155, 52397, 52638, 52876, 53113, 53347, 53580, 53810, 54039, 54265, 54490, 54712, 54933, 55151, 55367, 55581, 55793, 56003, 56211, 56416, 56620, 56821, 57021, 57218, 57413, 57606, 57796, 57985, 58171, 58355, 58537, 58717, 58894, 59069, 59242, 59413, 59582, 59748, 59912, 60074, 60234, 60391, 60546, 60699, 60849, 60997, 61143, 61287, 61428, 61567, 61704, 61838, 61970, 62100, 62227, 62352, 62474, 62595, 62713, 62828, 62941, 63052, 63161, 63267, 63370, 63472, 63570, 63667, 63761, 63853, 63942, 64029, 64114, 64196, 64275, 64353, 64427, 64500, 64570, 64637, 64702, 64765, 64825, 64883, 64938, 64991, 65042, 65090, 65135, 65178, 65219, 65257, 65293, 65326, 65357, 65385, 65411, 65435, 65456, 65474, 65490, 65504, 65515, 65523, 65530, 65533, 65535
};

#if defined(FT900_PLATFORM)
ft_uint16_t __flash__ const *element = &sintab;
#endif

ft_int16_t rsin(ft_int16_t r, ft_uint16_t th) 
{
  ft_int32_t th4 ; ft_uint16_t s;ft_int16_t p;
  //element = &sintab;
  th >>= 6; // angle 0-123
  // return int(r * sin((2 * M_PI) * th / 1024.));
  th4 = th & 511;
  if (th4 & 256)
    th4 = 512 - th4; // 256->256 257->255, etc
#if defined(FT900_PLATFORM)
  s = *(element + th4);
#else
  s = ft_pgm_read_byte_near(sintab + th4);
#endif
  p = ((ft_uint32_t)s * r) >> 16;
  if (th & 512)
    p = -p;
  return p;
}

ft_void_t plot()
{
	ft_int32_t mm[2],m,j;
	byte fadeout,h,i;
	ft_int16_t pixels_per_div,x,clock_r,x1;
	ft_int32_t y[YY + 1];
	ft_uint32_t x32 ;
	ft_uint16_t x2, options;

	Ft_App_WrCoCmd_Buffer(phost,STENCIL_OP(ZERO, ZERO));
	//Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0x202020, 0, 0x11f, 0x107fff);
	//Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0x202020, 0, FT_DispHeight/16, 0x107fff);
	Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0x202020, 0, FT_DispHeight, 0x107fff);


	mm[0] = s2m(0);
	mm[1] = s2m(FT_DispWidth);
	pixels_per_div = m2s(0x4000) - m2s(0);


#if defined(DISPLAY_RESOLUTION_WVGA)
	fadeout = min(255, max(0, (pixels_per_div - 60) * 16));
#else
	fadeout = min(255, max(0, (pixels_per_div - 32) * 16));
#endif
	Ft_App_WrCoCmd_Buffer(phost, LINE_WIDTH(max(8, pixels_per_div >> 2)));
	for (m = mm[0] & ~0x3fff; m <= mm[1]; m += 0x4000) 
	{
		x = m2s(m);
		//if ((-60 <= x) && (x <= 512)) 
		if ((-60 <= x) && (x <= (FT_DispWidth+60)))
		{
		   h = 3 * (7 & (m >> 14));

		  Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0,0,0));
		  Ft_App_WrCoCmd_Buffer(phost,COLOR_A(((h == 0) ? 192 : 64)));
		  Ft_App_WrCoCmd_Buffer(phost, BEGIN(LINES));
		  Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,0));
		  //Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,272*16));
		  Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(x*16,FT_DispHeight*16));

		  if (fadeout) 
		  {
			x -= 1;
			Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xd0,0xd0,0xd0));
			Ft_App_WrCoCmd_Buffer(phost,COLOR_A(fadeout));
#if defined(DISPLAY_RESOLUTION_QVGA) | defined(DISPLAY_RESOLUTION_WQVGA)
			Ft_Gpu_CoCmd_Number(phost,x, 0, 26, OPT_RIGHTX | 2, h);
			Ft_Gpu_CoCmd_Text(phost,x, 0, 26, 0, ":00");
#elif defined(DISPLAY_RESOLUTION_WVGA)
			Ft_Gpu_CoCmd_Number(phost,x, 0, 30, OPT_RIGHTX | 2, h);
			Ft_Gpu_CoCmd_Text(phost,x, 0, 30, 0, ":00");
#endif
#ifdef  DISPLAY_RESOLUTION_HVGA_PORTRAIT

			Ft_Gpu_CoCmd_Number(phost,x, 0, 28, OPT_RIGHTX | 2, h);
			Ft_Gpu_CoCmd_Text(phost,x, 0, 28, 0, ":00");

#endif
		  }
		}
	}
	Ft_App_WrCoCmd_Buffer(phost,COLOR_A(255));

	for (i = 0; i < (YY + 1); i++) 
	{
		x32 = s2m(SUBDIV * i);
		x2 = (ft_uint16_t)x32 + rsin(7117, x32);
		y[i] = 130 * 16 + rsin(1200, (217 * x32) >> 8) + rsin(700, 3 * x2);
	}

	Ft_App_WrCoCmd_Buffer(phost,STENCIL_OP(INCR, INCR));
	Ft_App_WrCoCmd_Buffer(phost, BEGIN(EDGE_STRIP_B));
	for (j = 0; j < (YY + 1); j++)
	{
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(16 * SUBDIV * j, y[j]));
	}
	Ft_App_WrCoCmd_Buffer(phost,STENCIL_FUNC(EQUAL, 1, 255));
	Ft_App_WrCoCmd_Buffer(phost,STENCIL_OP(KEEP, KEEP));
	//Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0xf1b608, 0, 220, 0x98473a);
	//Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0xf1b608, 0, FT_DispHeight/1.236, 0x98473a);
	Ft_Gpu_CoCmd_Gradient(phost,0, 0, 0xf1b608, 0, FT_DispHeight, 0x98473a);

	Ft_App_WrCoCmd_Buffer(phost, STENCIL_FUNC(ALWAYS, 1, 255));
	Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xE0,0xE0,0xE0));
	Ft_App_WrCoCmd_Buffer(phost, LINE_WIDTH(24));
	Ft_App_WrCoCmd_Buffer(phost, BEGIN(LINE_STRIP));

	for (j = 0; j < (YY + 1); j++)
	{
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(16 * SUBDIV * j, y[j]));
	}


#if defined(DISPLAY_RESOLUTION_QVGA) || defined(DISPLAY_RESOLUTION_WQVGA)
	clock_r = min(24, pixels_per_div >> 2);
#elif defined(DISPLAY_RESOLUTION_WVGA)
	clock_r = min(48, pixels_per_div >> 2);
#elif  DISPLAY_RESOLUTION_HVGA_PORTRAIT

	clock_r = min(48, pixels_per_div >> 2);

#endif

#if defined(FT900_PLATFORM)
	if (clock_r > 4 && fadeout)
#else
	if (clock_r > 4) 
#endif
	{
		Ft_App_WrCoCmd_Buffer(phost,COLOR_A(200));
		Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xff,0xff,0xff));
		options = OPT_NOSECS | OPT_FLAT;
		if (clock_r < 10)
		  options |= OPT_NOTICKS;
		for (m = mm[0] & ~0x3fff; m <= mm[1]; m += 0x4000) 
		{
		  x1 = m2s(m);
		  h = 3 * (3 & (m >> 14));
		  if(x1 >= -1024)
			Ft_Gpu_CoCmd_Clock(phost,x1, FT_DispHeight-clock_r, clock_r, options, h, 0, 0, 0);
		}
	}
}

#if defined(MSVC_PLATFORM) || defined(FT900_PLATFORM)
/* Main entry point */
ft_int32_t main(ft_int32_t argc,ft_char8_t *argv[])
#endif
#if defined(ARDUINO_PLATFORM) || defined(MSVC_FT800EMU)
ft_void_t setup()
#endif
{
	char *path = "..\\..\\..\\Test";
	ft_int16_t sx[5], sy[5];
	ft_int32_t f,j;
	static ft_int32_t _f;
	ft_int16_t cx = 0, cy = 0;
	static ft_bool_t down[2]={0,0};
	static ft_int32_t m[2]={0UL,0UL};

	byte n=0,i=0;


	 ft_uint8_t chipid;
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

   //printf("Ft_Gpu_Hal_Open done \n");
	phost = &host;

    Ft_BootupConfig();

#if ((defined FT900_PLATFORM) || defined(MSVC_PLATFORM))
	printf("\n reg_touch_rz =0x%x ", Ft_Gpu_Hal_Rd16(phost, REG_TOUCH_RZ));
	printf("\n reg_touch_rzthresh =0x%x ", Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_RZTHRESH));
   printf("\n reg_touch_tag_xy=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG_XY));
	printf("\n reg_touch_tag=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG));
#endif

	Info();
	set(0, 0, 0x10000, FT_DispWidth);

#if (defined(FT_801_ENABLE) || defined(FT_811_ENABLE) || defined(FT_813_ENABLE))
	Ft_Gpu_Hal_Wr8(phost,REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif

	while(1)
	{
		if (REPORT_FRAMES) {
			f = Ft_Gpu_Hal_Rd16(phost,REG_FRAMES);
			
			printf("Error : %d" , f - _f);
			_f = f;
		}
#if (defined(FT_801_ENABLE) || defined(FT_811_ENABLE) || defined(FT_813_ENABLE))
		read_extended(sx, sy);
#else
		read_compatible(sx, sy);
#endif
		for (i = 0; i < 2; i++) 
		{
			if (sx[i] > -10 && !down[i]) 
			{
				down[i] = 1;
				m[i] = s2m(sx[i]);
			}
			if (sx[i] < -10)
				down[i] = 0;
		}
		if (down[0] && down[1]) 
		{
			if (m[0] != m[1])
				set(m[0], sx[0], m[1], sx[1]);      
		}
		else if (down[0] && !down[1])
			sset(m[0], sx[0]);
		else if (!down[0] && down[1])
			sset(m[1], sx[1]);

		Ft_App_WrCoCmd_Buffer(phost, CMD_DLSTART);
		plot();

		// display touches
		if (0) 
		{
			Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xff,0xff,0xff));
			Ft_App_WrCoCmd_Buffer(phost, LINE_WIDTH(8));
			Ft_App_WrCoCmd_Buffer(phost, BEGIN(LINES));
			for (i = 0; i < 2; i++) 
			{
				if (sx[i] > -10) 
				{
#if defined(DISPLAY_RESOLUTION_QVGA) | defined(DISPLAY_RESOLUTION_WQVGA)
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2II(sx[i], 0,0,0));
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2II(sx[i], 272,0,0));
#elif defined(DISPLAY_RESOLUTION_WVGA)
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(sx[i]*16, 0));
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(sx[i]*16, FT_DispHeight*16));
#elif  defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(sx[i]*16, 0));
					Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(sx[i]*16, FT_DispHeight*16));
#endif

				}
			}
		}

		Ft_Gpu_Hal_WaitCmdfifo_empty(phost);

		Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
		Ft_Gpu_CoCmd_Swap(phost);
		Ft_Gpu_CoCmd_LoadIdentity(phost);
		Ft_App_Flush_Co_Buffer(phost);		
	}

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













