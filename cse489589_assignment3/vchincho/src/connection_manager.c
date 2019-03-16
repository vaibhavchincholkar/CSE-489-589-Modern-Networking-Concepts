/**
 * @connection_manager
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/init.h"

void main_loop()
{
    int selret, sock_index, fdaccept;     
    timeout.tv_sec = 1;
    timeout.tv_usec = 1000000;

    while(TRUE){
        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, &timeout);

        if(selret < 0)
        {
            ERROR("select failed.");
        }
         if(selret == 0)
        {
            //timeout has occured 
            //check if routing table is inititalized or not
            if(router_init)
            {
                //now lets create routing table
                char *packet;
                int offset=0;
                packet=(char *) malloc(sizeof(char)*1000);
                memcpy(&router_count, packet+offset, sizeof(router_count));
                offset+=2;
                memcpy(&ROUTER_PORT, packet+offset , sizeof(ROUTER_PORT));
                offset+=2;
                memcpy(&current_ip, packet+offset , sizeof(current_ip));
                offset+=4;
                uint16_t padding=0;
                for(int i=0;i<router_count;i++)
                {   
                    if(i!=current_router_id)
                    {
                        memcpy(&destIp[i] , packet+offset , sizeof(destIp[i]));
                        offset+=4;
                        memcpy(&routerPort[i] , packet+offset , sizeof(destIp[i]));
                        offset+=2;
                        memcpy(&padding , packet+offset , sizeof(padding));
                        offset+=2;
                        memcpy(&routerID[i] , packet+offset , sizeof(routerID[i]));
                        offset+=2;
                        memcpy(&cost[i] , packet+offset , sizeof(cost[i]));
                        offset+=2;
                    }
                }
                printf("packet is:%s\n",packet);
                
                //finally send packet every neighbor
                for(int i =0;i<router_count;i++)
                {
                    if(isNeighbor[i] == 1)
                    {
                        struct sockaddr_in neighbor_addr;
                        int port = ntohs(routerPort[i]);
                        int udp_router_sock;
                        socklen_t addrlen = sizeof(neighbor_addr);
                        udp_router_sock = socket(AF_INET, SOCK_DGRAM, 0);
                        if(udp_router_sock < 0)
                            ERROR("socket() failed");
                        if(setsockopt(udp_router_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
                            ERROR("setsockopt() failed");
                        bzero(&neighbor_addr, sizeof(neighbor_addr));
                        neighbor_addr.sin_family = AF_INET;
                        neighbor_addr.sin_port = routerPort[i];              
                        neighbor_addr.sin_addr.s_addr = destIp[i];
                        sendto(udp_router_sock, packet, strlen(packet), 0, (struct sockaddr *)&neighbor_addr, sizeof(neighbor_addr));

                    }
                }

            }

        }

        /* Loop through file descriptors to check which ones are ready */
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            if(FD_ISSET(sock_index, &watch_list))
            {

                /* control_socket */
                if(sock_index == control_socket){
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }
                /* router_socket */
                else if(sock_index == router_socket){
                    //call handler that will call recvfrom() .....
                }
                /* data_socket */
                else if(sock_index == data_socket){
                    //new_data_conn(sock_index);
                }
                /* Existing connection */
                else
                {
                    if(isControl(sock_index))
                    {
                        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");
                }
            }
        }
    }
}

void init()
{
    printf("Initializing the router\n");
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    main_loop();
}
