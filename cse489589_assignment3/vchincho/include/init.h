#ifndef INIT_H_
#define INIT_H_

uint16_t  router_count, update_interval;
uint16_t *routerID,*routerPort, *dataPort, *cost, *nextHop;
uint32_t *destIp;
uint32_t current_ip;
uint16_t rt_table[5][5];
int current_router_id;
int isNeighbor[5];

bool router_init;
void init_response(int sock_index, char* cntrl_payload);

#endif

