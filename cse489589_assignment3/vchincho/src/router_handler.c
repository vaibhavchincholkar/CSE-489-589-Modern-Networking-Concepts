#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>

#include "../include/global.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"
#include "../include/router_handler.h"
#include "../include/connection_manager.h"
#include "../include/init.h"
#include "../include/control_handler.h"
void create_router_sock(uint16_t r_port)
{
	int enable = 1;
	ROUTER_PORT=r_port;
	
	struct sockaddr_in router_addr;
	router_socket = socket(AF_INET, SOCK_DGRAM, 0);//router socket already defined in connectionmanager.h
	 if(router_socket < 0){
	 	printf("Failed to create router socket");
	 }
	
	if (setsockopt(router_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)// to make udp reusesable
	{
		printf("setsockopt for router failed");
	}

	bzero(&router_addr, sizeof(router_addr));

    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = destIp[current_router_id]; //still added :P
    router_addr.sin_port = r_port;
      
	if(bind(router_socket, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0)
	{
		printf("router bind failed");
	}
    
	FD_SET(router_socket, &master_list);

	if(router_socket > head_fd)
	{
		head_fd = router_socket;  
	}
}