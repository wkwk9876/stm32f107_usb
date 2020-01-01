#ifndef __TEST_LWIP_ECHO_H__
#define __TEST_LWIP_ECHO_H__

#define UDP_ECHO_PORT	8888
#define TCP_ECHO_PORT	8888

void start_lwip_echo_thread(void const * argument);

#endif

