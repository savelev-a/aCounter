/*
 * data_io.h
 *
 *  Created on: 22 июн. 2017 г.
 *      Author: alchemist
 */

#ifndef DATA_IO_H_
#define DATA_IO_H_

typedef enum {
	LEFT = 0,
	RIGHT
} PASS_DIRECTION;

void pass_register(PASS_DIRECTION direction);

#endif /* DATA_IO_H_ */
