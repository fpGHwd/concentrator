/*
 * fcs.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef FCS_H_
#define FCS_H_

#define INITFCS16	0xffff		/* Initial FCS value */
#define GOODFCS16	0xf0b8		/* Good final FCS value */

#define INITFCS32	0xffffffff	/* Initial FCS value */
#define GOODFCS32	0xdebb20e3	/* Good final FCS value */

unsigned short fcs16(unsigned short fcs, const unsigned char *cp, int len);
unsigned int fcs32(unsigned int fcs, const unsigned char *cp, int len);
#endif /* FCS_H_ */
