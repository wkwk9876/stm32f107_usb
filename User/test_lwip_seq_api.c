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
#include "test_lwip_seq_api.h"

#if LWIP_NETCONN

static struct netif 		gnetif; 				/* network interface structure */
static char 				rxbuf[RX_BUFFER_SIZE];	/* network RX buffer */
struct TX_buffer_manage 	* txbuf = NULL;			/* network TX buffer */

struct netif * get_gnetif(void)
{
	return &gnetif;
}

#ifdef USE_TCP
static void tcp_send_thread(void const * argument)
{	
	ip_addr_t           	serverAddr;
	err_t               	err;
	
	struct netconn    		* conn = (struct netconn *)argument;
	struct TX_buffer_manage * tmp = NULL;
	
	tmp = (struct TX_buffer_manage *)pvPortMalloc(sizeof(struct TX_buffer_manage));
	if(NULL != tmp)
	{
		memset(tmp, 0, sizeof(struct TX_buffer_manage));
		txbuf = tmp;
		
		serverAddr.addr = ipaddr_addr(TARGET_SERVER);

		if(NULL != conn)
		{
			__PRINT_LOG__(__CRITICAL_LEVEL__, "TCP identifier create success!\r\n");
			
			err = netconn_connect(conn, &serverAddr, TARGET_PORT);
			if(ERR_OK == err)
			{
				__PRINT_LOG__(__CRITICAL_LEVEL__, "TCP connect success!\r\n");

				while(netif_is_up(&gnetif))
				{
					if(txbuf->p_write - txbuf->p_read)
					{				
						unsigned short p_rd = txbuf->p_read;
						unsigned short p_wt = txbuf->p_write;
						unsigned short len = (p_wt - p_rd) & TX_BUFFER_MASK;
						
						if(((p_rd & TX_BUFFER_MASK) + len) > TX_BUFFER_SIZE)
						{
							err = netconn_write(conn, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK], 
										TX_BUFFER_SIZE - (p_rd & TX_BUFFER_MASK), NETCONN_COPY);
							if(ERR_OK != err)
							{
								//__PRINT_LOG__(__ERR_LEVEL__, "write failed(%d)!\r\n", ret);
								break;
							}
								
							err = netconn_write(conn, &txbuf->tx_buffer[0], 
									(p_rd & TX_BUFFER_MASK) + len - TX_BUFFER_SIZE, NETCONN_COPY);
							if(ERR_OK != err)
							{
								//__PRINT_LOG__(__ERR_LEVEL__, "write failed(%d)!\r\n", ret);
								break;
							}

						}
						else
						{
							err = netconn_write(conn, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK], 
												len, NETCONN_COPY);
							if(ERR_OK != err)
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
			}
			else
			{
				__PRINT_LOG__(__ERR_LEVEL__, "TCP connect failed!\r\n");
			}
		}
		else
		{
			__PRINT_LOG__(__ERR_LEVEL__, "TCP identifier create failed!\r\n");
		}
		
		__PRINT_LOG__(__ERR_LEVEL__, "netif_is_down tx thread exit(%d)!\r\n", err);

		portENTER_CRITICAL();
		vPortFree(txbuf);
		txbuf = NULL;
		portEXIT_CRITICAL();
	}
	else
	{
		__PRINT_LOG__(__ERR_LEVEL__, "malloc txbuf failed!\r\n");
	}

	osThreadTerminate(NULL);
}

static void start_tcp_recv(struct netconn * conn)
{
	err_t               	err;
	uint16_t            	len;
	struct netbuf     		* buf;
	void              		* data;

	while(1)
	{
		if(ERR_OK == (err = netconn_recv(conn, &buf)))
	    {
	        uint32_t totallen = 0;
	        do 
	        {
	            netbuf_data(buf, &data, &len);                            
	            if(totallen + len < RX_BUFFER_SIZE)
	            {
	                memcpy(rxbuf + totallen, data, len);
	            }
	            else
	            {
	                __PRINT_LOG__(__ERR_LEVEL__, "Data too long!");
	            }
	            totallen += len;
	        } while (netbuf_next(buf) >= 0);
	        netbuf_delete(buf);
	        buf = NULL;
			__PRINT_LOG__(__CRITICAL_LEVEL__, "%s!\r\n", rxbuf);
	    }
	    else
	    {
	        if(NULL != buf)
	        {
	            __PRINT_LOG__(__ERR_LEVEL__, "Delete buffer !\r\n");
	            netbuf_delete(buf);
	        }
	        
	        if(ERR_MEM == err)
	        {                            
	            __PRINT_LOG__(__ERR_LEVEL__, "Err val: Memory Error\r\n");
	            memp_init();
	        }
	        break;
	    }
	}
}

#else

static void udp_send_thread(void const * argument)
{
	ip_addr_t           	serverAddr;
	err_t               	err;
	
	struct netconn    		* conn = (struct netconn *)argument;
	struct netbuf     		* buf;
	struct TX_buffer_manage * tmp = NULL;
	
	tmp = (struct TX_buffer_manage *)pvPortMalloc(sizeof(struct TX_buffer_manage));
	if(NULL != tmp && NULL != conn)
	{
		memset(tmp, 0, sizeof(struct TX_buffer_manage));
		txbuf = tmp;		
		serverAddr.addr = ipaddr_addr(TARGET_SERVER);

		err = netconn_bind(conn, IP_ADDR_ANY, TARGET_PORT + 1);
		if(ERR_OK == err)
		{
			err = netconn_connect(conn, &serverAddr, TARGET_PORT);
			if(ERR_OK == err)
			{
				buf = netbuf_new();
				if(NULL != buf) 
				{
					while(netif_is_up(&gnetif))
					{
						if(txbuf->p_write - txbuf->p_read)
						{
							unsigned short p_rd = txbuf->p_read;
							unsigned short p_wt = txbuf->p_write;
							unsigned short len = (p_wt - p_rd) & TX_BUFFER_MASK;								

							if(((p_rd & TX_BUFFER_MASK) + len) > TX_BUFFER_SIZE)
							{
								netbuf_ref(buf, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK],
										TX_BUFFER_SIZE - (p_rd & TX_BUFFER_MASK));

								err = netconn_send(conn, buf);
								if(ERR_OK != err)
								{
									//__PRINT_LOG__(__ERR_LEVEL__, "netconn_send failed(%d)!\r\n", err);
									break;
								}

								netbuf_ref(buf, &txbuf->tx_buffer[0],
										(p_rd & TX_BUFFER_MASK) + len - TX_BUFFER_SIZE);
								
								err = netconn_send(conn, buf);
								if(ERR_OK != err)
								{
									//__PRINT_LOG__(__ERR_LEVEL__, "netconn_send failed(%d)!\r\n", err);
									break;
								}
							}
							else
							{
 								netbuf_ref(buf, &txbuf->tx_buffer[p_rd & TX_BUFFER_MASK], len);

								err = netconn_send(conn, buf);
								if(ERR_OK != err)
								{
									//__PRINT_LOG__(__ERR_LEVEL__, "netconn_send failed(%d)!\r\n", err);
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
					if(NULL != buf)
                    {
                        netbuf_delete(buf);
                        buf = NULL;
                    }
				}
				else
				{
					__PRINT_LOG__(__ERR_LEVEL__, "netbuf_new failed!\r\n");
				}
			}
			else
			{
				__PRINT_LOG__(__ERR_LEVEL__, "UDP local port bind fail!\r\n");
			}
		}
		else
		{
			__PRINT_LOG__(__ERR_LEVEL__, "UDP netconn_connect fail!\r\n");
		}
	}
	else
	{
		__PRINT_LOG__(__ERR_LEVEL__, "malloc txbuf failed or conn is null!\r\n");
	}
	
	osThreadTerminate(NULL);
}

static void start_udp_recv(struct netconn * conn)
{
	err_t               	err;
	uint16_t            	len;
	struct netbuf     		* buf = NULL;

	while(1)
	{
		err = netconn_recv(conn, &buf);
        if(ERR_OK == err)
        {
        	len = RX_BUFFER_SIZE > buf->p->tot_len ? buf->p->tot_len : RX_BUFFER_SIZE;
			if(RX_BUFFER_SIZE < buf->p->tot_len)
			{
				__PRINT_LOG__(__ERR_LEVEL__, "data too long\r\n");
			}
			
        	if(netbuf_copy(buf, rxbuf, len) != buf->p->tot_len) 
            {
                __PRINT_LOG__(__ERR_LEVEL__, "netbuf_copy failed\r\n");
            } 
            else 
            {
                rxbuf[len] = '\0';      
                __PRINT_LOG__(__ERR_LEVEL__, "Recv message: %s\r\n", rxbuf);
            }
            netbuf_delete(buf);
            buf = NULL;             
    	}
		else
		{
			if(NULL != buf)
            {
                netbuf_delete(buf);
                buf = NULL; 
            }
			break;
		}
	}
}
#endif

struct netconn * connect_to_server()
{
	struct netconn * conn = NULL;
	
#ifdef USE_TCP
	conn = netconn_new(NETCONN_TCP);
#else
	conn = netconn_new(NETCONN_UDP);
#endif

	return conn;
}

static osThreadId init_send_thread(struct netconn * conn)
{
	osThreadId id;
	
#ifdef USE_TCP
	osThreadDef(tcp_send_thread, tcp_send_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
	id = osThreadCreate (osThread(tcp_send_thread), conn);
#else
	osThreadDef(udp_send_thread, udp_send_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
	id = osThreadCreate (osThread(udp_send_thread), conn);
#endif

	return id;
}

static void start_recv(struct netconn * conn)
{
#ifdef USE_TCP
	start_tcp_recv(conn);
#else
	start_udp_recv(conn);
#endif

}

void my_lwip_init_done(void * arg)
{
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip init done!\r\n");
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
	IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
	IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif /* USE_DHCP */

	/* add the network interface */    
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

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
}

void start_lwip_thread_seq(void const * argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(my_lwip_init_done, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

#ifdef USE_DHCP
	if(ERR_OK == dhcp_start(&gnetif))
	{		
		while(0 == dhcp_supplied_address(&gnetif))
        {
            HAL_Delay(100);
        }

		__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: gw:%s!\r\n", ipaddr_ntoa(&gnetif.gw));
		__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: ip:%s!\r\n", ipaddr_ntoa(&gnetif.ip_addr));
		__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: mask:%s!\r\n", ipaddr_ntoa(&gnetif.netmask));
#endif

		while(1)
		{
			if(netif_is_up(&gnetif))
			{
				struct netconn * conn = connect_to_server();
				if(conn)
				{
					osThreadId id;
					if(NULL == (id = init_send_thread(conn)))
					{
						__PRINT_LOG__(__ERR_LEVEL__, "lwip send init failed !\r\n");
						HAL_Delay(1000);
					}
					else
					{
						start_recv(conn);// should nerver return
						while(NULL != txbuf)
						{
							HAL_Delay(1);
						}
						//if thread Terminate itself,the resource of this thread will be release
						//only when the system in idle. If the system no idle status, the resource
						//will not be free. So, we delete it by ourself again.
						osThreadTerminate(id);
					}

					netconn_close(conn);
	                netconn_delete(conn);  
				}
				else
				{
					HAL_Delay(1000);
				}
			}
			else
			{
				HAL_Delay(1000);
			}
		}
				
#ifdef USE_DHCP
	}
	else
	{
		__PRINT_LOG__(__ERR_LEVEL__, "dhcp get error !\r\n");
	}
#endif
}
#endif
