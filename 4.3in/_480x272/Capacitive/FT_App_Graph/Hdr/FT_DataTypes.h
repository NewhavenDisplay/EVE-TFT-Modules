#ifndef _FT_DATATYPES_H_
#define _FT_DATATYPES_H_


#define FT_FALSE           (0)
#define FT_TRUE            (1)

/* Inclusion of datatypes from MSVC */
#ifdef MSVC_PLATFORM
typedef char ft_char8_t;
typedef signed char ft_schar8_t;
typedef unsigned char ft_uchar8_t;
typedef ft_uchar8_t ft_uint8_t;
typedef short  ft_int16_t;
typedef unsigned short ft_uint16_t;
typedef unsigned int ft_uint32_t;
typedef int ft_int32_t;
typedef void ft_void_t;
typedef long long ft_int64_t;
typedef unsigned long long ft_uint64_t;
typedef float ft_float_t;
typedef double ft_double_t;
typedef char ft_bool_t;

#define FT_BYTE_SIZE (1)
#define FT_SHORT_SIZE (2)
#define FT_WORD_SIZE (4)
#define FT_DWORD_SIZE (8)

#define FT_NUMBITS_IN_BYTE (1*8)
#define FT_NUMBITS_IN_SHORT (2*8)
#define FT_NUMBITS_IN_WORD (4*8)
#define FT_NUMBITS_IN_DWORD (8*8)

#define ft_prog_uchar8_t  ft_uchar8_t
#define ft_prog_char8_t   ft_char8_t
#define ft_prog_uint16_t  ft_uint16_t


#define ft_random(x)		(rand() % (x))
#define ft_millis()         GetTickCount()

#define ft_pgm_read_byte_near(x)   (*(x))
#define ft_pgm_read_byte(x)        (*(x))

#define ft_strcpy_P     strcpy
#define ft_strlen_P     strlen

#define ft_delay(x) Ft_Gpu_Hal_Sleep(x)
#define FT_DBGPRINT(x)  printf(x)
#define FT_PROGMEM

#define ft_pgm_read_byte_near(x)   (*(x))
#define ft_pgm_read_byte(x)        (*(x))

#define ft_pgm_read_word(addr)   (*(ft_int16_t*)(addr))
#endif


#ifdef ARDUINO_PLATFORM
typedef byte ft_uint8_t;
typedef char ft_char8_t;
typedef signed char ft_schar8_t;
typedef unsigned char ft_uchar8_t;
typedef int  ft_int16_t;
typedef word ft_uint16_t;
typedef unsigned long ft_uint32_t;
typedef long ft_int32_t;
typedef void ft_void_t;

typedef boolean ft_bool_t;

typedef prog_uchar  ft_prog_uchar8_t;
typedef prog_char   ft_prog_char8_t;
typedef prog_uint16_t ft_prog_uint16_t;


//#define ft_delay(x) delay(x)

#define FT_PROGMEM PROGMEM
#define ft_pgm_read_byte_near pgm_read_byte_near
#define ft_pgm_read_byte pgm_read_byte
#define ft_pgm_read_word pgm_read_word
#define ft_random(x)		(random(x))

#define TRUE     (1)

#endif


#endif /*_FT_DATATYPES_H_*/


/* Nothing beyond this*/




