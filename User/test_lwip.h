#ifndef __TEST_LWIP_H__
#define __TEST_LWIP_H__

#define USE_DHCP

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1 


#define TARGET_SERVER		"192.168.31.30"
#define TARGET_PORT			(8080)
#define RX_BUFFER_SIZE		(512)
#define TX_BUFFER_SIZE		(1U << 7)
#define TX_BUFFER_MASK		(TX_BUFFER_SIZE - 1)


struct TX_buffer_manage
{
	unsigned short p_write;
	unsigned short p_read;
	unsigned char tx_buffer[TX_BUFFER_SIZE];
};


void start_lwip_thread(void const * argument);

#endif
