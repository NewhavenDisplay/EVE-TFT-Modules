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

Abstract:

This file contains is functions for all UI fields.

Author : FTDI 

Revision History: 
0.1 - date 2013.04.24 - initial version
0.2 - date 2014.04.28 - Split in individual files according to platform

*/
#ifndef _FT_DATATYPES_H_
#define _FT_DATATYPES_H_


#define FT_FALSE           (0)
#define FT_TRUE            (1)

typedef char				ft_char8_t;
typedef signed char 		ft_schar8_t;
typedef unsigned char 		ft_uchar8_t;

typedef ft_uchar8_t 		ft_uint8_t;
typedef ft_uchar8_t 		byte;
typedef short  				ft_int16_t;
typedef unsigned short 		ft_uint16_t;
typedef unsigned int 		ft_uint32_t;
typedef int 				ft_int32_t;
typedef void 				ft_void_t;
typedef long long 			ft_int64_t;
typedef unsigned long long 	ft_uint64_t;
typedef float 				ft_float_t;
typedef double 				ft_double_t;
typedef char 				ft_bool_t;


typedef volatile unsigned char 	ft_vuint8_t;
typedef volatile unsigned short ft_vuint16;
typedef volatile unsigned long 	ft_vuint32;

typedef volatile char 	ft_vint8;
typedef volatile short 	ft_vint16;
typedef volatile long 	ft_vint32;

#define ft_delay(x) Ft_Gpu_Hal_Sleep(x)
#define FT_DBGPRINT(x)  printf(x)
#define ft_random(x)	rand() % x
#define min(X,Y) ((X) < (Y) ?  (X) : (Y))
#define max(X,Y) ((X) > (Y) ?  (X) : (Y))

#define ft_prog_uchar8_t  ft_uchar8_t __flash__ const
#define ft_prog_char8_t   ft_char8_t __flash__ const
#define ft_prog_uint16_t  ft_uint16_t __flash__ const

#define FT_PROGMEM __flash__
#define ft_pgm_read_byte_near(x)   (*((ft_prog_uchar8_t *)(x)))
#define ft_pgm_read_byte(x)        (*((ft_prog_uchar8_t *)(x)))
#define ft_pgm_read_word(addr)   (*((ft_prog_uint16_t *)(addr)))
#define TRUE     (1)
#endif /*_FT_DATATYPES_H_*/


/* Nothing beyond this*/




