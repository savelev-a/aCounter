/*
 * wifi.h
 *
 *  Created on: 3 июл. 2017 г.
 *      Author: alchemist
 */

#ifndef WIFI_H_
#define WIFI_H_

#define WIFI_NOTREADY_ERROR 1
#define WIFI_INIT_ERROR 2
#define WIFI_CONNECT_ERROR 3

uint8_t wifi_init();
void wifi_process();

#endif /* WIFI_H_ */
