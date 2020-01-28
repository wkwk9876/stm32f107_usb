#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"


#include "main.h"
#include "test_lwip.h"

#if LWIP_SOCKET

#ifdef USE_DHCP
/* DHCP process states */
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5


#define MAX_DHCP_TRIES  16
__IO uint8_t DHCP_state = DHCP_OFF;
#endif

static struct netif 	gnetif; /* network interface structure */
static char 			rxbuf[RX_BUFFER_SIZE];
struct TX_buffer_manage * txbuf = NULL;

struct netif * get_gnetif(void)
{
	return &gnetif;
}

void ethernetif_notify_conn_changed(struct netif *netif)
{
#ifndef USE_DHCP
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
#endif
  
	if(netif_is_link_up(netif))
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "The network cable is now connected \n");

#ifdef USE_DHCP
		/* Update DHCP state machine */
		DHCP_state = DHCP_START;
#else
		IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
		IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
		IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);  

		netif_set_addr(netif, &ipaddr , &netmask, &gw);      
#endif /* USE_DHCP */   

		/* When the netif is fully configured this function must be called.*/
		netif_set_up(netif);     
	}
	else
	{
#ifdef USE_DHCP
		/* Update DHCP state machine */
		DHCP_state = DHCP_LINK_DOWN;
#endif /* USE_DHCP */

		/*  When the netif link is down this function must be called.*/
		netif_set_down(netif);

		__PRINT_LOG__(__CRITICAL_LEVEL__, "The network cable is not connected \n");
	}
}


static void Netif_Config(void)
{
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;

#ifdef USE_DHCP
	ip_addr_set_zero_ip4(&ipaddr);
	ip_addr_set_zero_ip4(&netmask);
	ip_addr_set_zero_ip4(&gw);
#else
	IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
	IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
	IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */

	/* add the network interface */    
	while(NULL == netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input))
	{
		__PRINT_LOG__(__ERR_LEVEL__, "netif_add failed! Please check network connect!\r\n");
		HAL_Delay(1000);
	}

	/*  Registers the default network interface. */
	netif_set_default(&gnetif);

	if (netif_is_link_up(&gnetif))
	{
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&gnetif);
	}
	else
	{
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}

	/* Set the link callback function, this function is called on change of link status */
	//netif_set_link_callback(&gnetif, ethernetif_update_config);
}


#ifdef USE_DHCP
/**
* @brief  DHCP Process
* @param  argument: network interface
* @retval None
*/
void DHCP_thread(void const * argument)
{
	struct netif *netif = (struct netif *) argument;
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
	struct dhcp *dhcp;

	__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_thread(%d)!\r\n", DHCP_state);

	for (;;)
	{
		switch (DHCP_state)
		{
			case DHCP_START:
			{
				ip_addr_set_zero_ip4(&netif->ip_addr);
				ip_addr_set_zero_ip4(&netif->netmask);
				ip_addr_set_zero_ip4(&netif->gw);     
				dhcp_start(netif);
				DHCP_state = DHCP_WAIT_ADDRESS;

				__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_START!\r\n");
			}
			break;

			case DHCP_WAIT_ADDRESS:
			{                
				if (dhcp_supplied_address(netif)) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;	

					__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: gw:%s!\r\n", ipaddr_ntoa(&netif->gw));

					__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: ip:%s!\r\n", ipaddr_ntoa(&netif->ip_addr));

					__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: mask:%s!\r\n", ipaddr_ntoa(&netif->netmask));
				}
				else
				{
					dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

					/* DHCP timeout */
					if (dhcp->tries > MAX_DHCP_TRIES)
					{
						DHCP_state = DHCP_TIMEOUT;

						/* Stop DHCP */
						dhcp_stop(netif);

						/* Static address used */
						IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
						IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
						IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
						netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

						__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_TIMEOUT!\r\n");

					}
					else
					{
						__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_GETTING(%d %d)!\r\n", dhcp->state, dhcp->tries);
					}
				}
			}
			break;
			
			case DHCP_LINK_DOWN:
			{
				/* Stop DHCP */
				dhcp_stop(netif);
				DHCP_state = DHCP_OFF; 
				__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_LINK_DOWN!\r\n");
			}
			break;
			
			default: 
			break;
		}

		/* wait 1000 ms */
		HAL_Delay(1000);
	}
}
#endif  /* USE_DHCP */


void my_lwip_init_done(void * arg)
{
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip init done!\r\n");
}


int connect_to_server(struct netif *netif)
{
	struct sockaddr_in server_addr;
	int socket_fd = -1, ret = -1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = PP_HTONS(TARGET_PORT);
	server_addr.sin_addr.s_addr = inet_addr(TARGET_SERVER);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd > 0)
	{
		ret = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		if(ret != 0)
		{
			__PRINT_LOG__(__ERR_LEVEL__, "connect failed!\r\n");
			close(socket_fd);
			return -1;
		}
		
		__PRINT_LOG__(__CRITICAL_LEVEL__, "connect success!\r\n");
		
		return socket_fd;
	}
	else
	{
		__PRINT_LOG__(__ERR_LEVEL__, "create socket failed(%d)!\r\n", socket_fd);
		return socket_fd;
	}
}

void send_thread(void const * argument)
{
	int * socket_fd = (int *) argument;
	int ret;
	struct TX_buffer_manage * tmp = NULL;
	
	tmp = (struct TX_buffer_manage *)pvPortMalloc(sizeof(struct TX_buffer_manage));

	if(NULL != tmp)
	{
		memset(tmp, 0, sizeof(struct TX_buffer_manage));
		txbuf = tmp;
		
		while(netif_is_up(&gnetif))
		{
			if(txbuf->p_write - txbuf->p_read)
			{				
				unsigned short p_rd = txbuf->p_read;
				unsigned short p_wt = txbuf->p_write;
				unsigned short len = (p_wt - p_rd) & TX_BUFFER_MASK;
				
				if(((p_rd & TX_BUFFER_MASK) + len) > TX_BUFFER_SIZE)
				{
					ret = write(*socket_fd, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK], 
								TX_BUFFER_SIZE - (p_rd & TX_BUFFER_MASK));
					if(ret <= 0)
					{
						//__PRINT_LOG__(__ERR_LEVEL__, "write failed(%d)!\r\n", ret);
						break;
					}
						
					ret = write(*socket_fd, &txbuf->tx_buffer[0], 
							(p_rd & TX_BUFFER_MASK) + len - TX_BUFFER_SIZE);
					if(ret <= 0)
					{
						//__PRINT_LOG__(__ERR_LEVEL__, "write failed(%d)!\r\n", ret);
						break;
					}

				}
				else
				{
					ret = write(*socket_fd, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK], len);
					if(ret <= 0)
					{
						//__PRINT_LOG__(__ERR_LEVEL__, "write failed(%d)!\r\n", ret);
						break;
					}
				}

				txbuf->p_read += (p_wt - p_rd);
			}
			else
			{
				HAL_Delay(1);
			}
		}

		__PRINT_LOG__(__ERR_LEVEL__, "netif_is_down tx thread exit(%d)!\r\n", ret);

		portENTER_CRITICAL();
		vPortFree(txbuf);
		txbuf = NULL;
		portEXIT_CRITICAL();
	}
	else
	{
		__PRINT_LOG__(__ERR_LEVEL__, "malloc txbuf failed!\r\n");
	}

	/* Delete the Init Thread */ 
	for( ;; )
	{
		osThreadTerminate(NULL);
	}
}

int recv_data(struct netif *netif, int socket)
{
	int ret;
	
	while(1)
	{
		ret = read(socket, rxbuf, RX_BUFFER_SIZE);
		if(ret > 0)
			__PRINT_LOG__(__CRITICAL_LEVEL__, "%s!\r\n", rxbuf);
		else
			break;
	}
	return ret;
}

void start_lwip_thread(void const * argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(my_lwip_init_done, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate (osThread(DHCP), &gnetif);
#endif

	while(0 == netif_is_up(&gnetif))
	{
        delay_ms(250);
	}
	
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#endif 

	__PRINT_LOG__(__CRITICAL_LEVEL__, "netif_is_up!\r\n");

	while(1)
	{
		if(netif_is_up(&gnetif))
		{

			int socket_fd = -1;
			if((socket_fd = connect_to_server(&gnetif)) > 0)
			{
				osThreadId ret;
				osThreadDef(send_thread, send_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
				ret = osThreadCreate (osThread(send_thread), &socket_fd);
				if(NULL == ret)
				{
					__PRINT_LOG__(__ERR_LEVEL__, "osThreadCreate failed!\r\n");
				}
				else
				{
					recv_data(&gnetif, socket_fd);

					while(NULL != txbuf)
					{
						HAL_Delay(1);
					}
					//if thread Terminate itself,the resource of this thread will be release
					//only when the system in idle. If the system no idle status, the resource
					//will not be free. So, we delete it by ourself again.
					osThreadTerminate(ret);

					//if recv return
				}				

				close(socket_fd);
			}
			else
			{
				HAL_Delay(1000);
			}
		}
		else
		{
			__PRINT_LOG__(__CRITICAL_LEVEL__, "netif_is_down!\r\n");
			HAL_Delay(1000);
		}
	}
	
	/*for( ;; )
	{
		osThreadTerminate(NULL);
	}*/
}

#endif
