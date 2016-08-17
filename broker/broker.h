/*
 * broker.h
 *
 *  Created on: 30 Mar 2016
 *      Author: stuartm
 */

#ifndef BROKER_BROKER_H_
#define BROKER_BROKER_H_

#define GetAServerSocket(our_address, port) netTcpServer((our_address), (port))
#define MAX_FILENAME_LENGTH 255

//struct ServiceRegistration service_repository;

static const char* CONFIG_FILENAME = "config.ini";


#ifdef USE_THREADING
#undef USE_THREADING
#endif


THREADFUNC(fnOnConnect);
void SetupAndRegisterCmdArgs();
static void wait_for_connections();
void readDataOnSocket(SOCKET s, struct sockaddr_in *peerp, struct Config *brokerConfig, struct Details *brokerDetails);
void GetVerboseConfigSetting(struct Config *brokerConfig, List* settings);
void setVerboseFlag(char *verbose, int numExtraArgs, ...);
void GetWaitIndefConfigSetting(struct Config *brokerConfig, List* settings);
void setWaitIndefinitelyFlag(char *arg, int numExtraArgs, ...);
void GetBrokerAddressConfigSetting(struct Details* brokerDetails, List* settings);
void GetBrokerPortConfigSettings(struct Details* brokerDetails, List* settings);
void setPortNumber(char *arg, int numExtraArgs, ...);
int _wait(struct Config *brokerConfig, SOCKET listening_socket, fd_set *read_file_descriptors, struct timeval *timeout);

#endif /* BROKER_BROKER_H_ */
