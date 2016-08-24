/*
 * broker.h
 *
 *  Created on: 30 Mar 2016
 *      Author: stuartm
 */

#ifndef BROKER_BROKER_H_
#define BROKER_BROKER_H_

#define MAX_FILENAME_LENGTH 255

static const char* CONFIG_FILENAME = "config.ini";

#define USE_THREADING

#ifdef USE_THREADING
	#define PROCESS_DATA_FN() THREAD_RunAndForget(process_data_avail, (void*) threadParams)
#else
	#define PROCESS_DATA_FN() process_data_avail((void*) threadParams)
#endif


THREADFUNC(process_data_avail);
static void wait_for_connections();
void read_data(SOCKET s, struct sockaddr_in *peerp, struct Config *config, struct Details *details);
void get_verbose_setting(struct Config *config, List* settings);
void get_wait_setting(struct Config *config, List* settings);
void get_address_setting(struct Details* details, List* settings);
void get_port_setting(struct Details* details, List* settings);
int wait_rd_socket(struct Config *brokerConfig, SOCKET listening_socket, fd_set *rd_fds, struct timeval *timeout);

#endif /* BROKER_BROKER_H_ */
