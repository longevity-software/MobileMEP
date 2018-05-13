/*
 * utilities.h
 *
 * Created: 10/05/2018 20:35:10
 *  Author: Graham
 */ 


#ifndef UTILITIES_H_
#define UTILITIES_H_

typedef enum
{
	False,
	True
}Boolean;
 
#define GET_16_BIT_LSB(x)	((x) & 0x00FF)
#define GET_16_BIT_MSB(x)	(((x) & 0xFF00) >> 8)

#define MAKE_16_BITS(msb, lsb) (unsigned short)(((unsigned char)(msb) << 8) + (unsigned char)(lsb))

#endif /* UTILITIES_H_ */