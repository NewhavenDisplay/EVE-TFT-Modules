/*

Copyright (c) Future Technology Devices International 2014

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

Author : FTDI 

Revision History: 
0.1 - date 2013.04.24 - Initial Version
0.2 - date 2013.08.19 - added few APIs

*/

#include "FT_Platform.h"


/***************************************************************************
* Interface Description    : API for USB-to-SPI/I2C bridge IC LIB initialize  
*							 For MSVC_PLATFORM the following actions are
*							 performed
*								-libMPSSE/libFT4222 init
*								-read info about SPI channels on usb-bridge chip 
*								 through lib service
* Implementation           : 
* Return Value             : ft_bool_t
*                            TRUE , FALSE
* Author                   : 
****************************************************************************/
ft_bool_t  Ft_Gpu_Hal_Init(Ft_Gpu_HalInit_t *halinit)
{
    #ifdef FT900_PLATFORM        
        // Initialize SPIM HW
        sys_enable(sys_device_spi_master);

        gpio_function(27, pad_spim_sck); /* GPIO27 to SPIM_CLK */
        gpio_function(28, pad_spim_ss0); /* GPIO28 as CS */
        gpio_function(29, pad_spim_mosi); /* GPIO29 to SPIM_MOSI */
        gpio_function(30, pad_spim_miso); /* GPIO30 to SPIM_MISO */

        gpio_dir(27, pad_dir_output);
        gpio_dir(28, pad_dir_output);
        gpio_dir(29, pad_dir_output);
        gpio_dir(30, pad_dir_input);
    #if (defined(ENABLE_SPI_QUAD))
        /* Initialize IO2 and IO3 pad/pin for quad settings */
        gpio_function(31, pad_spim_io2); /* GPIO31 to IO2 */
        gpio_function(32, pad_spim_io3); /* GPIO32 to IO3 */
        gpio_dir(31, pad_dir_output);
        gpio_dir(32, pad_dir_output);
    #endif
        gpio_write(28, 1);
        spi_init(SPIM, spi_dir_master, spi_mode_0, 16);//SPISysInit(SPIM);
    #endif

	#ifdef MSVC_PLATFORM
		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
				/* Initialize the libmpsse */
			Init_libMPSSE();
			SPI_GetNumChannels(&halinit->TotalChannelNum);
			/* By default i am assuming only one mpsse cable is connected to PC and channel 0 of that mpsse cable is used for spi transactions */
			if (halinit->TotalChannelNum > 0)
			{
				FT_DEVICE_LIST_INFO_NODE devList;
				SPI_GetChannelInfo(0, &devList);
				printf(" Information on channel number %d:\n", 0);
				/* print the dev info */
				printf(" Flags=0x%x\n", devList.Flags);
				printf(" Type=0x%x\n", devList.Type);
				printf(" ID=0x%x\n", devList.ID);
				printf(" LocId=0x%x\n", devList.LocId);
				printf(" SerialNumber=%s\n", devList.SerialNumber);
				printf(" Description=%s\n", devList.Description);
				printf(" ftHandle=0x%x\n", devList.ftHandle);/*is 0 unless open*/
			}
		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)
			{				
				FT_STATUS status;
				//ft_ulong_t numdevs;
                ft_uint32_t numdevs;

				status = FT_CreateDeviceInfoList(&numdevs);
				if (FT_OK == status)
				{
					printf("Number of D2xx devices connected = %d\n", numdevs);
					halinit->TotalChannelNum = numdevs;
				}
                else
                {
                    printf("FT_CreateDeviceInfoList failed");
                    return FALSE;
                }
			}
		#else
			#error "Usb bridge library configuration missing\n"
		#endif
	#endif
	return TRUE;
}

/***************************************************************************
* Interface Description    : To obtain handle for interface communication
* Implementation           :
* Return Value             : ft_bool_t
*                            TRUE - Configuration successful
*                            FALSE - Configuration failed
* Author                   :
****************************************************************************/
ft_bool_t    Ft_Gpu_Hal_Open(Ft_Gpu_Hal_Context_t *host)
{
    ft_bool_t ret = TRUE;

    #ifdef FT900_PLATFORM
	    gpio_function(host->hal_config.spi_cs_pin_no, pad_spim_ss0); /* GPIO28 as CS */
	    gpio_write(host->hal_config.spi_cs_pin_no, 1);

	    gpio_function(host->hal_config.pdn_pin_no, pad_gpio43);
	    gpio_dir(host->hal_config.pdn_pin_no, pad_dir_output);

		    gpio_write(host->hal_config.pdn_pin_no,1);
	#endif

	#ifdef MSVC_FT800EMU
		Ft_GpuEmu_SPII2C_begin();
	#endif

	#ifdef ARDUINO_PLATFORM_SPI
		pinMode(host->hal_config.pdn_pin_no, OUTPUT);
		digitalWrite(host->hal_config.pdn_pin_no, HIGH);
		pinMode(host->hal_config.spi_cs_pin_no, OUTPUT);
		digitalWrite(host->hal_config.spi_cs_pin_no, HIGH);
		SPI.begin();
		SPI.setClockDivider(SPI_CLOCK_DIV2);
		SPI.setBitOrder(MSBFIRST);
		SPI.setDataMode(SPI_MODE0);
	#endif

	#ifdef MSVC_PLATFORM
		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
			ChannelConfig channelConf;			//channel configuration
			FT_STATUS status;					
		
	        /* configure the spi settings */
	        channelConf.ClockRate = host->hal_config.spi_clockrate_khz * 1000; 
	        channelConf.LatencyTimer= 2;       
	        channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	        channelConf.Pin = 0x00000000;	/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

			/* Open the first available channel */
			SPI_OpenChannel(host->hal_config.channel_no,(FT_HANDLE *)&host->hal_handle);
			status = SPI_InitChannel((FT_HANDLE)host->hal_handle,&channelConf);
			printf("\nhandle=0x%x status=0x%x\n",host->hal_handle,status);	

			host->spichannel = 0;

		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)		
			FT_STATUS status;
			//ft_ulong_t numdevs;
            ft_uint32_t numdevs;
			ft_uint32_t index;
			FT_HANDLE fthandle;
			FT4222_Version pversion;
			FT4222_ClockRate ftclk = 0;
			ft_uint16_t max_size = 0;
            FT4222_ClockRate selclk = 0;
            FT4222_SPIClock seldiv = 0;            
								   /* GPIO0		 , GPIO1	  , GPIO2	   , GPIO3		 } */	
			GPIO_Dir gpio_dir[4] = { GPIO_OUTPUT , GPIO_INPUT , GPIO_INPUT, GPIO_INPUT };

           
            host->hal_handle = host->hal_handle2 = NULL;

            status = FT_CreateDeviceInfoList(&numdevs);
            if (FT_OK != status)
            {
                printf("FT_CreateDeviceInfoList failed");
                ret = FALSE;
            }

			status = FT_ListDevices(&numdevs, NULL, FT_LIST_NUMBER_ONLY);
            if (FT_OK != status)
            {              
                printf("FT_ListDevices failed");                
                ret = FALSE;
            }

            if (ret)
            {
                for (index = 0; (index<numdevs) && ret; index++)
                {
                    FT_DEVICE_LIST_INFO_NODE devInfo;
                    memset(&devInfo, 0, sizeof(devInfo));

                    status = FT_GetDeviceInfoDetail(index,
                        &devInfo.Flags, &devInfo.Type, &devInfo.ID, &devInfo.LocId,
                        devInfo.SerialNumber, devInfo.Description, &devInfo.ftHandle);
                    if (FT_OK == status)
                    {
                        printf("Dev %d:\n", index);
                        printf(" Flags= 0x%x, (%s) (%s) \n", devInfo.Flags,
                            ((devInfo.Flags & 0x01) ? "DEVICE_OPEN" : "DEVICE_CLOSED"), ((devInfo.Flags & 0x02) ? "High-speed USB" : "Full-speed USB"));
                        printf(" Type= 0x%x\n", devInfo.Type);
                        printf(" ID= 0x%x\n", devInfo.ID);
                        printf(" LocId= 0x%x\n", devInfo.LocId);
                        printf(" SerialNumber= %s\n", devInfo.SerialNumber);
                        printf(" Description= %s\n", devInfo.Description);
                        printf(" ftHandle= %p\n", devInfo.ftHandle);
                    }
                    else
                        ret = FALSE;

                    if(ret && !(devInfo.Flags & 0x01) && ((!strcmp(devInfo.Description, "FT4222 A") && (host->hal_handle == NULL)) || (!strcmp(devInfo.Description, "FT4222 B") && (host->hal_handle2 == NULL))))
                    {                        
                        /* obtain handle for the first discovered "FT4222 A" and first "FT4222 B" */
                        status = FT_OpenEx(devInfo.Description, FT_OPEN_BY_DESCRIPTION, &fthandle);
                        if (status != FT_OK)
                        {
                            printf("FT_OpenEx failed %d\n", status);
                            ret = FALSE;
                        }
                        else
                        {
                            if (!strcmp(devInfo.Description, "FT4222 A"))
                            {
                                //is SPI
                                host->hal_handle = fthandle; //SPI communication handle
                                printf("[%d]th of total connected devices is FT4222 A (SPI) : host->hal_hanlde = %p\n", index + 1, host->hal_handle);
                            }
                            else if (!strcmp(devInfo.Description, "FT4222 B"))
                            {
                                //is GPIO
                                host->hal_handle2 = fthandle; //GPIO communication handle
                                printf("[%d]th of total connected devices is FT4222 B (GPIO) : host->hal_hanlde = %p\n", index + 1, host->hal_handle);
                            }
                            else
                            {
                                printf("Error in FT4222 configuration\n");
                            }
                        }                       
                    }
                    else
                    {
                        if(
                            (!strcmp(devInfo.Description, "FT4222 A") && host->hal_handle != NULL ) ||
                            (!strcmp(devInfo.Description, "FT4222 B") && host->hal_handle2 != NULL)
                          )
                            printf("[%d]th of total connected devices is not the first %s detected. Hence skipping.\n", index + 1, devInfo.Description);  
                        else if(devInfo.Flags & 0x01)
                            printf("[%d]th of total connected devices is already open in another context. Hence skipping.\n", index + 1);
                        else
                            printf("[%d]th of total connected devices is not FT4222 but is %s. Hence skipping.\n", index+1 , devInfo.Description);                        
                        continue;
                    }
                }
            }

            if (ret)
            {
                status = FT4222_GetVersion(host->hal_handle, &pversion);
                if (status != FT4222_OK)
                    printf("FT4222_GetVersion failed\n");
                else
                    printf("SPI:chipversion = 0x%x\t dllversion = 0x%x\n", pversion.chipVersion, pversion.dllVersion);
            }

            if (ret)
            {
                //Set default Read timeout 5s and Write timeout 5sec
                status = FT_SetTimeouts(host->hal_handle, FT4222_ReadTimeout, FT4222_WriteTimeout);
                if (FT_OK != status)
                {               
                    printf("FT_SetTimeouts failed!\n");                 
                    ret = FALSE;
                }
            }
				
            if (ret)
            {
                // no latency to usb
                status = FT_SetLatencyTimer(host->hal_handle, FT4222_LatencyTime);
                if (FT_OK != status)
                {                    
                    printf("FT_SetLatencyTimerfailed!\n");
                    ret = FALSE;
                }
            }
            
            if (ret)
            {
                if (!Ft_Gpu_Hal_FT4222_ComputeCLK(host, &selclk, &seldiv))
                {
                    printf("Requested clock %d KHz is not supported in FT4222 \n", host->hal_config.spi_clockrate_khz);
                    ret = FALSE;
                }
            }

            if(ret)
            {                
                status = FT4222_SetClock(host->hal_handle, selclk);
                if (FT_OK != status)
                {                   
                    printf("FT4222_SetClock failed!\n");                    
                    ret = FALSE;
                }

                status = FT4222_GetClock(host->hal_handle, &ftclk);

                if (FT_OK != status)                
                       printf("FT4222_SetClock failed\n");                
                else
                    printf("FT4222 clk = %d\n", ftclk);

            }            

			if(ret)
            {
				/* Interface 1 is SPI master */                
				status = FT4222_SPIMaster_Init(
												host->hal_handle,
												SPI_IO_SINGLE,
												seldiv,		
												CLK_IDLE_LOW, //,CLK_IDLE_HIGH
                                                CLK_LEADING,// CLK_LEADING CLK_TRAILING
                                                host->hal_config.spi_cs_pin_no
                                              );	/* slave selection output pins */                           
				if (FT_OK != status) 
				{                   
				    printf("Init FT4222 as SPI master device failed!\n");					
                    ret = FALSE;
				}
				else					
                    host->spichannel = FT_GPU_SPI_SINGLE_CHANNEL; //SPI_IO_SINGLE;				

				status = FT4222_SPI_SetDrivingStrength(host->hal_handle, DS_4MA, DS_4MA, DS_4MA);
				if (FT4222_OK != status)
				    printf("FT4222_SPI_SetDrivingStrength failed!\n");					                    
				
				Ft_Gpu_Hal_Sleep(20);				

                status = FT4222_SetSuspendOut(host->hal_handle2, FALSE);
                if (FT_OK != status)
                {
                    printf("Disable suspend out function on GPIO2 failed!\n");
                    ret = FALSE;
                }

                status = FT4222_SetWakeUpInterrupt(host->hal_handle2, FALSE);
                if (FT_OK != status)
                {
                    printf("Disable wakeup/interrupt feature on GPIO3 failed!\n");
                    ret = FALSE;
                }
				/* Interface 2 is GPIO */
				status = FT4222_GPIO_Init(host->hal_handle2, gpio_dir);
				if (FT_OK != status)
				{                   
				    printf("Init FT4222 as GPIO interface failed!\n");                    
                    ret = FALSE;
				}
			}					

            /* dedicated write buffer used for SPI write. Max size is 2^uint16 */
            if ((host->spiwrbuf_ptr = malloc(FT4222_DYNAMIC_ALLOCATE_SIZE)) == NULL)
            {
                printf("malloc error\n");
                ret = FALSE;
            }

		#else	
			#error "USB-to-SPI bridge configuration missing"
		#endif
	#endif

	/* Initialize the context valriables */
	host->ft_cmd_fifo_wp = host->ft_dl_buff_wp = 0;
	host->spinumdummy = FT_GPU_SPI_ONEDUMMY; //by default ft800/801/810/811 goes with single dummy byte for read	
	host->status = FT_GPU_HAL_OPENED;

    return ret;	
}

/***************************************************************************
* Interface Description    : Free global wr_buf, release LibFT4222 resources
*                            and release SPI communication handle
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t  Ft_Gpu_Hal_Close(Ft_Gpu_Hal_Context_t *host)
{	
	#if defined(MSVC_PLATFORM) && defined (MSVC_PLATFORM_SPI_LIBFT4222)
    FT_STATUS ftstatus;
	FT4222_STATUS status;
	#endif

	host->status = FT_GPU_HAL_CLOSED;
	#ifdef MSVC_PLATFORM	
	/* Close the channel*/
		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
			SPI_CloseChannel(host->hal_handle);
		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)     

        free(host->spiwrbuf_ptr);

		if (FT4222_OK != (status = FT4222_UnInitialize(host->hal_handle)))
			printf("FT4222_UnInitialize failed %d \n",status);
		
		if (FT4222_OK != (status = FT4222_UnInitialize(host->hal_handle2)))
			printf("FT4222_UnInitialize failed %d \n", status);
		
		if(FT_OK != (ftstatus = FT_Close(host->hal_handle)))
			printf("FT_CLOSE failed %d \n", ftstatus);
		
		if (FT_OK != (ftstatus = FT_Close(host->hal_handle2)))
			printf("FT_CLOSE failed %d \n", ftstatus);
		#else
			#error "Lib configuration missing"
		#endif
	#endif
	#ifdef ARDUINO_PLATFORM_SPI
			SPI.end();
	#endif
	#ifdef MSVC_FT800EMU
		Ft_GpuEmu_SPII2C_end();
	#endif
	#ifdef FT900_PLATFORM
		//spi_close(SPIM,0);
	#endif
}

/***************************************************************************
* Interface Description    : Uninitialize
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_DeInit()
{
    #ifdef MSVC_PLATFORM
       //Cleanup the MPSSE Lib
	    #if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
		    Cleanup_libMPSSE();	 
	    #endif
    #endif
    #ifdef FT900_PLATFORM
       spi_uninit(SPIM);
    #endif
}

/***************************************************************************
* Interface Description    : The APIs for reading/writing transfer continuously
*							 only with small buffer system
*                            For FT4222Lib, this function is obsolete 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t  Ft_Gpu_Hal_StartTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw,ft_uint32_t addr)
{
	if (FT_GPU_READ == rw)
	{ 
        //EVE mem read
        #ifdef FT900_PLATFORM
		    ft_uint8_t spidata[4];
		    spidata[0] = (addr >> 16);
		    spidata[1] = (addr >> 8);
		    spidata[2] = addr & 0xff;
		    spi_open(SPIM, host->hal_config.spi_cs_pin_no);

		    spi_writen(SPIM, spidata, 3);
        #endif

        #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)	       
			ft_uint8_t Transfer_Array[4];
			ft_uint32_t SizeTransfered;

			/* Compose the read packet */
			Transfer_Array[0] = addr >> 16;
			Transfer_Array[1] = addr >> 8;
			Transfer_Array[2] = addr;

			Transfer_Array[3] = 0; //Dummy Read byte
			SPI_Write(
				(FT_HANDLE)host->hal_handle,
				Transfer_Array,
				sizeof(Transfer_Array),
				&SizeTransfered,
				SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
				);	       
        #endif

        #ifdef ARDUINO_PLATFORM_SPI
		    digitalWrite(host->hal_config.spi_cs_pin_no, LOW);
		    SPI.transfer(addr >> 16);
		    SPI.transfer(highByte(addr));
		    SPI.transfer(lowByte(addr));

		SPI.transfer(0); //Dummy Read Byte
#endif

#ifdef MSVC_FT800EMU
		Ft_GpuEmu_SPII2C_StartRead(addr);
#endif
		host->status = FT_GPU_HAL_READING;
	}
	else
	{
        //EVE mem write
        #ifdef FT900_PLATFORM
		    ft_uint8_t spidata[4];
		    spidata[0] = (0x80 | (addr >> 16));
		    spidata[1] = (addr >> 8);
		    spidata[2] = addr;

		    spi_open(SPIM, host->hal_config.spi_cs_pin_no);
		    spi_writen(SPIM, spidata, 3);
        #endif

        #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)	        
			ft_uint8_t Transfer_Array[3];
			ft_uint32_t SizeTransfered;

			/* Compose the read packet */
			Transfer_Array[0] = (0x80 | (addr >> 16));
			Transfer_Array[1] = addr >> 8;
			Transfer_Array[2] = addr;
			SPI_Write(
				(FT_HANDLE)host->hal_handle,
				Transfer_Array,
				3,
				&SizeTransfered,
				SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
				);	        
        #endif

        #ifdef ARDUINO_PLATFORM_SPI
		    digitalWrite(host->hal_config.spi_cs_pin_no, LOW);
		    SPI.transfer(0x80 | (addr >> 16));
		    SPI.transfer(highByte(addr));
		    SPI.transfer(lowByte(addr));
        #endif

        #ifdef MSVC_FT800EMU
		    Ft_GpuEmu_SPII2C_StartWrite(addr);
        #endif
		host->status = FT_GPU_HAL_WRITING;
	}
}

/***************************************************************************
* Interface Description    : The APIs for writing transfer continuously to RAM_CMD
*                            For FT4222Lib, this function is obsolete 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t  Ft_Gpu_Hal_StartCmdTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw, ft_uint16_t count)
{     
    Ft_Gpu_Hal_StartTransfer(host,rw,host->ft_cmd_fifo_wp + RAM_CMD);    
}

/***************************************************************************
* Interface Description    : 
*                            For FT4222Lib, this function is obsolete
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_TransferString(Ft_Gpu_Hal_Context_t *host,const ft_char8_t *string)
{    
    ft_uint16_t length = strlen(string);
    while(length --)
    {
        Ft_Gpu_Hal_Transfer8(host,*string);
        string ++;
    }
    //Append one null as ending flag
    Ft_Gpu_Hal_Transfer8(host,0);    
}

/***************************************************************************
* Interface Description    :Function to tranfer byte using SPI_Write()
*                           For FT4222Lib, this function is obsolete
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_Transfer8(Ft_Gpu_Hal_Context_t *host,ft_uint8_t value)
{
    #ifdef FT900_PLATFORM
	    ft_uint8_t ReadByte;
	    if (host->status == FT_GPU_HAL_WRITING)
	    {
		    spi_write(SPIM, value);
	    }
	    else
	    {
		    spi_read(SPIM, ReadByte);
	    }
	    return ReadByte;
    #endif

    #ifdef ARDUINO_PLATFORM_SPI
	    return SPI.transfer(value);
    #endif

    #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)	    
		ft_uint32_t SizeTransfered;
		if (host->status == FT_GPU_HAL_WRITING) {
			SPI_Write(host->hal_handle, &value, sizeof(value), &SizeTransfered, SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
		}
		else {
			SPI_Read(host->hal_handle, &value, sizeof(value), &SizeTransfered, SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
		}

	if (SizeTransfered != sizeof(value))
		host->status = FT_GPU_HAL_STATUS_ERROR;
        return value;
#endif	

#ifdef MSVC_FT800EMU
	return Ft_GpuEmu_SPII2C_transfer(value);
#endif
}

/***************************************************************************
* Interface Description    :
*                            For FT4222Lib, this function is obsolete
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint16_t  Ft_Gpu_Hal_Transfer16(Ft_Gpu_Hal_Context_t *host,ft_uint16_t value)
{
	ft_uint16_t retVal = 0;

    if (host->status == FT_GPU_HAL_WRITING)
	{
		Ft_Gpu_Hal_Transfer8(host,value & 0xFF);//LSB first
		Ft_Gpu_Hal_Transfer8(host,(value >> 8) & 0xFF);
	}
	else
	{
		retVal = Ft_Gpu_Hal_Transfer8(host,0);
		retVal |= (ft_uint16_t)Ft_Gpu_Hal_Transfer8(host,0) << 8;
	}

	return retVal;
}

/***************************************************************************
* Interface Description    : 
*                            For FT4222Lib, this function is obsolete
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint32_t  Ft_Gpu_Hal_Transfer32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t value)
{
	ft_uint32_t retVal = 0;
    
	if (host->status == FT_GPU_HAL_WRITING)
	{
		Ft_Gpu_Hal_Transfer16(host,value & 0xFFFF);//LSB first
		Ft_Gpu_Hal_Transfer16(host,(value >> 16) & 0xFFFF);
	}
	else
	{
		retVal = Ft_Gpu_Hal_Transfer16(host,0);
		retVal |= (ft_uint32_t)Ft_Gpu_Hal_Transfer16(host,0) << 16;
	}
	return retVal;
}

/***************************************************************************
* Interface Description    : Inactivate CS
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t   Ft_Gpu_Hal_EndTransfer(Ft_Gpu_Hal_Context_t *host)
{

	#ifdef FT900_PLATFORM
		spi_close(SPIM, host->hal_config.spi_cs_pin_no);
	#endif

	#if defined(MSVC_PLATFORM ) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)		
		//just disbale the CS - send 0 bytes with CS disable
		SPI_ToggleCS((FT_HANDLE)host->hal_handle,FALSE);		
	#endif

	#ifdef ARDUINO_PLATFORM_SPI
		digitalWrite(host->hal_config.spi_cs_pin_no, HIGH);
	#endif

#ifdef MSVC_FT800EMU
	Ft_GpuEmu_SPII2C_csHigh();
#endif
	host->status = FT_GPU_HAL_OPENED;
}

/***************************************************************************
* Interface Description    : EVE mem read API for 1 byte
* Implementation           : 
* Return Value             : ft_uint8_t
* Author                   : 
****************************************************************************/
ft_uint8_t  Ft_Gpu_Hal_Rd8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint8_t value = 0;   
	#ifdef FT900_PLATFORM
		ft_uint8_t spiData[4] = {0};
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
		spi_readn(SPIM,spiData,host->spinumdummy + 1);
		value = spiData[host->spinumdummy];
        Ft_Gpu_Hal_EndTransfer(host);
	#else
		#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
            if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, &value, sizeof(value)))
            {
                printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
            }           
		#else
			Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
			value = Ft_Gpu_Hal_Transfer8(host,0);
            Ft_Gpu_Hal_EndTransfer(host);
		#endif	
	#endif	
	return value;
}


/***************************************************************************
* Interface Description    : EVE mem read API for 2 byte
* Implementation           :
* Return Value             : ft_uint8_t
* Author                   :
****************************************************************************/
ft_uint16_t Ft_Gpu_Hal_Rd16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint16_t value;
	#ifdef FT900_PLATFORM
		ft_uint8_t spiData[4] = {0};
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
		spi_readn(SPIM,spiData,host->spinumdummy + 2);
		value = spiData[host->spinumdummy] |(spiData[host->spinumdummy+1] << 8) ;
        Ft_Gpu_Hal_EndTransfer(host);
	#else
		#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
            if (!Ft_Gpu_Hal_FT4222_Rd(host, addr,(ft_uint8_t *) &value, sizeof(value)))
            {
                printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
            }            
		#else
			Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
			value = Ft_Gpu_Hal_Transfer16(host,0);
            Ft_Gpu_Hal_EndTransfer(host);
		#endif
	#endif	
	return value;
}

/***************************************************************************
* Interface Description    : EVE mem read API for 4 byte
* Implementation           :
* Return Value             : ft_uint8_t
* Author                   :
****************************************************************************/
ft_uint32_t Ft_Gpu_Hal_Rd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint32_t value;
	#ifdef FT900_PLATFORM
		ft_uint8_t spiData[8] = {0};
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
		spi_readn(SPIM,spiData,host->spinumdummy + 4);
		value = (spiData[host->spinumdummy+3] << 24) | (spiData[host->spinumdummy+2] << 16) | (spiData[host->spinumdummy+1] << 8) | spiData[host->spinumdummy];
        Ft_Gpu_Hal_EndTransfer(host);
	#else
		#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)           
            if (!Ft_Gpu_Hal_FT4222_Rd(host, addr,(ft_uint8_t *) &value, sizeof(value)))
            {
                printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
            }           
		#else
			Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
			value = Ft_Gpu_Hal_Transfer32(host,0);
            Ft_Gpu_Hal_EndTransfer(host);
		#endif
	#endif	
	return value;
}

/***************************************************************************
* Interface Description    : EVE mem write API for 1 byte
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Wr8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t v)
{	
	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
		if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, &v, sizeof(v)))
		{
			printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
		}
	#else
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
		Ft_Gpu_Hal_Transfer8(host,v);
		Ft_Gpu_Hal_EndTransfer(host);
	#endif
}


/***************************************************************************
* Interface Description    : EVE mem write API for 2 byte
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Wr16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint16_t v)
{
	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)		
        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, &v, sizeof(v)))
		{
			printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
		}
	#else
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
		Ft_Gpu_Hal_Transfer16(host,v);
		Ft_Gpu_Hal_EndTransfer(host);
	#endif
}

/***************************************************************************
* Interface Description    : EVE mem write API for 4 byte
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Wr32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint32_t v)
{
	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, &v, sizeof(v)))
		{
			printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
		}
	#else
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
		Ft_Gpu_Hal_Transfer32(host,v);
		Ft_Gpu_Hal_EndTransfer(host);
	#endif
}

/***************************************************************************
* Interface Description    : Function to transfer the HOST CMD from host to  
*							 EVE through lib service
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_HostCommand(Ft_Gpu_Hal_Context_t *host,ft_uint8_t cmd)
{
	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
	FT4222_STATUS status;
	#endif

	#ifdef FT900_PLATFORM
	ft_uint8_t hcmd[4] = {0};
	hcmd[0] = cmd;
	hcmd[1] = 0;
	hcmd[2] = 0;
	hcmd[3] = 0;

		spi_open(SPIM,host->hal_config.spi_cs_pin_no);
		spi_writen(SPIM,hcmd,3);
		spi_close(SPIM,host->hal_config.spi_cs_pin_no);
	#endif

	#ifdef MSVC_PLATFORM		
			ft_uint8_t Transfer_Array[3];
			ft_uint32_t SizeTransfered;	
            ft_uint8_t dummy_read;

			Transfer_Array[0] = cmd;
			Transfer_Array[1] = 0;
			Transfer_Array[2] = 0;

		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
			SPI_Write(
					host->hal_handle,Transfer_Array,
					sizeof(Transfer_Array),
					&SizeTransfered,
					SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
				);
		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)			
            switch(host->spichannel)
			{
				case FT_GPU_SPI_SINGLE_CHANNEL:
                    /* FYI : All HOST CMDs should only be executed in single channel mode
                    */
					status = FT4222_SPIMaster_SingleWrite(
															host->hal_handle,
															Transfer_Array,
															sizeof(Transfer_Array),
															(ft_uint16_t *)&SizeTransfered,
															TRUE
														 );
					if (FT4222_OK != status)
						printf("SPI write failed = %d\n",status);
				break;
				case FT_GPU_SPI_DUAL_CHANNEL:
				case FT_GPU_SPI_QUAD_CHANNEL:
					/* only reset command among host commands can be executed in multi channel mode*/					
					status = FT4222_SPIMaster_MultiReadWrite(
						                                        host->hal_handle,
						                                        &dummy_read,
						                                        Transfer_Array,
						                                        0,
                                                                sizeof(Transfer_Array),
						                                        0,
                                                                &SizeTransfered
						                                    );
					if (FT4222_OK != status)
						printf("SPI write failed = %d\n", status);
				break;
				default:
					printf("No transfer\n");
			}			
		#else
			#error "LIB configuration missing "
		#endif
	#endif

	#ifdef ARDUINO_PLATFORM_SPI
	  digitalWrite(host->hal_config.spi_cs_pin_no, LOW);
	  SPI.transfer(cmd);
	  SPI.transfer(0);
	  SPI.transfer(0);
	  digitalWrite(host->hal_config.spi_cs_pin_no, HIGH);
	#endif

	#ifdef MSVC_FT800EMU
	  //Not implemented in FT800EMU
	#endif
}

/***************************************************************************
* Interface Description    : API to select clock source
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_ClockSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_SOURCE_T pllsource)
{
   Ft_Gpu_HostCommand(host,pllsource);
}

/***************************************************************************
* Interface Description    : API to select frequency
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_PLL_FreqSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_FREQ_T freq)
{
   Ft_Gpu_HostCommand(host,freq);
}

/***************************************************************************
* Interface Description    : 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_PowerModeSwitch(Ft_Gpu_Hal_Context_t *host,FT_GPU_POWER_MODE_T pwrmode)
{
   Ft_Gpu_HostCommand(host,pwrmode);
}

/***************************************************************************
* Interface Description    : 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_CoreReset(Ft_Gpu_Hal_Context_t *host)
{
   Ft_Gpu_HostCommand(host,FT_GPU_CORE_RESET);
}

/***************************************************************************
* Interface Description    : This API can only be called when PLL is stopped
*                            (SLEEP mode).For compatibility, set frequency to
*                            the FT_GPU_12MHZ option in the FT_GPU_SETPLLSP1_T
*                            table.
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_81X_SelectSysCLK(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_PLL_FREQ_T freq)
{
		if(FT_GPU_SYSCLK_72M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x06 << 8)); 
		else if(FT_GPU_SYSCLK_60M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x05 << 8)); 
		else if(FT_GPU_SYSCLK_48M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x04 << 8)); 
		else if(FT_GPU_SYSCLK_36M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x03 << 8)); 
		else if(FT_GPU_SYSCLK_24M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x02 << 8)); 
		else if(FT_GPU_SYSCLK_DEFAULT == freq)//default clock
			Ft_Gpu_HostCommand_Ext3(host, 0x61); 
}

/***************************************************************************
* Interface Description    : Power down or up ROMs and ADCs.  Specified one 
*                            or more elements in the FT_GPU_81X_ROM_AND_ADC_T
*                            table to power down, unspecified elements will be
*                            powered up.  The application must retain the state
*                            of the ROMs and ADCs as they're not readable from
*                            the device.
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_GPU_81X_PowerOffComponents(Ft_Gpu_Hal_Context_t *host, ft_uint8_t val)
{
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x49 | (val<<8));
}

/***************************************************************************
* Interface Description    : this API sets the current strength of supported 
*                            GPIO/IO group(s)
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_GPU_81X_PadDriveStrength(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_GPIO_DRIVE_STRENGTH_T strength, FT_GPU_81X_GPIO_GROUP_T group)
{
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x70 | (group << 8) | (strength << 8));
}

/***************************************************************************
* Interface Description    : this API will hold the system reset active,
*                            Ft_Gpu_81X_ResetRemoval() must be called to
*                            release the system reset.
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_81X_ResetActive(Ft_Gpu_Hal_Context_t *host)
{
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_ACTIVE); 
}

/***************************************************************************
* Interface Description    : This API will release the system reset, and the 
*                            system will exit reset and behave as after POR,
*                            settings done through SPI commands will not be
*                            affected.
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_81X_ResetRemoval(Ft_Gpu_Hal_Context_t *host)
{
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_REMOVAL); 
}
#endif

/***************************************************************************
* Interface Description    : This API sends a 3byte command from host to EVE
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_HostCommand_Ext3(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
	FT4222_STATUS status;
    ft_uint8_t dummy_read;
	#endif

	#ifdef FT900_PLATFORM
		ft_uint8_t hcmd[4] = {0};
		hcmd[0] = cmd & 0xff;
		hcmd[1] = (cmd>>8) & 0xff;
		hcmd[2] = (cmd>>16) & 0xff;
		hcmd[3] = 0;
	spi_open(SPIM,host->hal_config.spi_cs_pin_no);
	spi_writen(SPIM,hcmd,3);
	spi_close(SPIM,host->hal_config.spi_cs_pin_no);
		
	#endif

	#ifdef MSVC_PLATFORM
		ft_uint8_t Transfer_Array[3];
		ft_uint32_t SizeTransfered;
	
		Transfer_Array[0] = cmd;
		Transfer_Array[1] = (cmd>>8) & 0xff;
		Transfer_Array[2] = (cmd>>16) & 0xff;
		
		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
			SPI_Write(
						host->hal_handle,
						Transfer_Array,
						sizeof(Transfer_Array),
						&SizeTransfered,
						SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
					);
		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)			
			switch (host->spichannel)
			{
			  case FT_GPU_SPI_SINGLE_CHANNEL:
				  status = FT4222_SPIMaster_SingleWrite(
					                                      host->hal_handle,
					                                      Transfer_Array,
					                                      sizeof(Transfer_Array),
					                                      (ft_uint16_t *) &SizeTransfered,
					                                      TRUE
					                                   );
				  if (FT4222_OK != status)
					  printf("SPI write failed = %d\n", status);
				  break;
			  case FT_GPU_SPI_DUAL_CHANNEL:
			  case FT_GPU_SPI_QUAD_CHANNEL:
				  /* FYI : Mostly all HOST CMDs can be executed in single channel mode
				   * except system reset cmd */				  
				  status = FT4222_SPIMaster_MultiReadWrite(
					                                          host->hal_handle,
					                                          &dummy_read,
					                                          Transfer_Array,
					                                          0,
                                                              sizeof(Transfer_Array),
					                                          0,
                                                              &SizeTransfered
					                                      );
				  if (FT4222_OK != status)
					  printf("SPI write failed = %d\n", status);
				  break;
			  default:
				  printf("No transfer\n");
			 }
		#else
			#error "LIB configuration missing "
		#endif		  
	#endif
	#ifdef ARDUINO_PLATFORM_SPI
	  digitalWrite(host->hal_config.spi_cs_pin_no, LOW);
	  SPI.transfer(cmd);
	  SPI.transfer((cmd>>8) & 0xff);
	  SPI.transfer((cmd>>16) & 0xff);
	  digitalWrite(host->hal_config.spi_cs_pin_no, HIGH);
	#endif
	#ifdef MSVC_FT800EMU
	  //Not implemented in FT800EMU
	#endif
}

/***************************************************************************
* Interface Description    : Function to update global HAL context variable 
*							 ft_cmd_fifo_wp pointer and write to REG_CMD_WRITE
*							 to indicate GPU to start processing new commands 
*							 in RAM_CMD
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
	host->ft_cmd_fifo_wp  = (host->ft_cmd_fifo_wp + count) & 4095;

	//4 byte alignment
	host->ft_cmd_fifo_wp = (host->ft_cmd_fifo_wp + 3) & 0xffc;
	Ft_Gpu_Hal_Wr16(host,REG_CMD_WRITE,host->ft_cmd_fifo_wp);
}

/***************************************************************************
* Interface Description    : Function to compute available freespace in RAM_CMD.
*							 RAM_CMD is 4K in size.
*							 REG_CMD_READ reg provides command buffer read pointer
* Implementation           :
* Return Value             : ft_uint16_t
* Author                   :
****************************************************************************/
ft_uint16_t Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host)
{
	ft_uint16_t fullness,retval;

	//host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);

	fullness = (host->ft_cmd_fifo_wp - Ft_Gpu_Hal_Rd16(host,REG_CMD_READ)) & 4095;
	retval = (FT_CMD_FIFO_SIZE - 4) - fullness;
	return (retval);
}

/***************************************************************************
* Interface Description    : Continuous write to RAM_CMD with wait with start
*                            address as host->ft_cmd_fifo_wp + RAM_CMD.
*                            FT81x RAM_CMD size is 4K (4096 bytes)
*                            Hence one SPI write is adequate.
* Implementation           :
* Return Value             : ft_uint16_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0,availablefreesize;
    
    #if defined (MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)  
        FT4222_STATUS status;
        ft_uint8_t * wrpktptr;
        ft_uint8_t dummy_read;
        wrpktptr = host->spiwrbuf_ptr;  //Using global buf , FT4222_DYNAMIC_ALLOCATE_SIZE
    #endif

	do 
	{                
		length = count;
        availablefreesize = Ft_Gpu_Cmdfifo_Freespace(host);

		if (length > availablefreesize)
		{
		    length = availablefreesize;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

        #if defined (MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)  
        *(wrpktptr + 0) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 16) | 0x80;
        *(wrpktptr + 1) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 8);
        *(wrpktptr + 2) = (ft_uint8_t) (host->ft_cmd_fifo_wp + RAM_CMD) & 0xff;
        memcpy((wrpktptr + 3), buffer, length);

        if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
        {
            status = FT4222_SPIMaster_SingleWrite(
                host->hal_handle,
                wrpktptr,
                (length + 3), //3 for RAM_CMD address
                (ft_uint16_t *) &SizeTransfered,
                TRUE
                );
            if ((FT4222_OK != status) || (SizeTransfered != (length + 3)))
            {
                printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, SizeTransfered, status);
                break;
            }
        }
        else
        {	/* DUAL and QAUD */
            status = FT4222_SPIMaster_MultiReadWrite(
                host->hal_handle,
                &dummy_read,
                wrpktptr,
                0,
                (length + 3),
                0,
                &SizeTransfered
                );
        }
        buffer += length;
        #else
		    Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length); 
		    #ifdef FT900_PLATFORM
			    spi_writen(SPIM,buffer,length);
			    buffer += length;
		    #endif
		    #if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
			    SizeTransfered = 0;
			    while (length--) {
			    Ft_Gpu_Hal_Transfer8(host,*buffer);
			    buffer++;
			    SizeTransfered ++;
			    }
			    length = SizeTransfered;
		    #endif
		    #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)			
			    {   
				    SPI_Write(host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
				    length = SizeTransfered;
   				    buffer += SizeTransfered;
			    }		
		    #endif
		Ft_Gpu_Hal_EndTransfer(host);
        #endif
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

#if defined (ARDUINO_PLATFORM_SPI) || defined (FT900_PLATFORM)
ft_void_t Ft_Gpu_Hal_WrCmdBufFromFlash(Ft_Gpu_Hal_Context_t *host,FT_PROGMEM ft_prog_uchar8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);


                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;

    	        Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}
#endif

/***************************************************************************
* Interface Description    : Blocking function call
*							 Blocks until "count" number of bytes gets available
*							 in RAM_CMD
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
   ft_uint16_t getfreespace;
   do{
        getfreespace = Ft_Gpu_Cmdfifo_Freespace(host);
   }while(getfreespace < count);
}

/***************************************************************************
* Interface Description    : Blocking function call
*							 Blocks until all commands in RAM_CMD are executed and 
*							 it is fully empty
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host)
{
   while(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));
   
   host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
}

/***************************************************************************
* Interface Description    : Continuous write to RAM_CMD with no wait
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WrCmdBuf_nowait(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0 , availablefreesize;
    #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
        FT4222_STATUS status;
        ft_uint8_t * wrpktptr;
        ft_uint8_t dummy_read;
        wrpktptr = host->spiwrbuf_ptr; //Using global buf , FT4222_DYNAMIC_ALLOCATE_SIZE  
    #endif

	do {                
			length = count;
            availablefreesize = Ft_Gpu_Cmdfifo_Freespace(host);

			if (length > availablefreesize)
			{
				length = availablefreesize;
			}
      	    Ft_Gpu_Hal_CheckCmdBuffer(host,length);

            #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
                *(wrpktptr + 0) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 16);
                *(wrpktptr + 1) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 8);
                *(wrpktptr + 2) = (ft_uint8_t) (host->ft_cmd_fifo_wp + RAM_CMD) & 0xff;
                memcpy((wrpktptr + 3), buffer, length);

                if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
                {
                    status = FT4222_SPIMaster_SingleWrite(
                        host->hal_handle,
                        wrpktptr,
                        length,
                        (ft_uint16_t *) &SizeTransfered,
                        TRUE
                        );
                    if ((FT4222_OK != status) || (SizeTransfered != length))
                    {
                        printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, SizeTransfered, status);
                        break;
                    }
                }
                else
                {	/* DUAL and QAUD */
                    status = FT4222_SPIMaster_MultiReadWrite(
                        host->hal_handle,
                        &dummy_read,
                        wrpktptr,
                        0,
                        length,
                        0,
                        0
                        );
                 }                
                buffer += length;
            #else
            Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);
			#ifdef FT900_PLATFORM
				spi_writen(SPIM,buffer,length);
				buffer += length;
			#endif
			//#ifdef ARDUINO_PLATFORM_SPI
			#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
				SizeTransfered = 0;
				while (length--)
				{
                    Ft_Gpu_Hal_Transfer8(host,*buffer);
		            buffer++;
                    SizeTransfered ++;
		        }
                length = SizeTransfered;
            #endif

            #if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBMPSSE)	
			{   
				SPI_Write(host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
                length = SizeTransfered;
   				buffer += SizeTransfered;
			}
			#endif

			Ft_Gpu_Hal_EndTransfer(host);
            #endif  
			Ft_Gpu_Hal_Updatecmdfifo(host,length);

	//	Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

/***************************************************************************
* Interface Description    : 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint8_t Ft_Gpu_Hal_WaitCmdfifo_empty_status(Ft_Gpu_Hal_Context_t *host)
{
   if(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE))
   {
     return 0;
   }
   else
   {
     host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
     return 1;
   }  
}

/***************************************************************************
* Interface Description    :
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WaitLogo_Finish(Ft_Gpu_Hal_Context_t *host)
{
    ft_int16_t cmdrdptr,cmdwrptr;

    do{
         cmdrdptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_READ);
         cmdwrptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
    }while ((cmdwrptr != cmdrdptr) || (cmdrdptr != 0));
    host->ft_cmd_fifo_wp = 0;
}

/***************************************************************************
* Interface Description    :
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_ResetCmdFifo(Ft_Gpu_Hal_Context_t *host)
{
   host->ft_cmd_fifo_wp = 0;
}

/***************************************************************************
* Interface Description    :
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WrCmd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
         Ft_Gpu_Hal_CheckCmdBuffer(host,sizeof(cmd));
      
         Ft_Gpu_Hal_Wr32(host,RAM_CMD + host->ft_cmd_fifo_wp,cmd);
      
         Ft_Gpu_Hal_Updatecmdfifo(host,sizeof(cmd));
}

/***************************************************************************
* Interface Description    :
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_ResetDLBuffer(Ft_Gpu_Hal_Context_t *host)
{
           host->ft_dl_buff_wp = 0;
}


/***************************************************************************
* Interface Description    : Toggle PD_N pin of FT800 board for a power cycle
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Powercycle(Ft_Gpu_Hal_Context_t *host, ft_bool_t up)
{

	if (up)
	{
		#ifdef MSVC_PLATFORM
			#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
				//FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
				FT_WriteGPIO(host->hal_handle, (1 << host->hal_config.pdn_pin_no) | 0x3B, (0<<host->hal_config.pdn_pin_no)|0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
			
            Ft_Gpu_Hal_Sleep(20);

				//FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
				FT_WriteGPIO(host->hal_handle, (1 << host->hal_config.pdn_pin_no) | 0x3B, (1<<host->hal_config.pdn_pin_no)|0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
				Ft_Gpu_Hal_Sleep(20);
			#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)
                FT4222_STATUS status = FT4222_OTHER_ERROR;

				if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, host->hal_config.pdn_pin_no, 0)))
					printf("FT4222_GPIO_Write error = %d\n",status);
				Ft_Gpu_Hal_Sleep(20);
			

				if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, host->hal_config.pdn_pin_no, 1)))
					printf("FT4222_GPIO_Write error = %d\n", status);
				Ft_Gpu_Hal_Sleep(20);
			#else
				#error "LIB configuration missing for MSVC platform"
			#endif
		#endif
		#ifdef ARDUINO_PLATFORM      
			digitalWrite(host->hal_config.pdn_pin_no, LOW);
			Ft_Gpu_Hal_Sleep(20);

			digitalWrite(host->hal_config.pdn_pin_no, HIGH);
			Ft_Gpu_Hal_Sleep(20);
		#endif
		#ifdef FT900_PLATFORM
			gpio_write(host->hal_config.pdn_pin_no, 0);
			ft_delay(20);
			gpio_write(host->hal_config.pdn_pin_no, 1);
			ft_delay(20);
		#endif
	}
	else
	{
		#ifdef MSVC_PLATFORM
			#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
				//FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
				FT_WriteGPIO(host->hal_handle, (1 << host->hal_config.pdn_pin_no) | 0x3B, (1<<host->hal_config.pdn_pin_no)|0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
				Ft_Gpu_Hal_Sleep(20);
            
            //FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
	        FT_WriteGPIO(host->hal_handle, (1 << host->hal_config.pdn_pin_no) | 0x3B, (0<<host->hal_config.pdn_pin_no)|0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
			
				Ft_Gpu_Hal_Sleep(20);
			#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)
                FT4222_STATUS status = FT4222_OTHER_ERROR;

				if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, host->hal_config.pdn_pin_no, 1)))
					printf("FT4222_GPIO_Write error = %d\n", status);
				Ft_Gpu_Hal_Sleep(20);				

				if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, host->hal_config.pdn_pin_no, 0)))
					printf("FT4222_GPIO_Write error = %d\n", status);
				Ft_Gpu_Hal_Sleep(20);				
			#else
				#error "LIB configuration missing for MSVC platform"
			#endif
		#endif
		#ifdef ARDUINO_PLATFORM
			digitalWrite(host->hal_config.pdn_pin_no, HIGH);
			Ft_Gpu_Hal_Sleep(20);
            
            digitalWrite(host->hal_config.pdn_pin_no, LOW);
            Ft_Gpu_Hal_Sleep(20);
#endif
#ifdef FT900_PLATFORM
            gpio_write(host->hal_config.pdn_pin_no, 1);
            ft_delay(20);
            gpio_write(host->hal_config.pdn_pin_no, 0);
            ft_delay(20);
#endif

	}
}

/***************************************************************************
* Interface Description    : Ft_Gpu_Hal_WrMemFromFlash and Ft_Gpu_Hal_WrMem ideally
*                            perform same operation.Find why was 2 created?
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WrMemFromFlash(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_prog_uchar8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU) || defined(FT900_PLATFORM)
    Ft_Gpu_Hal_StartTransfer(host, FT_GPU_WRITE, addr);
	while (length--) {
            Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
	    buffer++;
	}
    Ft_Gpu_Hal_EndTransfer(host);
#endif

	#ifdef MSVC_PLATFORM
		#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)	
            Ft_Gpu_Hal_StartTransfer(host, FT_GPU_WRITE, addr);
			SPI_Write((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);		
            Ft_Gpu_Hal_EndTransfer(host);
		#elif defined(MSVC_PLATFORM_SPI_LIBFT4222)
			Ft_Gpu_Hal_FT4222_Wr(host, addr, buffer, length);
		#endif
	#endif	
}

/***************************************************************************
* Interface Description    : 
*                            
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_WrMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_uint8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
		if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, buffer, length))
		{
			printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
		}
	#else
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
		#ifdef FT900_PLATFORM
			spi_writen(SPIM,buffer,length);
		#endif

		#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
			while (length--)
			{
				Ft_Gpu_Hal_Transfer8(host,*buffer);
				buffer++;
			}
		#endif

		#ifdef MSVC_PLATFORM
			{
				SPI_Write((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
			}
		#endif
        Ft_Gpu_Hal_EndTransfer(host);
	#endif	
}

/***************************************************************************
* Interface Description    : 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_RdMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

	#if defined(MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
        if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, buffer, length))
        {
            printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
        }
	#else
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
		#ifdef FT900_PLATFORM
			unsigned char spiData[2] = {0};
			spi_readn(SPIM,spiData,host->spinumdummy);
			spi_readn(SPIM,buffer,length);
		#endif
		#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
			while (length--) {
			   *buffer = Ft_Gpu_Hal_Transfer8(host,0);
			   buffer++;
			}
		#endif

		#ifdef MSVC_PLATFORM
			{
			   SPI_Read((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
			}
		#endif
        Ft_Gpu_Hal_EndTransfer(host);
	#endif	
}

/***************************************************************************
* Interface Description    : Helper api for dec to ascii
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_int32_t Ft_Gpu_Hal_Dec2Ascii(ft_char8_t *pSrc,ft_int32_t value)
{
	ft_int16_t Length;
	ft_char8_t *pdst,charval;
	ft_int32_t CurrVal = value,tmpval,i;
	ft_char8_t tmparray[16],idx = 0;

	Length = strlen(pSrc);
	pdst = pSrc + Length;

	if(0 == value)
	{
		*pdst++ = '0';
		*pdst++ = '\0';
		return 0;
	}

	if(CurrVal < 0)
	{
		*pdst++ = '-';
		CurrVal = - CurrVal;
	}
	/* insert the value */
	while(CurrVal > 0){
		tmpval = CurrVal;
		CurrVal /= 10;
		tmpval = tmpval - CurrVal*10;
		charval = '0' + tmpval;
		tmparray[idx++] = charval;
	}

	for(i=0;i<idx;i++)
	{
		*pdst++ = tmparray[idx - i - 1];
	}
	*pdst++ = '\0';

	return 0;
}

/***************************************************************************
* Interface Description    : Calls platform specific sleep call
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Gpu_Hal_Sleep(ft_uint32_t ms)
{
#ifdef FT900_PLATFORM
	delayms(ms);
#endif
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
	Sleep(ms);
#endif
#ifdef ARDUINO_PLATFORM
	delay(ms);
#endif
}

/***************************************************************************
* Interface Description    : Set EVE spi communication mode
*                            Set USB bridge communication mode
*                            Update global variable
* Implementation           :
* Return Value             : ft_void_t
*                            -1 - Error, 0 - Success
* Author                   :
****************************************************************************/
#ifdef FT_81X_ENABLE
ft_int16_t Ft_Gpu_Hal_SetSPI(Ft_Gpu_Hal_Context_t *host,FT_GPU_SPI_NUMCHANNELS_T numchnls,FT_GPU_SPI_NUMDUMMYBYTES numdummy)
{
	ft_uint8_t writebyte = 0;
    #if defined (MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
    FT4222_STATUS ftstatus;
    FT4222_SPIMode spimode;
    #endif
	
	if((numchnls > FT_GPU_SPI_QUAD_CHANNEL) || (numdummy > FT_GPU_SPI_TWODUMMY) || (numdummy < FT_GPU_SPI_ONEDUMMY))
	{
		return -1;//error
	}

    //swicth EVE to multi channel SPI mode
    writebyte = numchnls;
    if(numdummy == FT_GPU_SPI_TWODUMMY)
        writebyte |= FT_SPI_TWO_DUMMY_BYTE;
    Ft_Gpu_Hal_Wr8(host, REG_SPI_WIDTH, writebyte);

    #if defined (MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
    //swicth FT4222 to relevant multi channel SPI communication mode
    if (numchnls == FT_GPU_SPI_DUAL_CHANNEL)
        spimode = SPI_IO_DUAL;
    else if (numchnls == FT_GPU_SPI_QUAD_CHANNEL)
        spimode = SPI_IO_QUAD;
    else
        spimode = SPI_IO_SINGLE;

    ftstatus = FT4222_SPIMaster_SetLines(host->hal_handle, spimode);
    if (FT4222_OK != ftstatus)
        printf("FT4222_SPIMaster_SetLines failed with status %d\n", ftstatus);
    #endif
        
    //FT81x swicthed to dual/quad mode, now update global HAL context 
    host->spichannel = numchnls;
    host->spinumdummy = numdummy;
 
	return 0;
}
#endif

#ifdef FT900_PLATFORM
/* Helper api for millis */
/* api to return the time in ms. 0 after reset */

/* Globals for polling implementation */
ft_uint32_t ft_millis_curr = 0,ft_millis_prev = 0;

/* Globals for interrupt implementation */
ft_uint32_t ft_TotalMilliseconds = 0;

ft_void_t ft_millis_ticker()
{
	timer_disable_interrupt(FT900_FT_MILLIS_TIMER);
	/* Clear the interrupt and increment the counter */
	timer_is_interrupted(FT900_FT_MILLIS_TIMER);

	ft_TotalMilliseconds += 1;
	timer_enable_interrupt(FT900_FT_MILLIS_TIMER);
}
ft_void_t ft_millis_init()
{
#ifdef FT900_PLATFORM
	ft_millis_curr = 0;
	ft_millis_prev = 0;

	sys_enable(sys_device_timer_wdt);
	timer_prescaler(FT900_TIMER_PRESCALE_VALUE);
	timer_init(FT900_FT_MILLIS_TIMER,FT900_TIMER_OVERFLOW_VALUE,timer_direction_up,timer_prescaler_select_on,timer_mode_continuous);

	interrupt_attach(interrupt_timers, 17, ft_millis_ticker);


	/* enabling the interrupts for timer */
	timer_enable_interrupt(FT900_FT_MILLIS_TIMER);

	timer_start(FT900_FT_MILLIS_TIMER);
#endif
}
/* Need to ensure that below api is called at least once in 6.5 seconds duration for FT900 platform as this module doesnt use timer for context update */
/* global counter to loopback after ~49.71 days */
ft_uint32_t ft_millis()
{
#if defined(ARDUINO_PLATFORM) ||defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
	return millis();
#endif
#ifdef FT900_PLATFORM

	/* Polling implementation */
#if 0
	ft_uint32_t currtime;
	currtime = ft900_timer_get_value(FT900_FT_MILLIS_TIMER);

	if(ft_millis_prev > currtime)
	{
		/* loop back condition */
		ft_millis_curr += ((FT900_TIMER_MAX_VALUE - ft_millis_prev + currtime)/10);
	}
	else
	{
		ft_millis_curr += ((currtime - ft_millis_prev)/10);
	}
	ft_millis_prev = currtime;
	//printf("current time %d \n",ft_millis_curr);
	return ft_millis_curr;
#endif

	/* Interrupt implementation */
	return (ft_TotalMilliseconds);
#endif
}

ft_void_t ft_millis_exit()
{
#ifdef FT900_PLATFORM
	timer_stop(FT900_FT_MILLIS_TIMER);
	timer_disable_interrupt(FT900_FT_MILLIS_TIMER);
#endif
}
#endif

/***************************************************************************
* Interface Description    : FIFO related apis
*                            Init all the parameters of fifo buffer
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Fifo_Init(Ft_Fifo_t *pFifo,ft_uint32_t StartAddress,ft_uint32_t Length,ft_uint32_t HWReadRegAddress,ft_uint32_t HWWriteRegAddress)
{
	/* update the context parameters */
	pFifo->fifo_buff = StartAddress;
	pFifo->fifo_len = Length;
	pFifo->fifo_rp = pFifo->fifo_wp = 0;

	/* update the hardware register addresses - specific to FT800 series chips */
	pFifo->HW_Read_Reg = HWReadRegAddress;
	pFifo->HW_Write_Reg = HWWriteRegAddress;
}

/***************************************************************************
* Interface Description    : FIFO related apis
*                            update both the read and write pointers
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Fifo_Update(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{
	pFifo->fifo_rp = Ft_Gpu_Hal_Rd32(host,pFifo->HW_Read_Reg);
	//Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);
}

/***************************************************************************
* Interface Description    : FIFO related apis
*                            just write and update the write register
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint32_t Ft_Fifo_Write(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t NumbytetoWrite)
{
	ft_uint32_t FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo),TotalBytes = NumbytetoWrite;

	if(NumbytetoWrite > FreeSpace)
	{
		/* update the read pointer and get the free space */
		Ft_Fifo_Update(host,pFifo);
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);

		if(NumbytetoWrite > FreeSpace)
		{
			TotalBytes = FreeSpace;
		}
	}

	/* sanity check */
	if(TotalBytes <= 0)
	{
		//printf("no space in fifo write %d %d %d %d\n",TotalBytes,FreeSpace,pFifo->fifo_wp,pFifo->fifo_rp);
		return 0;//error condition
	}
	/* check for the loopback conditions */
	if(pFifo->fifo_wp + TotalBytes >= pFifo->fifo_len)
	{
		ft_uint32_t partialchunk = pFifo->fifo_len - pFifo->fifo_wp,secpartialchunk = TotalBytes - partialchunk;

		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,partialchunk);
		if(secpartialchunk > 0)
		{
			Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff,buffer + partialchunk,secpartialchunk);
		}
		pFifo->fifo_wp = secpartialchunk;
		//printf("partial chunks %d %d %d %d\n",partialchunk,secpartialchunk,pFifo->fifo_wp,pFifo->fifo_rp);

	}
	else
	{
		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,TotalBytes);
		pFifo->fifo_wp += TotalBytes;
	}

	/* update the write pointer address in write register */
	Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);

	return TotalBytes;
}

/***************************************************************************
* Interface Description    : FIFO related apis
*                            just write one word and update the write register
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Fifo_Write32(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint32_t WriteWord)
{
	Ft_Fifo_Write(host,pFifo,(ft_uint8_t *)&WriteWord,4);
}

/***************************************************************************
* Interface Description    : FIFO related apis
*                            write and wait for the fifo to be empty. handle cases even if
*                            the Numbytes are more than freespace
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_void_t Ft_Fifo_WriteWait(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t Numbyte)
{
	ft_uint32_t TotalBytes = Numbyte,currchunk = 0,FreeSpace;
	ft_uint8_t *pbuff = buffer;
	/* blocking call, manage to check for the error case and break in case of error */
	while(TotalBytes > 0)
	{
		currchunk = TotalBytes;
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);
		if(currchunk > FreeSpace)
		{
			currchunk = FreeSpace;
		}

		Ft_Fifo_Write(host,pFifo,pbuff,currchunk);
		pbuff += currchunk;
		TotalBytes -= currchunk;


	}
}

#if defined(FT900_PLATFORM)
ft_void_t getFlashTextString(char __flash__ *str, ft_uchar8_t *destArray, ft_uint16_t numOfChars){
		ft_uint16_t i;
		for(i=0;i<numOfChars;i++)
			destArray[i] = str[i];
}
#endif

/***************************************************************************
* Interface Description    : FIFO related apis
*                            get the free space in the fifo - make sure the 
*                            return value is maximum of (LENGTH - 4)
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint32_t Ft_Fifo_GetFreeSpace(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{
	ft_uint32_t FreeSpace = 0;

	Ft_Fifo_Update(host,pFifo);

	if(pFifo->fifo_wp >= pFifo->fifo_rp)
	{
		FreeSpace = pFifo->fifo_len - pFifo->fifo_wp + pFifo->fifo_rp;
	}
	else
	{
		FreeSpace = pFifo->fifo_rp - pFifo->fifo_wp;
	}

	if(FreeSpace >= 4)
	{
		FreeSpace -= 4;//make sure 1 word space is maintained between rd and wr pointers
	}
	return FreeSpace;
}

/***************************************************************************
* Interface Description    : 
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_uint32_t Ft_Gpu_CurrentFrequency(Ft_Gpu_Hal_Context_t *host)
{
    ft_uint32_t t0, t1;
    ft_uint32_t addr = REG_CLOCK;
    ft_uint8_t spidata[4];
	ft_int32_t r = 15625;

    t0 = Ft_Gpu_Hal_Rd32(host,REG_CLOCK); /* t0 read */
               
#ifdef FT900_PLATFORM
    __asm__
    (
                    "   move.l  $r0,%0"             "\n\t"
                    "   mul.l   $r0,$r0,100"                                                  "\n\t"
                    "1:"               "\n\t"
                    "   sub.l   $r0,$r0,3"          "\n\t" /* Subtract the loop time = 4 cycles */
                    "   cmp.l   $r0,0"              "\n\t" /* Check that the counter is equal to 0 */
                    "   jmpc    gt, 1b"  "\n\t"
                    /* Outputs */ :
                    /* Inputs */  : "r"(r)
                    /* Using */   : "$r0"

    );

    //usleep(15625);
	//ft_delay(15625);
#endif
#if (defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU))
	//may not be precise
	Sleep(15625/1000);
#endif
#ifdef ARDUINO_PLATFORM
	delayMicroseconds(15625);
#endif

    t1 = Ft_Gpu_Hal_Rd32(host,REG_CLOCK); /* t1 read */
    return ((t1 - t0) * 64); /* bitshift 6 places is the same as multiplying 64 */
}

/***************************************************************************
* Interface Description    :
* Implementation           :
* Return Value             : ft_void_t
* Author                   :
****************************************************************************/
ft_int32_t Ft_Gpu_ClockTrimming(Ft_Gpu_Hal_Context_t *host,ft_int32_t LowFreq)
{
   ft_uint32_t f;
   ft_uint8_t i;

  /* Trim the internal clock by increase the REG_TRIM register till the measured frequency is within the acceptable range.*/
   for (i=0; (i < 31) && ((f= Ft_Gpu_CurrentFrequency(host)) < LowFreq); i++)
   {
	   Ft_Gpu_Hal_Wr8(host,REG_TRIM, i);  /* increase the REG_TRIM register value automatically increases the internal clock */

   }

   Ft_Gpu_Hal_Wr32(host,REG_FREQUENCY,f);  /* Set the final frequency to be used for internal operations */

   return f;
}

#if defined (MSVC_PLATFORM) && defined(MSVC_PLATFORM_SPI_LIBFT4222)
/***************************************************************************
* Interface Description    : Function to tranfer HOST MEMORY READ command followed 
*							 by read of data bytes from GPU
* Implementation           : Using FT4222_SPIMaster_SingleRead,
*							 FT4222_SPIMaster_SingleWrite
*							 FT4222_SPIMaster_MultiReadWrite
*							 
* Return Value             : ft_uint8_t
*							 1 - Success
*							 0 - Failure
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_FT4222_Rd(Ft_Gpu_Hal_Context_t *host, ft_uint32_t hrdcmd, ft_uint8_t * rdbufptr, ft_uint32_t exprdbytes)
{
    ft_uint32_t SizeTransfered;    
	FT4222_STATUS status;
	ft_uint8_t hrdpkt[8] = {0,0,0,0,0,0,0,0}; //3 byte addr + 2 or1 byte dummy
	ft_uint8_t retcode = 1;		/* assume successful operation */	
    ft_uint16_t bytes_per_read;    

    if (rdbufptr == NULL || exprdbytes == 0)
        retcode = 0; //fail                

    if (retcode)
    {
        if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
        {
            /* Compose the HOST MEMORY READ packet */
            hrdpkt[0] = (ft_uint8_t)(hrdcmd >> 16) & 0xFF;
            hrdpkt[1] = (ft_uint8_t)(hrdcmd >> 8) & 0xFF;
            hrdpkt[2] = (ft_uint8_t)(hrdcmd & 0xFF);

            status = FT4222_SPIMaster_SingleWrite(
                host->hal_handle,
                hrdpkt,
                3 + host->spinumdummy, /* 3 address and chosen dummy bytes */
                (ft_uint16_t *)&SizeTransfered,
                FALSE			/* continue transaction */
            );
            if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != (3 + host->spinumdummy)))
            {
                printf("FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", (ft_uint16_t)SizeTransfered, status);
                retcode = 0;
                if ((ft_uint16_t)SizeTransfered != (3 + host->spinumdummy))
                    host->status = FT_GPU_HAL_STATUS_ERROR;
            }
            else
            {
                /* continue reading data bytes only if HOST MEMORY READ command sent successfully */
                if (rdbufptr != NULL)
                {
                    BOOL disable_cs = FALSE; //assume multi SPI read calls				
                    bytes_per_read = exprdbytes;

                    while (retcode && exprdbytes)
                    {
                        if (exprdbytes <= FT4222_MAX_RD_BYTES_PER_CALL_IN_SINGLE_CH)
                        {
                            bytes_per_read = exprdbytes;
                            disable_cs = TRUE; //1 iteration of SPI read adequate
                        }
                        else
                        {
                            bytes_per_read = FT4222_MAX_RD_BYTES_PER_CALL_IN_SINGLE_CH;
                            disable_cs = FALSE;
                        }

                        status = FT4222_SPIMaster_SingleRead(
                            host->hal_handle,
                            rdbufptr,
                            bytes_per_read,
                            (ft_uint16_t *)&SizeTransfered,
                            disable_cs
                        );
                        if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != bytes_per_read))
                        {
                            printf("FT4222_SPIMaster_SingleRead failed,SizeTransfered is %d with status %d\n", (ft_uint16_t)SizeTransfered, status);
                            retcode = 0;
                            if ((ft_uint16_t)SizeTransfered != bytes_per_read)
                                host->status = FT_GPU_HAL_STATUS_ERROR;
                        }

                        //multiple iterations of SPI read needed
                        bytes_per_read = (ft_uint16_t)SizeTransfered;

                        exprdbytes -= bytes_per_read;
                        rdbufptr += bytes_per_read;

                    }
                }
            }
        }
        else
        {
            /* Multi channel SPI communication */
            ft_uint32_t t_hrdcmd = hrdcmd;
            ft_uint32_t read_data_index = 0;

            while (retcode && exprdbytes)
            {
                /* Compose the HOST MEMORY READ ADDR packet */
                hrdpkt[0] = (ft_uint8_t)(t_hrdcmd >> 16) & 0xFF;
                hrdpkt[1] = (ft_uint8_t)(t_hrdcmd >> 8) & 0xFF;
                hrdpkt[2] = (ft_uint8_t)(t_hrdcmd & 0xff);

                if (exprdbytes <= FT4222_MAX_RD_BYTES_PER_CALL_IN_MULTI_CH)
                    bytes_per_read = exprdbytes;
                else
                    bytes_per_read = FT4222_MAX_RD_BYTES_PER_CALL_IN_MULTI_CH;

                status = FT4222_SPIMaster_MultiReadWrite(
                    host->hal_handle,
                    rdbufptr + read_data_index,
                    hrdpkt,
                    0,
                    3 + host->spinumdummy, // 3 addr + dummy bytes
                    bytes_per_read,
                    &SizeTransfered
                );
                if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != bytes_per_read))
                {
                    printf("FT4222_SPIMaster_MultiReadWrite failed, SizeTransfered is %d with status %d\n", SizeTransfered, status);
                    retcode = 0;
                    if ((ft_uint16_t)SizeTransfered != bytes_per_read)
                        host->status = FT_GPU_HAL_STATUS_ERROR;
                }

                //its multi SPI read calls
                bytes_per_read = (ft_uint16_t)SizeTransfered;

                exprdbytes -= bytes_per_read;
                read_data_index += bytes_per_read;
                t_hrdcmd += bytes_per_read;
            }
        }
    }
	return retcode;
}

/***************************************************************************
* Interface Description    : Function to tranfer HOST MEMORY WRITE command
*
* Implementation           : Uisng FT4222_SPIMaster_SingleWrite
*							 FT4222_SPIMaster_MultiReadWrite	
*
* Return Value             : ft_uint8_t
*							 1 - Success
*							 0 - Failure
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_FT4222_Wr(Ft_Gpu_Hal_Context_t *host, ft_uint32_t hwraddr, const ft_uint8_t * wrbufptr, ft_uint32_t bytestowr)
{
    ft_uint32_t SizeTransfered;    
	FT4222_STATUS status;
	ft_uint8_t * temp_wrpktptr;
    ft_uint16_t per_write = 0;
    BOOL disable_cs = FALSE; //assume multi SPI write calls    
    ft_uint8_t dummy_read;
	ft_uint8_t retcode = 1;		/* assume successful operation */		

    if (wrbufptr == NULL || bytestowr == 0)
        retcode = 0;

    if (retcode)
    {

        temp_wrpktptr = host->spiwrbuf_ptr; //global host write buffer of size FT4222_MAX_BYTES_PER_CALL
        
        if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
        {
            *(temp_wrpktptr + 0) = (hwraddr >> 16) | 0x80; //MSB bits 10 for WRITE
            *(temp_wrpktptr + 1) = (hwraddr >> 8) & 0xFF;
            *(temp_wrpktptr + 2) = hwraddr & 0xff;

            status = FT4222_SPIMaster_SingleWrite(
                host->hal_handle,
                temp_wrpktptr,
                3, //3 address bytes
                (ft_uint16_t *)&SizeTransfered,
                FALSE
            );

            if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != 3))
            {
                printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, (ft_uint16_t)SizeTransfered, status);
                retcode = 0;
            }

            if (retcode)
            {
                while (retcode && bytestowr)
                {
                    if (bytestowr <= FT4222_MAX_WR_BYTES_PER_CALL_IN_SINGLE_CH)
                    {
                        per_write = bytestowr;
                        disable_cs = TRUE;
                    }
                    else
                    {
                        per_write = FT4222_MAX_WR_BYTES_PER_CALL_IN_SINGLE_CH;
                        disable_cs = FALSE;
                    }
    
                    status = FT4222_SPIMaster_SingleWrite(
                        host->hal_handle,
                        wrbufptr,
                        per_write,
                        (ft_uint16_t *)&SizeTransfered,
                        disable_cs
                    );

                    if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != per_write))
                    {
                        printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, (ft_uint16_t)SizeTransfered, status);
                        retcode = 0;
                        if ((ft_uint16_t)SizeTransfered != per_write)
                            host->status = FT_GPU_HAL_STATUS_ERROR;
                    }

                    //continue writing more bytes
                    per_write = (ft_uint16_t)SizeTransfered;
                    wrbufptr += per_write;
                    bytestowr -= per_write;
                }
            }
        }
        else
        {
            //multi channel SPI communication
            while (bytestowr && retcode)
            {
                *(temp_wrpktptr + 0) = (hwraddr >> 16) | 0x80; //MSB bits 10 for WRITE
                *(temp_wrpktptr + 1) = (hwraddr >> 8) & 0xFF;
                *(temp_wrpktptr + 2) = hwraddr & 0xff;

                if (bytestowr <= FT4222_MAX_WR_BYTES_PER_CALL_IN_MULTI_CH) //3 for address            
                    per_write = bytestowr;
                else
                    per_write = FT4222_MAX_WR_BYTES_PER_CALL_IN_MULTI_CH;

                memcpy((temp_wrpktptr + 3), wrbufptr, per_write);

                status = FT4222_SPIMaster_MultiReadWrite(
                    host->hal_handle,
                    &dummy_read,
                    temp_wrpktptr,
                    0,
                    per_write + 3, // 3 byte of mem address
                    0,
                    &SizeTransfered
                );
                if (FT4222_OK != status)
                {
                    printf("FT4222_SPIMaster_MultiReadWrite failed, status %d\n", status);
                    retcode = 0;
                    host->status = FT_GPU_HAL_STATUS_ERROR;
                }

                hwraddr += per_write;
                bytestowr -= per_write;
                wrbufptr += per_write;
            }
        }
    }
	return retcode;
}

/***************************************************************************
* Interface Description    : Function to compute FT4222 sys clock and divisor
*                            to obtain user requested SPI communication clock
*                            Available FT4222_ClockRate (FT4222 system clock):
*                               SYS_CLK_60,
*                               SYS_CLK_24,
*                               SYS_CLK_48,
*                               SYS_CLK_80 
*                            Divisors available (FT4222_SPIClock):
*                               CLK_NONE,
*                               CLK_DIV_2,
*                               CLK_DIV_4,
*                               CLK_DIV_8,
*                               CLK_DIV_16,
*                               CLK_DIV_32,
*                               CLK_DIV_64,
*                               CLK_DIV_128,
*                               CLK_DIV_256,
*                               CLK_DIV_512 
* Implementation           : Good performance is observed with divisors other than CLK_DIV_2
*                            and CLK_DIV_4 from test report by firmware developers.
*                            Hence supporting the following clocks for SPI communication
*                               5000KHz
*                               10000KHz
*                               15000KHz
*                               20000KHz
*                               25000KHz 
*                               30000KHz
*                            Global variable host->hal_config.spi_clockrate_khz is
*                            updated accodingly   
* Return Value             : ft_bool_t
*                               TRUE : Supported by FT4222
*                               FALSE : Not supported by FT4222
*
* Author                   :
****************************************************************************/
ft_bool_t Ft_Gpu_Hal_FT4222_ComputeCLK(Ft_Gpu_Hal_Context_t *host, FT4222_ClockRate *sysclk, FT4222_SPIClock *sysdivisor)
{
    //host->hal_config.spi_clockrate_khz is the user requested SPI communication clock      

    if (host->hal_config.spi_clockrate_khz <= 5000)
    {  //set to 5000 KHz              
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_16;
    }
    else if (host->hal_config.spi_clockrate_khz > 5000 && host->hal_config.spi_clockrate_khz <= 10000)
    {
        //set to 10000 KHz
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_8;
    }
    else if (host->hal_config.spi_clockrate_khz > 10000 && host->hal_config.spi_clockrate_khz <= 15000)
    {
        //set to 15000 KHz
        *sysclk = SYS_CLK_60;
        *sysdivisor = CLK_DIV_4; 
    }
    else
    {
        //set to 20000 KHz : Maximum throughput is obeserved with this clock combination
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_4; 
    }
    printf("User Selected SPI clk : %d KHz \n", host->hal_config.spi_clockrate_khz);
    printf("Configured clk :  Ft4222 sys clk enum = %d , divisor enum = %d \n",*sysclk, *sysdivisor);        
    return(TRUE);     
}

#endif