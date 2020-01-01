#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/api.h"

#include "main.h"
#include "test_lwip.h"
#include "test_lwip_seq_api.h"
#include "test_lwip_tcp_udp_echo_server.h"

static void udpecho_thread(void const * argument)
{
    static struct netconn *conn;
    static struct netbuf *buf;
    //char buffer[1500];
    err_t err;

	__PRINT_LOG__(__CRITICAL_LEVEL__, "UDP echo thread start !\r\n");

    conn = netconn_new(NETCONN_UDP);
    LWIP_ASSERT("con != NULL", conn != NULL);
    netconn_bind(conn, NULL, UDP_ECHO_PORT);

    while (1) 
    {
        err = netconn_recv(conn, &buf);
        if (err == ERR_OK) 
        {        
        	err = netconn_send(conn, buf);
            /*if(netbuf_copy(buf, buffer, buf->p->tot_len) != buf->p->tot_len) 
            {
                __PRINT_LOG__(__ERR_LEVEL__, "netbuf_copy failed\r\n");
            } 
            else 
            {
                //buffer[buf->p->tot_len] = '\0';
                
                err = netconn_send(conn, buf);
                if(err != ERR_OK) 
                {
                    __PRINT_LOG__(__ERR_LEVEL__, "netconn_send failed: %d\r\n", (int)err);
                } 
                else 
                {                    
                    //__PRINT_LOG__(__ERR_LEVEL__, "got %s\r\n", buffer + 4);
                }
            }*/
            netbuf_delete(buf);
        }
		else
		{
			__PRINT_LOG__(__ERR_LEVEL__, "netconn_recv failed: %d\r\n", (int)err);
			if(NULL != buf)
            {
                netbuf_delete(buf);
                buf = NULL; 
            }
		}
    }
}

static void tcpecho_thread(void const * argument)
{
    struct netconn *conn, *newconn;
    err_t err;

    __PRINT_LOG__(__CRITICAL_LEVEL__, "TCP echo thread start !\r\n");

    // Create a new connection identifier
    conn = netconn_new(NETCONN_TCP);

    // Bind connection to well known port number 7
    err = netconn_bind(conn, NULL, TCP_ECHO_PORT);
    if(err != ERR_OK)
    {
        __PRINT_LOG__(__ERR_LEVEL__, "bind failed !\r\n"); 
    }

    // Tell connection to go into listening mode
    netconn_listen(conn);

    __PRINT_LOG__(__ERR_LEVEL__, "TCP listening !\r\n");

    while (1) 
    {
        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);
        /* Process the new connection. */
        if (err == ERR_OK) 
        {
            struct netbuf *buf;
            void *data;
            u16_t len;

            __PRINT_LOG__(__CRITICAL_LEVEL__, "accept new !\r\n");

            while ((err = netconn_recv(newconn, &buf)) == ERR_OK) 
            {
                do 
                {
                    netbuf_data(buf, &data, &len);
                    err = netconn_write(newconn, data, len, NETCONN_COPY);
                } while (netbuf_next(buf) >= 0);
                netbuf_delete(buf);
            }
            /* Close connection and discard connection identifier. */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}

osThreadId udp_echo_init(void)
{    
	osThreadId id;
	
	osThreadDef(udpecho_thread, udpecho_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
	id = osThreadCreate (osThread(udpecho_thread), NULL);

	return id;
}

osThreadId tcp_echo_init(void)
{
	osThreadId id;

	osThreadDef(tcpecho_thread, tcpecho_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
	id = osThreadCreate (osThread(tcpecho_thread), NULL);

	return id;
}

void start_lwip_echo_thread(void const * argument)
{
	while(0 == dhcp_supplied_address(get_gnetif()))
    {
        HAL_Delay(100);
    }

	udp_echo_init();
	tcp_echo_init();

	osThreadTerminate(NULL);
}

