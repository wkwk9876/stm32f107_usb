#include "lwip/ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"

#include "main.h"
#include "test_lwip.h"

struct netif gnetif; /* network interface structure */


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
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

	/*  Registers the default network interface. */
	netif_set_default(&gnetif);

	//if (netif_is_link_up(&gnetif))
	{
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&gnetif);
	}
	//else
	{
		/* When the netif link is down this function must be called */
		//netif_set_down(&gnetif);
	}
}

void my_lwip_init_done(void * arg)
{
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip init done!\r\n");
}


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

		/* wait 250 ms */
		HAL_Delay(250);
	}
}
#endif  /* USE_DHCP */


void start_lwip_thread(void const * argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(my_lwip_init_done, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
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
	
	for( ;; )
	{
		/* Delete the Init Thread */ 
		osThreadTerminate(NULL);
	}
}


