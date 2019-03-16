#include <string.h>
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/init.h"
#include <arpa/inet.h>
#include "../include/connection_manager.h"
#include "../include/control_handler.h"
#include "../include/router_handler.h"

void init_response(int sock_index, char *cntrl_payload)
{
    printf("inside init rspn:%s\n",cntrl_payload);
    
	memcpy(&router_count, cntrl_payload, sizeof(router_count));
	memcpy(&update_interval, cntrl_payload+2, sizeof(update_interval));
    
    router_count = ntohs(router_count);
    update_interval = ntohs(update_interval);

    timeout.tv_sec=update_interval;//setting timeout for select
    printf("memset rounter count:%d and update:%d \n",router_count,update_interval);

    memset(routerID, 0,sizeof(uint16_t)*router_count);
    memset(routerPort, 0,sizeof(uint16_t)*router_count);
    memset(dataPort, 0,sizeof(uint16_t)*router_count);
    memset(cost, 0,sizeof(uint16_t)*router_count);
    memset(nextHop, 0,sizeof(uint16_t)*router_count);
    memset(destIp, 0,sizeof(uint32_t)*router_count);

printf("before loop\n");

    int MEMORY_OFFSET =4;

    for(int i =0;i<router_count;i++)
    {
        memcpy(&routerID[i], cntrl_payload + MEMORY_OFFSET, sizeof(routerID[i]));
        memcpy(&nextHop[i], cntrl_payload + MEMORY_OFFSET, sizeof(routerID[i]));
        MEMORY_OFFSET+=2;
        memcpy(&routerPort[i], cntrl_payload + MEMORY_OFFSET, sizeof(routerPort[i]));
        MEMORY_OFFSET+=2;
        memcpy(&dataPort[i], cntrl_payload + MEMORY_OFFSET, sizeof(dataPort[i]));
        MEMORY_OFFSET+=2;
        memcpy(&cost[i], cntrl_payload + MEMORY_OFFSET, sizeof(cost[i]));
        MEMORY_OFFSET+=2;
        memcpy(&destIp[i], cntrl_payload + MEMORY_OFFSET, sizeof(destIp));
        MEMORY_OFFSET+=4;

        if(cost[i]==0)
        {
            current_router_id  = i;
            isNeighbor[i] = 0;
            current_ip=destIp[i];
        }
        else if(cost[i] == 65535)
        {
            isNeighbor[i] = 0;
        }
        else
        {
            isNeighbor[i] = 1;
        }

    }
    
    printf("creating router socket with port:%d\n",routerPort[current_router_id]);
    create_router_sock(routerPort[current_router_id]);

    for(int i =0; i<router_count; i++)
    {
        for(int j =0; j< router_count; j++)
        {
            if(i == current_router_id)
            {		
                rt_table[current_router_id][j] = cost[j];
            }
            else
            {
                rt_table[i][j] = 65535; 
            }
        }
        
    }
    
    printTable();
    printf("after 2nd loop\n");

    // sending response
	uint16_t payload_len;
	char *cntrl_response_header;
	payload_len = 0;
	cntrl_response_header = create_response_header(sock_index, 1, 0, payload_len);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	
}
void printTable()
{
    
    for(int i =0; i<router_count; i++)
    {
    
        for(int j =0; j< router_count; j++)
        {
            printf("%5d|",ntohs(rt_table[i][j]));
        }
    printf("\n");    
    }
    
}
