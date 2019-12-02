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
0.2 - date 2013.08.19 - fixed bugs
1.0 - date 2015.03.16 - Updated with new APIs for FT81X

*/
#include "FT_Platform.h"
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_SetBitmap(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t source, ft_uint16_t fmt, ft_uint16_t w, ft_uint16_t h)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETBITMAP);
  Ft_Gpu_Copro_SendCmd(phost, source);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)w<<16)|(fmt & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, h);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));  
}
ft_void_t Ft_Gpu_CoCmd_SetScratch(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t handle)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETSCRATCH);
  Ft_Gpu_Copro_SendCmd(phost, handle);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));  
}
#endif
ft_void_t Ft_Gpu_CoCmd_Text(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3 + strlen(s) + 1);
  Ft_Gpu_Copro_SendCmd(phost, CMD_TEXT);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(font&0xffff)));
  Ft_Gpu_CoCmd_SendStr(phost, s);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3 + strlen(s) + 1));  
}
ft_void_t Ft_Gpu_CoCmd_Number(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, ft_int32_t n)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);  
  Ft_Gpu_Copro_SendCmd(phost, CMD_NUMBER);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(font&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, n);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));  
}
ft_void_t Ft_Gpu_CoCmd_LoadIdentity(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_LOADIDENTITY);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));  
}
ft_void_t Ft_Gpu_CoCmd_Toggle(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t font, ft_uint16_t options, ft_uint16_t state, const ft_char8_t* s)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4 + strlen(s) + 1);     
  Ft_Gpu_Copro_SendCmd(phost, CMD_TOGGLE);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)font<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)state<<16)|options));
  Ft_Gpu_CoCmd_SendStr(phost, s);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4 + strlen(s) + 1));       
}
/* Error handling for val is not done, so better to always use range of 65535 in order that needle is drawn within display region */
ft_void_t Ft_Gpu_CoCmd_Gauge(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t major, ft_uint16_t minor, ft_uint16_t val, ft_uint16_t range)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_GAUGE);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(r&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)minor<<16)|(major&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)range<<16)|(val&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));         
}
ft_void_t Ft_Gpu_CoCmd_RegRead(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_REGREAD);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, 0);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3)); 
    
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_VideoStart(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_VIDEOSTART);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));           
}
#endif
ft_void_t Ft_Gpu_CoCmd_GetProps(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t w, ft_uint32_t h)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_GETPROPS);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, w);
  Ft_Gpu_Copro_SendCmd(phost, h);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));           
}
ft_void_t Ft_Gpu_CoCmd_Memcpy(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t dest, ft_uint32_t src, ft_uint32_t num)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEMCPY);
  Ft_Gpu_Copro_SendCmd(phost, dest);
  Ft_Gpu_Copro_SendCmd(phost, src);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));           
}
ft_void_t Ft_Gpu_CoCmd_Spinner(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_uint16_t style, ft_uint16_t scale)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_SPINNER);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)scale<<16)|(style&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));           
}
ft_void_t Ft_Gpu_CoCmd_BgColor(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t c)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_BGCOLOR);
  Ft_Gpu_Copro_SendCmd(phost, c);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));           
}
ft_void_t Ft_Gpu_CoCmd_Swap(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_SWAP);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));         
}
ft_void_t Ft_Gpu_CoCmd_Inflate(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);
  Ft_Gpu_Copro_SendCmd(phost, CMD_INFLATE);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));  
}
ft_void_t Ft_Gpu_CoCmd_Translate(Ft_Gpu_Hal_Context_t *phost,ft_int32_t tx, ft_int32_t ty)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);  
  Ft_Gpu_Copro_SendCmd(phost, CMD_TRANSLATE);
  Ft_Gpu_Copro_SendCmd(phost, tx);
  Ft_Gpu_Copro_SendCmd(phost, ty);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));  
}
ft_void_t Ft_Gpu_CoCmd_Stop(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_STOP);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));    
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_SetBase(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t base)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETBASE);
  Ft_Gpu_Copro_SendCmd(phost, base);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));    
}
#endif
ft_void_t Ft_Gpu_CoCmd_Slider(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_SLIDER);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)val<<16)|(options&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, range);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));    
}

#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_VideoFrame(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t dst, ft_uint32_t ptr)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);    
  Ft_Gpu_Copro_SendCmd(phost, CMD_VIDEOFRAME);
  Ft_Gpu_Copro_SendCmd(phost, dst);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));
}
#endif

ft_void_t Ft_Gpu_CoCmd_TouchTransform(Ft_Gpu_Hal_Context_t *phost,ft_int32_t x0, ft_int32_t y0, ft_int32_t x1, ft_int32_t y1, ft_int32_t x2, ft_int32_t y2, ft_int32_t tx0, ft_int32_t ty0, ft_int32_t tx1, ft_int32_t ty1, ft_int32_t tx2, ft_int32_t ty2, ft_uint16_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*6*2+FT_CMD_SIZE*2);      
  Ft_Gpu_Copro_SendCmd(phost, CMD_TOUCH_TRANSFORM);
  Ft_Gpu_Copro_SendCmd(phost, x0);
  Ft_Gpu_Copro_SendCmd(phost, y0);
  Ft_Gpu_Copro_SendCmd(phost, x1);
  Ft_Gpu_Copro_SendCmd(phost, y1);
  Ft_Gpu_Copro_SendCmd(phost, x2);
  Ft_Gpu_Copro_SendCmd(phost, y2);
  Ft_Gpu_Copro_SendCmd(phost, tx0);
  Ft_Gpu_Copro_SendCmd(phost, ty0);
  Ft_Gpu_Copro_SendCmd(phost, tx1);
  Ft_Gpu_Copro_SendCmd(phost, ty1);
  Ft_Gpu_Copro_SendCmd(phost, tx2);
  Ft_Gpu_Copro_SendCmd(phost, ty2);
  Ft_Gpu_Copro_SendCmd(phost, result);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*6*2+FT_CMD_SIZE*2));        
}
ft_void_t Ft_Gpu_CoCmd_Interrupt(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ms)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);        
  Ft_Gpu_Copro_SendCmd(phost, CMD_INTERRUPT);
  Ft_Gpu_Copro_SendCmd(phost, ms);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));          
}
ft_void_t Ft_Gpu_CoCmd_FgColor(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t c)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);          
  Ft_Gpu_Copro_SendCmd(phost, CMD_FGCOLOR);
  Ft_Gpu_Copro_SendCmd(phost, c);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));          
}
ft_void_t Ft_Gpu_CoCmd_Rotate(Ft_Gpu_Hal_Context_t *phost,ft_int32_t a)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);            
  Ft_Gpu_Copro_SendCmd(phost, CMD_ROTATE);
  Ft_Gpu_Copro_SendCmd(phost, a);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));          
}
ft_void_t Ft_Gpu_CoCmd_Button(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4 + strlen(s) + 1);            
  Ft_Gpu_Copro_SendCmd(phost, CMD_BUTTON);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(font&0xffff)));
  Ft_Gpu_CoCmd_SendStr(phost, s);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4 + strlen(s) + 1));              
}
ft_void_t Ft_Gpu_CoCmd_MemWrite(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t num)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEMWRITE);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
ft_void_t Ft_Gpu_CoCmd_Scrollbar(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t size, ft_uint16_t range)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_SCROLLBAR);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)val<<16)|(options&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)range<<16)|(size&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));       
}
ft_void_t Ft_Gpu_CoCmd_GetMatrix(Ft_Gpu_Hal_Context_t *phost,ft_int32_t a, ft_int32_t b, ft_int32_t c, ft_int32_t d, ft_int32_t e, ft_int32_t f)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*7);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_GETMATRIX);
  Ft_Gpu_Copro_SendCmd(phost, a);
  Ft_Gpu_Copro_SendCmd(phost, b);
  Ft_Gpu_Copro_SendCmd(phost, c);
  Ft_Gpu_Copro_SendCmd(phost, d);
  Ft_Gpu_Copro_SendCmd(phost, e);
  Ft_Gpu_Copro_SendCmd(phost, f);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*7));         
}

ft_void_t Ft_Gpu_CoCmd_Sketch(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_uint16_t w, ft_uint16_t h, ft_uint32_t ptr, ft_uint16_t format)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_SKETCH);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, format);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));         
}
#ifdef FT_801_ENABLE
ft_void_t Ft_Gpu_CoCmd_CSketch(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_uint16_t w, ft_uint16_t h, ft_uint32_t ptr, ft_uint16_t format,ft_uint16_t freq)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_CSKETCH);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)freq<<16)|(format&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));         
}
#endif
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_RomFont(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t font, ft_uint32_t romslot)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_ROMFONT);
  Ft_Gpu_Copro_SendCmd(phost, font);
  Ft_Gpu_Copro_SendCmd(phost, romslot);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));         
}
ft_void_t Ft_Gpu_CoCmd_PlayVideo(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t options)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_PLAYVIDEO);
  Ft_Gpu_Copro_SendCmd(phost, options);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));
}
#endif
ft_void_t Ft_Gpu_CoCmd_MemSet(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t value, ft_uint32_t num)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEMSET);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, value);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));         
}
ft_void_t Ft_Gpu_CoCmd_GradColor(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t c)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_GRADCOLOR);
  Ft_Gpu_Copro_SendCmd(phost, c);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));         
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_Sync(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_SYNC);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));         
}
#endif
ft_void_t Ft_Gpu_CoCmd_BitmapTransform(Ft_Gpu_Hal_Context_t *phost,ft_int32_t x0, ft_int32_t y0, ft_int32_t x1, ft_int32_t y1, ft_int32_t x2, ft_int32_t y2, ft_int32_t tx0, ft_int32_t ty0, ft_int32_t tx1, ft_int32_t ty1, ft_int32_t tx2, ft_int32_t ty2, ft_uint16_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*6*2+FT_CMD_SIZE*2);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_BITMAP_TRANSFORM);
  Ft_Gpu_Copro_SendCmd(phost, x0);
  Ft_Gpu_Copro_SendCmd(phost, y0);
  Ft_Gpu_Copro_SendCmd(phost, x1);
  Ft_Gpu_Copro_SendCmd(phost, y1);
  Ft_Gpu_Copro_SendCmd(phost, x2);
  Ft_Gpu_Copro_SendCmd(phost, y2);
  Ft_Gpu_Copro_SendCmd(phost, tx0);
  Ft_Gpu_Copro_SendCmd(phost, ty0);
  Ft_Gpu_Copro_SendCmd(phost, tx1);
  Ft_Gpu_Copro_SendCmd(phost, ty1);
  Ft_Gpu_Copro_SendCmd(phost, tx2);
  Ft_Gpu_Copro_SendCmd(phost, ty2);
  Ft_Gpu_Copro_SendCmd(phost, result);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*6*2+FT_CMD_SIZE*2));     
}
ft_void_t Ft_Gpu_CoCmd_Calibrate(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);     
  Ft_Gpu_Copro_SendCmd(phost, CMD_CALIBRATE);
  Ft_Gpu_Copro_SendCmd(phost, result);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));   
  Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
  
}
ft_void_t Ft_Gpu_CoCmd_SetFont(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t font, ft_uint32_t ptr)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETFONT);
  Ft_Gpu_Copro_SendCmd(phost, font);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
ft_void_t Ft_Gpu_CoCmd_Logo(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_LOGO);  
  Ft_Gpu_CoCmd_EndFunc(phost,FT_CMD_SIZE*1); 
}
ft_void_t Ft_Gpu_CoCmd_Append(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t num)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_APPEND);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
ft_void_t Ft_Gpu_CoCmd_MemZero(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t num)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEMZERO);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
ft_void_t Ft_Gpu_CoCmd_Scale(Ft_Gpu_Hal_Context_t *phost,ft_int32_t sx, ft_int32_t sy)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_SCALE);
  Ft_Gpu_Copro_SendCmd(phost, sx);
  Ft_Gpu_Copro_SendCmd(phost, sy);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
ft_void_t Ft_Gpu_CoCmd_Clock(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t h, ft_uint16_t m, ft_uint16_t s, ft_uint16_t ms)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_CLOCK);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(r&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)m<<16)|(h&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)ms<<16)|(s&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));       
}

ft_void_t Ft_Gpu_CoCmd_Gradient(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x0, ft_int16_t y0, ft_uint32_t rgb0, ft_int16_t x1, ft_int16_t y1, ft_uint32_t rgb1)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_GRADIENT);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y0<<16)|(x0 & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, rgb0);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y1<<16)|(x1 & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, rgb1);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));         
}

ft_void_t Ft_Gpu_CoCmd_SetMatrix(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETMATRIX);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));         
}

ft_void_t Ft_Gpu_CoCmd_Track(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t tag)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_TRACK);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, tag);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));       
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_Int_RAMShared(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_INT_RAMSHARED);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));         
}
ft_void_t Ft_Gpu_CoCmd_Int_SWLoadImage(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t options)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_INT_SWLOADIMAGE);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, options);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));         
}
#endif
ft_void_t Ft_Gpu_CoCmd_GetPtr(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_GETPTR);
  Ft_Gpu_Copro_SendCmd(phost, result);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));         
}

ft_void_t Ft_Gpu_CoCmd_Progress(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_PROGRESS);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)val<<16)|(options&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, range);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));         
}

ft_void_t Ft_Gpu_CoCmd_ColdStart(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_COLDSTART);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));       
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_MediaFifo(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t size)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);       
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEDIAFIFO);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, size);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));       
}
#endif
ft_void_t Ft_Gpu_CoCmd_Keys(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4 + strlen(s) + 1);         
  Ft_Gpu_Copro_SendCmd(phost, CMD_KEYS);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(font&0xffff)));
  Ft_Gpu_CoCmd_SendStr(phost, s);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4 + strlen(s) + 1));           
}

ft_void_t Ft_Gpu_CoCmd_Dial(Ft_Gpu_Hal_Context_t *phost,ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t val)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_DIAL);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)options<<16)|(r&0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, val);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));           
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_Snapshot2(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t fmt, ft_uint32_t ptr, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*5);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_SNAPSHOT2);
  Ft_Gpu_Copro_SendCmd(phost, fmt);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)y<<16)|(x & 0xffff)));
  Ft_Gpu_Copro_SendCmd(phost, (((ft_uint32_t)h<<16)|(w&0xffff)));
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*5));           
}
#endif
ft_void_t Ft_Gpu_CoCmd_LoadImage(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t options)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*3);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_LOADIMAGE);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, options);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*3));           
}
#ifdef FT_81X_ENABLE
ft_void_t Ft_Gpu_CoCmd_SetFont2(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t font, ft_uint32_t ptr, ft_uint32_t firstchar)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETFONT2);
  Ft_Gpu_Copro_SendCmd(phost, font);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, firstchar);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));           
}
ft_void_t Ft_Gpu_CoCmd_SetRotate(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t r)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_SETROTATE);
  Ft_Gpu_Copro_SendCmd(phost, r);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));           
}
#endif
ft_void_t Ft_Gpu_CoCmd_Dlstart(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);           
  Ft_Gpu_Copro_SendCmd(phost, CMD_DLSTART);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));             
}

ft_void_t Ft_Gpu_CoCmd_Snapshot(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*2);             
  Ft_Gpu_Copro_SendCmd(phost, CMD_SNAPSHOT);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*2));             
}

ft_void_t Ft_Gpu_CoCmd_ScreenSaver(Ft_Gpu_Hal_Context_t *phost)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*1);             
  Ft_Gpu_Copro_SendCmd(phost, CMD_SCREENSAVER);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*1));             
}
ft_void_t Ft_Gpu_CoCmd_MemCrc(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t ptr, ft_uint32_t num, ft_uint32_t result)
{
  Ft_Gpu_CoCmd_StartFunc(phost,FT_CMD_SIZE*4);             
  Ft_Gpu_Copro_SendCmd(phost, CMD_MEMCRC);
  Ft_Gpu_Copro_SendCmd(phost, ptr);
  Ft_Gpu_Copro_SendCmd(phost, num);
  Ft_Gpu_Copro_SendCmd(phost, result);
  Ft_Gpu_CoCmd_EndFunc(phost,(FT_CMD_SIZE*4));             
}



ft_void_t Ft_Gpu_Copro_SendCmd(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{ 
  
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
#ifdef BUFFER_OPTIMIZATION
   Ft_App_WrCoCmd_Buffer(phost,cmd);
#else
	//Ft_App_WrCoCmd_Buffer(phost,cmd);
	Ft_Gpu_Hal_Transfer32(phost,cmd);
#endif
#endif

#ifdef ARDUINO_PLATFORM
#ifdef ARDUINO_PLATFORM_COCMD_BURST
   Ft_Gpu_Hal_Transfer32(phost,cmd);
#else
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
#endif
#endif

#ifdef FT900_PLATFORM
   Ft_Gpu_Hal_Transfer32(phost,cmd);
#endif
}

ft_void_t Ft_Gpu_CoCmd_SendStr(Ft_Gpu_Hal_Context_t *phost,const ft_char8_t *s)
{
  
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
#ifdef BUFFER_OPTIMIZATION 
	Ft_App_WrCoStr_Buffer(phost,s);
#else
	Ft_Gpu_Hal_TransferString(phost,s);
#endif
#endif  

#ifdef ARDUINO_PLATFORM
#ifdef ARDUINO_PLATFORM_COCMD_BURST
  Ft_Gpu_Hal_TransferString(phost,s);
#else
  ft_uint16_t length = 0;
  length = strlen(s) + 1;//last for the null termination 
  Ft_Gpu_Hal_WrCmdBuf(phost,(ft_uint8_t*)s,length);
#endif  
#endif

#ifdef FT900_PLATFORM
  Ft_Gpu_Hal_TransferString(phost,s);
#endif
}


ft_void_t Ft_Gpu_CoCmd_StartFunc(Ft_Gpu_Hal_Context_t *phost,ft_uint16_t count)
{
#ifdef ARDUINO_PLATFORM  
#ifdef ARDUINO_PLATFORM_COCMD_BURST 
  Ft_Gpu_Hal_CheckCmdBuffer(phost,count);
  Ft_Gpu_Hal_StartCmdTransfer(phost,FT_GPU_WRITE,count);
#endif  
#endif

#ifdef FT900_PLATFORM
  	Ft_Gpu_Hal_CheckCmdBuffer(phost,count);
    Ft_Gpu_Hal_StartCmdTransfer(phost,FT_GPU_WRITE,count);
#endif

#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
#ifndef BUFFER_OPTIMIZATION
  	Ft_Gpu_Hal_CheckCmdBuffer(phost,count);
    Ft_Gpu_Hal_StartCmdTransfer(phost,FT_GPU_WRITE,count);
#endif
#endif
}


ft_void_t Ft_Gpu_CoCmd_EndFunc(Ft_Gpu_Hal_Context_t *phost,ft_uint16_t count)
{
#ifdef ARDUINO_PLATFORM    
#ifdef ARDUINO_PLATFORM_COCMD_BURST 
  Ft_Gpu_Hal_EndTransfer(phost);
  Ft_Gpu_Hal_Updatecmdfifo(phost,count);
#endif
#endif

#ifdef FT900_PLATFORM
  Ft_Gpu_Hal_EndTransfer(phost);
  Ft_Gpu_Hal_Updatecmdfifo(phost,count);
 #endif


#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
#ifndef BUFFER_OPTIMIZATION
  Ft_Gpu_Hal_EndTransfer(phost);
  Ft_Gpu_Hal_Updatecmdfifo(phost,count);
#endif
#endif

}
/* Nothing beyond this */



