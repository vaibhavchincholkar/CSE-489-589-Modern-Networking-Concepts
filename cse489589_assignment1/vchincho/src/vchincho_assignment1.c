/**
 * @vchincho_assignment1
 * @author  vaibhav narayan chincholkar <vchincho@buffalo.edu>
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
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include "../include/logger.h"
#include "../include/global.h"

#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <errno.h>


#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

void clientside(int client_port);
void serverSide(int server_port);
int bind_the_socket(int c_port);
int connect_to_host(char *server_ip, int server_port, int c_port);
void getmyport();
void showIP();
void print_list();
void print_stats();
int isvalidIP(char *ip);
void sort_list_port();
void remove_from_list(int sock_delete);


int fdsocket,client_head_socket, client_sock_index;
fd_set client_master_list, client_watch_list;
struct client_msg
{
	char cmd[20];
	char ip[32];
	char info[256];
};

struct client_block_list
{
	int C_id;
	char C_ip[32];
	char ip1[32];
	char ip2[32];
	char ip3[32];
	char ip4[32];
	char buffer[1024];
}*client_ptr[5];


struct list_content
{
	int list_id;
	char list_host_name[40];
	char list_ip[32];
	int list_port;
	int fd_socket;
	int rcv_msg;
	int snd_msg;
	char state[20];

}*list_ptr[5];

struct server_msg
{
	char cmd[20];
	char sender_ip[32];
	char info[256];
	struct list_content list_row;
};


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	struct list_content hosts[6];
	if(argc != 3) 
	{
		printf("please enter two argument c/s and PORT number");
		exit(-1);
	}
	if(*argv[1]=='s')
	{
		serverSide(atoi(argv[2]));
	}
	else if(*argv[1]=='c')
	{
		clientside(atoi(argv[2]));
	}
	else
	{
		printf("Exiting the application");
		exit(-1);
	}
	return 0;
}
//client side

void clientside(int client_port)
{
	int bind_port=bind_the_socket(client_port);
	int loggedin=0;//using as bool
	if(bind_port==0)
	{
		exit(-1);
	}

	printf("\ninside client side");
	int server=0;//SOCKET FOR SERVER COMMUNICATION
	int selret;
	int j=0;
	struct client_msg data;
	
	FD_ZERO(&client_master_list);//Initializes the file descriptor set fdset to have zero bits for all file descriptors. 
    FD_ZERO(&client_watch_list);
    FD_SET(STDIN, &client_master_list);
    client_head_socket=0;
	while(TRUE)
	{
		fflush(stdout);	
		FD_ZERO(&client_master_list);//Initializes the file descriptor set fdset to have zero bits for all file descriptors. 
    	FD_ZERO(&client_watch_list);

    	FD_SET(STDIN, &client_master_list);
		FD_SET(server, &client_master_list);
		client_head_socket=server;

		memcpy(&client_watch_list, &client_master_list, sizeof(client_master_list));
        /* select() system call. This will BLOCK */
        selret = select(client_head_socket + 1, &client_watch_list, NULL, NULL, NULL);
        if(selret < 0)
        {
            perror("select failed.");
            exit(-1);
        }

        if(selret > 0)
        {
            /* Loop through socket descriptors to check which ones are ready */
            for(client_sock_index=0; client_sock_index<=client_head_socket; client_sock_index+=1)
            {
                if(FD_ISSET(client_sock_index, &client_watch_list))
                {
                	 if (client_sock_index == STDIN)
                    {
				        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);//MSG_SIZE=256
				    	memset(msg, '\0', MSG_SIZE);
						if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
							exit(-1);
						/*to get rid of '\n' added by fgets*/
						int len=strlen(msg);
						msg[len-1]='\0';

						//printf("I got: %s(size:%d chars)", msg, strlen(msg));

						if((strcmp(msg,"AUTHOR"))==0)
						{
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, vchincho, have read and understood the course academic integrity policy.\n");
							cse4589_print_and_log("[AUTHOR:END]\n");
						}
						else if((strcmp(msg,"IP"))==0)
						{
							showIP();
							cse4589_print_and_log("[IP:END]\n");
						}
						else if((strcmp(msg,"PORT"))==0)
						{
							getmyport();
							cse4589_print_and_log("[PORT:END]\n");
						}
						else if((strcmp(msg,"LIST"))==0 &&
								loggedin==1)
						{
							strcpy(data.cmd,"LIST");
							if(send(server, &data, sizeof(data), 0) == sizeof(data))
							{
								cse4589_print_and_log("[LIST:SUCCESS]\n");
							}
							else
							{
								cse4589_print_and_log("[LIST:ERROR]\n");
							}
							
							fflush(stdout);
						}
						else if((strncmp(msg,"LOGIN",5))==0)
						{
							char ip[32];
							char portv[32];
							int k=6;
							j=0;
							while(msg[k]!=' ')
							{
								ip[j]=msg[k];
								j=j+1;
								k=k+1;
							}
							ip[j]='\0';
							//printf("\n your ip add is %s",ip);
							if((isvalidIP(ip))==1)
							{
								//printf("\n you entered a valid IP address");
								j=0;
								k=k+1;
								while(msg[k]!='\0')
								{
									portv[j]=msg[k];
									k=k+1;
									j=j+1;
								}
								portv[j]='\0';
								int length = strlen (portv);
								int p_error=0;
							    for (int i=0;i<length; i++)
							    {
							        if (!isdigit(portv[i]))
							        {
							            printf ("Entered input is not a number\n");
							            p_error=1;
							        }
							    }
								if(p_error==1)
								{
									cse4589_print_and_log("[LOGIN:ERROR]\n");
									cse4589_print_and_log("[LOGIN:END]\n");
								}
								else
								{
									int l=1;
									int u=65535;
									int port_check=atoi(portv);
									if( l <= port_check && port_check <= u)
									{	/*connect to server*/
										server=connect_to_host(ip, atoi(portv), client_port);/*atoi converts the string argument str to an integer (type int).*/
										FD_SET(server, &client_master_list);
										client_head_socket=server;
										loggedin=1;
										cse4589_print_and_log("[LOGIN:SUCCESS]\n");
									}
									else
									{
										cse4589_print_and_log("[LOGIN:ERROR]\n");
										cse4589_print_and_log("[LOGIN:END]\n");
									}
								}
							}
							else
							{
								cse4589_print_and_log("[LOGIN:ERROR]\n");
								cse4589_print_and_log("[LOGIN:END]\n");
							}
							
							fflush(stdout);
						}
						else if((strcmp(msg,"REFRESH"))==0&&
								loggedin==0)
				        {  
				        	cse4589_print_and_log("[REFRESH:SUCCESS]\n"); 
				        	cse4589_print_and_log("[REFRESH:END]\n");                 
				        }
				        else if((strncmp(msg,"SEND",4))==0&&
								loggedin==1)
				        {
				        	char cip[32];
							char message[256];
							int k=5;
							j=0;

							while(msg[k]!=' ')
							{
								cip[j]=msg[k];
								j=j+1;
								k=k+1;
							}
							cip[j]='\0';
							//printf("\n your ip add is %s",cip);

							if((isvalidIP(cip))==1)
							{
								//printf("\n you entered a valid IP address");
								//cse4589_print_and_log("[SEND:SUCCESS]\n");
								j=0;
								k=k+1;
								while(msg[k]!='\0')
								{
									message[j]=msg[k];
									k=k+1;
									j=j+1;
								}
								message[j]='\0';
								//printf("\n Entered message is %s",message);
								k=0;
								j=0;
								strcpy(data.cmd,"SEND");
								strcpy(data.ip,cip);
								strcpy(data.info,message);
								if(send(server, &data, sizeof(data), 0) == sizeof(data))
								{
									//cse4589_print_and_log("[SEND:SUCCESS]\n");
								}
							}
							else
							{
								//EXCEPTION IP	
								cse4589_print_and_log("[SEND:ERROR]\n");
								cse4589_print_and_log("[SEND:END]\n");
							}
							
							fflush(stdout);
				        }
						else if((strncmp(msg,"BROADCAST",9))==0&&
								loggedin==1)
						{
							char message[256];
							int k=10;
							j=0;
							while(msg[k]!='\0')
							{
								message[j]=msg[k];
								k=k+1;
								j=j+1;
							}
							strcpy(data.cmd,"BROADCAST");
							strcpy(data.info,message);
							if(send(server, &data, sizeof(data), 0) == sizeof(data))
							{
								cse4589_print_and_log("[BROADCAST:SUCCESS]\n");
							}
							cse4589_print_and_log("[BROADCAST:END]\n");
							fflush(stdout);
							
						}
						else if((strncmp(msg,"BLOCK",5))==0&&
								loggedin==1)
						{
							char ip[32];
							int k=6,i=0;
							while(msg[k]!='\0')
							{	
								ip[i]=msg[k];
								i++;
								k++;
							}
							ip[i]='\0';
							if((isvalidIP(ip))==1)
							{	
								strcpy(data.cmd,"BLOCK");
								strcpy(data.ip,ip);
								printf("Entered ip is:- %s\n",&ip);
								if(send(server, &data, sizeof(data), 0) == sizeof(data))
								{
									printf("Done!\n");
									cse4589_print_and_log("[BLOCK:SUCCESS]\n");
								}
							}
							else
							{
								cse4589_print_and_log("[BLOCK:ERROR]\n");
							}
							cse4589_print_and_log("[BLOCK:END]\n");
							fflush(stdout);
						}
						else if((strncmp(msg,"UNBLOCK",7))==0&&
								loggedin==1)
						{
							char ip[32];
							int k=8,i=0;
							strcpy(data.cmd,"UNBLOCK");
							while(msg[k]!='\0')
							{	
								ip[i]=msg[k];
								i++;
								k++;
							}
							ip[i]='\0';
							if((isvalidIP(ip))==1)
							{
								strcpy(data.ip,ip);
								printf("Entered ip is:- %s\n",&ip);
								if(send(server, &data, sizeof(data), 0) == sizeof(data))
								{
									cse4589_print_and_log("[UNBLOCK:SUCCESS]\n");
								}
							}
							else
							{
								cse4589_print_and_log("[UNBLOCK:ERROR]\n");
							}
							cse4589_print_and_log("[UNBLOCK:END]\n");
							fflush(stdout);
						}
						else if((strcmp(msg,"LOGOUT"))==0&&
								loggedin==1)
						{
							strcpy(data.cmd,"LOGOUT");
							if(send(server, &data, sizeof(data), 0) == sizeof(data))
							{
								printf("[LOGOUT:SUCCESS]\n");
								loggedin=0;
								int bind_port=bind_the_socket(client_port);
								server=close(server);
							}
							cse4589_print_and_log("[LOGOUT:END]\n");	
						}
						else if((strncmp(msg,"SENDFILE",8))==0&&
								loggedin==1)
						{

						}
						else if((strcmp(msg,"EXIT"))==0)
						{
							close(server);
							cse4589_print_and_log("[EXIT:SUCCESS]\n");
							cse4589_print_and_log("[EXIT:END]\n");
							exit(0);
						}
                    }
                    else
                    {
        				struct server_msg srcv;
        				memset(&srcv, '\0', sizeof(srcv));
	                   
						if(recv(server, &srcv, sizeof(srcv), 0) >= 0)
						{
							if(strcmp(srcv.cmd,"MSG")==0)
							{	// format("msg from:%s\n[msg]:%s\n",client-ip,msg);
								cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
								cse4589_print_and_log("msg from:%s\n[msg]:%s\n",srcv.sender_ip,srcv.info);
								cse4589_print_and_log("[RECEIVED:END]\n");
							}
							else if(strcmp(srcv.cmd,"LIST")==0)
							{	
								cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",srcv.list_row.list_id,srcv.list_row.list_host_name,srcv.list_row.list_ip,srcv.list_row.list_port);
							}
							else if(strcmp(srcv.cmd,"LOGLIST")==0)
							{	
								cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",srcv.list_row.list_id,srcv.list_row.list_host_name,srcv.list_row.list_ip,srcv.list_row.list_port);
							}
							else if(strcmp(srcv.cmd,"LISTOVER")==0)
							{	
								cse4589_print_and_log("[LIST:END]\n");
							}
							else if(strcmp(srcv.cmd,"LOGLISTOVER")==0)
							{	
								cse4589_print_and_log("[LOGIN:END]\n");
							}
							else if(strcmp(srcv.cmd,"MSG_SENT")==0)
							{	
								cse4589_print_and_log("[SEND:SUCCESS]\n");
								cse4589_print_and_log("[SEND:END]\n");
							}
							else if(strcmp(srcv.cmd,"MSG_SENT_FAIL")==0)
							{	cse4589_print_and_log("[SEND:ERROR]\n");
								cse4589_print_and_log("[SEND:END]\n");
							}
							
							fflush(stdout);//there will be no waiting for sending message
						}
                    } 
                    fflush(stdout);	
                }
            }
        }	
	}
}

//server side
void serverSide(int server_port)
{
	printf("\n Inside server side with port %d", server_port);
	int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len, send_socket=0;
	struct server_msg server_data;
	struct list_content send_list;
	/*our defined client message*/
	struct client_msg rcv_data;
    /* Socket address*/
	struct sockaddr_in server_addr, client_addr;

    /*File discriptor list*/
	fd_set master_list, watch_list;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	fdsocket=server_socket;// bcoz fdsocket is used by getmyport function
    if(server_socket < 0)
    {
		perror("Cannot create socket");
    }
    else
    {
    	printf("created socket");
    }

	/* Fill up sockaddr_in struct */
	bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    /* bind*/
    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ){
    	perror("Bind failed");
    }
    else{
    	printf("Binded to socket\n");
    }
    /* Listen */
    if(listen(server_socket, BACKLOG) < 0){
    	perror("Unable to listen on port");
    }
    else{
    	printf("listening to socket\n");
    }
    /*Initializing list*/
    for(int i=0;i<5;i++)
    {
    	list_ptr[i]=(struct list_content *)malloc(sizeof(struct list_content));
    	list_ptr[i]->list_id=0; 
    	client_ptr[i]=(struct list_content *)malloc(sizeof(struct client_block_list));
		client_ptr[i]->C_id=0;
		strcpy(client_ptr[i]->ip1,"null");
		strcpy(client_ptr[i]->ip2,"null");
		strcpy(client_ptr[i]->ip3,"null");
		strcpy(client_ptr[i]->ip4,"null");
    }

    /* Zero select FD sets */
    FD_ZERO(&master_list);//Initializes the file descriptor set fdset to have zero bits for all file descriptors. 
    FD_ZERO(&watch_list);
    
    /* Register the listening socket */
    FD_SET(server_socket, &master_list);//FD stands for file discriptor
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while(TRUE)
    {
fflush(stdout);	
        memcpy(&watch_list, &master_list, sizeof(master_list));

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
        {
            perror("select failed.");
            exit(1);
        }

        if(selret > 0)
        {
        	 for(sock_index=0; sock_index<=head_socket; sock_index+=1)
            {
                fflush(stdout);
                memset(&server_data, '\0', sizeof(server_data));//mera
                if(FD_ISSET(sock_index, &watch_list))
                {
	        		/* Check if new command on STDIN */
	                if (sock_index == STDIN)
	                {
	                	char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
	                	memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);

						//printf("\nI got: %s\n", cmd);
						int len=strlen(cmd);
						cmd[len-1]='\0';// to remove \n from the msg
						
						//Process PA1 commands here ...
						if((strcmp(cmd, "AUTHOR"))==0)
						{
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, vchincho, have read and understood the course academic integrity policy.\n");
							cse4589_print_and_log("[AUTHOR:END]\n");
						}
	                    else if((strcmp(cmd, "IP"))==0)
	                    {
							showIP();
							cse4589_print_and_log("[IP:END]\n");
	                    }
	                    else if((strcmp(cmd, "PORT"))==0)
	                    {
							getmyport();
							cse4589_print_and_log("[PORT:END]\n");
	                    }
	                    else if((strcmp(cmd, "LIST"))==0)
	                    {
	                    	sort_list_port();
	                    	cse4589_print_and_log("[LIST:SUCCESS]\n");
	                        print_list();
	                        cse4589_print_and_log("[LIST:END]\n");
	                    }
	                    else if((strcmp(cmd, "STATISTICS"))==0)
	                    {

	                    	cse4589_print_and_log("[STATISTICS:SUCCESS]\n");
	                        print_stats();
	                        cse4589_print_and_log("[STATISTICS:END]\n");
	                    }
	                    else if((strncmp(cmd, "BLOCKED",7))==0)
	                    {
							char ip[32];
							int k=8,i=0;
							while(cmd[k]!='\0')
							{	
								ip[i]=cmd[k];
								i++;
								k++;
							}
							ip[i]='\0';
							printf("Entered IP is :-%s",ip);
							
							if((isvalidIP(ip))==1)
							{
								cse4589_print_and_log("[BLOCKED:SUCCESS]\n");
								int o=0;
								while(client_ptr[o]->C_id!=0)
								{
									if(strcmp(ip,client_ptr[o]->C_ip)==0)
									{
										break;	
									}
									o++;
								}	
							int q=1;
								//now print the information of blocked ips from listptr
								if(strcmp(client_ptr[o]->ip1,"null")!=0)
								{
									for(int i=0;i<5;i++)
									{
										if((strcmp(list_ptr[i]->list_ip,client_ptr[o]->ip1)) == 0)
										{	
											cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", &q, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port);
											break;
										}									                   	
									}	
								}
								q=q+1;
								if(strcmp(client_ptr[o]->ip2,"null")!=0)
								{
									for(int i=0;i<5;i++)
									{
										if((strcmp(list_ptr[i]->list_ip,client_ptr[o]->ip2)) == 0)
										{	
											cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", &q, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port);
											break;
										}									                   	
									}	
								}
								q=q+1;
								if(strcmp(client_ptr[o]->ip3,"null")!=0)
								{
									for(int i=0;i<5;i++)
									{
										if((strcmp(list_ptr[i]->list_ip,client_ptr[o]->ip3)) == 0)
										{	
											cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", &q, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port);
											break;
										}									                   	
									}	
								}
								q=q+1;
								if(strcmp(client_ptr[o]->ip4,"null")!=0)
								{
									for(int i=0;i<5;i++)
									{
										if((strcmp(list_ptr[i]->list_ip,client_ptr[o]->ip4)) == 0)
										{	
											cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", &q, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port);
											break;
										}									                   	
									}	
								}

							}
							else
							{
								cse4589_print_and_log("[BLOCKED:ERROR]\n");
							}
							cse4589_print_and_log("[BLOCKED:END]\n");
							//end of blocked				
	                    }
						free(cmd);
	                }
	                /* Check if new client is requesting connection */
	                else if(sock_index == server_socket)
	                {
	                    caddr_len = sizeof(client_addr);
	                    fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);// on success accept returns an integer, that is the file descriptor for the accepted socket.
	                    printf("\n fdaccept index value:-%d",fdaccept);

	                    if(fdaccept < 0)
	                    {
	                        perror("Accept failed.");
	                    }
	                    char ip[INET_ADDRSTRLEN];
	                    inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip, INET_ADDRSTRLEN);

						printf("\nRemote Host connected! with IP:-%s \n", ip);                        
						ntohs(client_addr.sin_port);
	                    /* Add to watched socket list */
	                    FD_SET(fdaccept, &master_list);
	                    if(fdaccept > head_socket) 
                        {
                            head_socket = fdaccept;
                        }
	                    char host[1024];
	                    getnameinfo((struct sockaddr *)&client_addr, caddr_len,host, sizeof(host), 0,0,0);
	                    printf("\nhost name is:-%s", host);
	                    printf("\nPort number is:-%d", ntohs(client_addr.sin_port));
	                    /*Add blocking information into the available structure*/
	                    int n=0;
						while((client_ptr[n]->C_id)!=0)
                        {
                        	n++;
                        }
						client_ptr[n]->C_id=n+1;
						strcpy(client_ptr[n]->C_ip,ip);
                     	/*Add new client information to the list*/
                        int m=0;
                        while((list_ptr[m]->list_id)!=0)
                        {
                        	m++;
                        }
                        list_ptr[m]->list_id=m+1;
                        list_ptr[m]->list_port=ntohs(client_addr.sin_port);
                        list_ptr[m]->fd_socket=fdaccept;
                        list_ptr[m]->snd_msg=0;
                        list_ptr[m]->rcv_msg=0;
                        strcpy(list_ptr[m]->state,"logged-in");
                        strcpy(list_ptr[m]->list_ip,ip);
                        strcpy(list_ptr[m]->list_host_name,host);

                        //send list of currently loged in clients to the client
                        sort_list_port();
                        /*for(int i=0;i<5;i++)
						{
							if(list_ptr[i]->list_id!=0)
							{	
								printf("%-5d%-35s%-20s%-8d\n", list_ptr[i]->list_id, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port,list_ptr[i]->fd_socket);
								
								send_list.list_id=list_ptr[i]->list_id;
								strcpy(send_list.list_host_name,list_ptr[i]->list_host_name);
								strcpy(send_list.list_ip,list_ptr[i]->list_ip);
								send_list.list_port=list_ptr[i]->list_port;

								strcpy(server_data.cmd,"LIST");
								server_data.list_row=send_list;
								if(send(fdaccept, &server_data, sizeof(server_data), 0) == sizeof(server_data))
	                            {
									printf("Done!\n");
	                            }
								fflush(stdout);	
							}
						}*/
						//INFORMING THAT LOGIN IS OVER
						strcpy(server_data.cmd,"LOGLISTOVER");
						if(send(fdaccept, &server_data, sizeof(server_data), 0) == sizeof(server_data))
	                            {
	                     
									printf("Done!\n");
	                            }
							fflush(stdout);	
						//end 
	                }
	                else
	                {
	                    /* Initialize buffer to receieve response */
	                	memset(&rcv_data, '\0', sizeof(rcv_data));

	                    if(recv(sock_index, &rcv_data, sizeof(rcv_data), 0) <= 0)// recv returns length of the message on successful completion.
	                    {
	                    	/*Remove from the contetn_list as well*/
	                    	remove_from_list(sock_index);
	                        //close(sock_index);
	                        printf("Remote Host terminated connection!\n");
	                        /* Remove from watched list */
	                        FD_CLR(sock_index,&master_list);
	                    }
	                    else 
	                    {
	                    	//Process incoming data from existing clients here ...
	                    	if((strcmp(rcv_data.cmd,"SEND"))==0)
	                    	{
	                    		char sender_ip[32];
								char receivers_ip[32];

								//search list for senders ip based on current socket
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->fd_socket==sock_index)
									{	
										strcpy(sender_ip,list_ptr[i]->list_ip);
										list_ptr[i]->snd_msg+=1;
										break;
									}
									
								}
								//search list for receivers socket based on ip sent by the sender
								for(int i=0;i<5;i++)
								{
									if((strcmp(list_ptr[i]->list_ip,rcv_data.ip)) == 0)
									{	
										send_socket=list_ptr[i]->fd_socket;
										list_ptr[i]->rcv_msg+=1;
										break;
									}									                   	
								
								}
								//put rcv.ip into receivers ip
								strcpy(receivers_ip,rcv_data.ip);
								// CHECK FOR BLOCKED IPS
								int p=0;
								int flag=1;

								while(client_ptr[p]->C_id!=0)
								{
									if(strcmp(sender_ip,client_ptr[p]->C_ip)==0)
									{
										break;	
									}
									p++;
								}

								if(strcmp(client_ptr[p]->ip1,receivers_ip)==0)
								{
									flag=0;
								}
								if(strcmp(client_ptr[p]->ip2,receivers_ip)==0)
								{
									flag=0;
								}
								if(strcmp(client_ptr[p]->ip3,receivers_ip)==0)
								{
									flag=0;	
								}
								if(strcmp(client_ptr[p]->ip4,receivers_ip)==0)
								{
									flag=0;
								}
								if(flag==1)
								{		
									strcpy(server_data.cmd,"MSG");
									strcpy(server_data.sender_ip,sender_ip);
									strcpy(server_data.info,rcv_data.info);

									if(send(send_socket, &server_data, sizeof(server_data), 0) == sizeof(server_data))
			                        {
										cse4589_print_and_log("[RELAYED:SUCCESS]\n");
										cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",sender_ip, receivers_ip, rcv_data.info);				 
										//send success message to sender that msg has been sent
										strcpy(server_data.cmd,"MSG_SENT");
				                        if(send(sock_index, &server_data, sizeof(server_data), 0) == sizeof(server_data))
				                        {
				                        	printf("Done!\n");	
				                        }
			                        }
			                        else
			                        {
			                        	cse4589_print_and_log("[RELAYED:ERROR]\n");
			                        	strcpy(server_data.cmd,"MSG_SENT_FAIL");
				                        if(send(sock_index, &server_data, sizeof(server_data), 0) == sizeof(server_data))
				                        {

				                        	printf("Done!\n");	
				                        }
			                        }
			                        cse4589_print_and_log("[RELAYED:END]\n");
			                    }
			                    else
			                    {
			                    	printf("this client blocked this ip");
			                    	cse4589_print_and_log("[RELAYED:SUCCESS]\n");
									cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",sender_ip, receivers_ip, rcv_data.info);
			                    	strcpy(server_data.cmd,"MSG_SENT");
			                        if(send(sock_index, &server_data, sizeof(server_data), 0) == sizeof(server_data))
			                        {
			                        	printf("\nsent msg to sender that msg has been delivered!\n");	
			                        }
			                        cse4589_print_and_log("[RELAYED:END]\n");
			                    }
			                   

			                   

	                    	}
	                    	else if((strcmp(rcv_data.cmd,"BROADCAST"))==0)
	                    	{
		                    	char sender_ip[32];

								//search list for senders ip based on current socket
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->fd_socket==sock_index)
									{	
										strcpy(sender_ip,list_ptr[i]->list_ip);
										list_ptr[i]->snd_msg+=1;
										break;
									}
									
								}
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->list_id!=0 &&
									(strcmp(list_ptr[i]->list_ip,sender_ip))!=0)
									{
										send_socket=list_ptr[i]->fd_socket;
										strcpy(server_data.cmd,"MSG");
										strcpy(server_data.sender_ip,sender_ip);
										strcpy(server_data.info,rcv_data.info);
										if(send(send_socket, &server_data, sizeof(server_data), 0) == sizeof(server_data))
		                        		{
		                        			cse4589_print_and_log("[RELAYED:SUCCESS]\n");
											cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n",sender_ip, rcv_data.info);
		                        		}
		                        		else
		                        		{
		                        			cse4589_print_and_log("[RELAYED:ERROR]\n");
		                        		}	
		                        		cse4589_print_and_log("[RELAYED:END]\n");	
									}									
								}	
													
	                    	}
	                    	else if((strcmp(rcv_data.cmd,"BLOCK"))==0)
	                    	{
	                    		printf("\nclient block received");
	                    		//
	                    		char sender_ip[32];
								//search list for senders ip based on current socket
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->fd_socket==sock_index)
									{	
										strcpy(sender_ip,list_ptr[i]->list_ip);
										break;
									}	
								}
								int z=0;
								printf("\ninside BLOCK SENDER IP:-%s",sender_ip);

								while((client_ptr[z]->C_id)!=0)
					            {
					            	printf("\ninside server  block");
							        if(strcmp(rcv_data.ip,client_ptr[z]->C_ip)==0)
									{
										break;
									}
									z++;
					         	}
					         	printf("\n CURRENT Z VALUE IS %d",&z);
					         	printf("\n CURRENT IP1 VALUE IS %s",client_ptr[z]->ip1);

								if(strcmp(client_ptr[z]->ip1,"null")==0)
								{
									strcpy(client_ptr[z]->ip1,sender_ip);
								}
								else if(strcmp(client_ptr[z]->ip2,"null")==0)
								{
									strcpy(client_ptr[z]->ip2,sender_ip);
								}
								else if(strcmp(client_ptr[z]->ip3,"null")==0)
								{
									strcpy(client_ptr[z]->ip3,sender_ip);
								}
								else if(strcmp(client_ptr[z]->ip4,"null")==0)
								{
									strcpy(client_ptr[z]->ip4,sender_ip);
								}
	                    		//
	                    	}
	                    	else if((strcmp(rcv_data.cmd,"UNBLOCK"))==0)
	                    	{
	                    		char sender_ip[32];
								//search list for senders ip based on current socket
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->fd_socket==sock_index)
									{	
										strcpy(sender_ip,list_ptr[i]->list_ip);
										break;
									}	
								}
								int z=0;
								while((client_ptr[z]->C_id)!=0)
					            {
					            	printf("\ninside server  block");
							        if(strcmp(rcv_data.ip,client_ptr[z]->C_ip)==0)
									{
										break;
									}
									z++;
					         	}
					         	if(strcmp(client_ptr[z]->ip1,sender_ip)==0)
								{
									strcpy(client_ptr[z]->ip1,"null");
								}
								else if(strcmp(client_ptr[z]->ip2,sender_ip)==0)
								{
									strcpy(client_ptr[z]->ip2,"null");
								}
								else if(strcmp(client_ptr[z]->ip3,sender_ip)==0)
								{
									strcpy(client_ptr[z]->ip3,"null");
								}
								else if(strcmp(client_ptr[z]->ip4,sender_ip)==0)
								{
									strcpy(client_ptr[z]->ip4,"null");
								}	
	                    	}
	                    	else if((strcmp(rcv_data.cmd,"LIST"))==0)
	                    	{
	                    		for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->list_id!=0)
									{	
										printf("%-5d%-35s%-20s%-8d\n", list_ptr[i]->list_id, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port,list_ptr[i]->fd_socket);
										
										send_list.list_id=list_ptr[i]->list_id;
										strcpy(send_list.list_host_name,list_ptr[i]->list_host_name);
										strcpy(send_list.list_ip,list_ptr[i]->list_ip);
										send_list.list_port=list_ptr[i]->list_port;

										strcpy(server_data.cmd,"LIST");
										server_data.list_row=send_list;
										if(send(sock_index, &server_data, sizeof(server_data), 0) == sizeof(server_data))
			                            {
											printf("Done!\n");
			                            }	
									}
								}
								//informing that the list is over
								strcpy(server_data.cmd,"LISTOVER");
								if(send(sock_index, &server_data, sizeof(server_data), 0) == sizeof(server_data))
	                            {
									printf("Done!\n");
	                            }
	                            fflush(stdout);	
								
	                    	}
	                    	else if((strcmp(rcv_data.cmd,"LOGOUT"))==0)
	                    	{
	                    		//search list for senders ip based on current socket
								for(int i=0;i<5;i++)
								{
									if(list_ptr[i]->fd_socket==sock_index)
									{	
										strcpy(list_ptr[i]->state,"logged-out");
										close(sock_index);
										FD_CLR(sock_index, &master_list);
										sort_list_port();
									}	
								}
	                    	}
							fflush(stdout);

	                    }
	             	}   
	         	}
            }
        }
    }
}

int bind_the_socket(int c_port)
{
	struct sockaddr_in my_addrs;
	fdsocket = socket(AF_INET, SOCK_STREAM, 0);// return socket file descriptor
    if(fdsocket < 0)
    {
       perror("Failed to create socket");
       return 0;
    }

    //setting up client socket
    my_addrs.sin_family=AF_INET;
    my_addrs.sin_addr.s_addr=INADDR_ANY;
    my_addrs.sin_port=htons(c_port);
    int optval=1;
    setsockopt(fdsocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if(bind(fdsocket, (struct  sockaddr*) &my_addrs, sizeof(struct sockaddr_in)) == 0)
    {
    	printf("\nclient binded to port correctly\n");
    	return 1;
    }
    else
    {
    	printf("\nError in binding client port\n");
    	return 0;
    }
}
int connect_to_host(char *server_ip, int server_port, int c_port)
{
    int len;
    struct sockaddr_in remote_server_addr;

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);//inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    remote_server_addr.sin_port = htons(server_port);//function converts the unsigned short integer hostshort from host byte order to network byte order.

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
    {
        perror("Connect failed");
    }
    else{
    	printf("\nLogged in\n");
    }
    return fdsocket;
}

void sort_list_port()
{
	for(int i=0;i<5;i++)
	{
		for(int j=0;j<5-i-1;j++)
		{
			if(list_ptr[j]->list_port> list_ptr[j+1]->list_port && list_ptr[j+1]->list_id!=0)
			{
				//TEMP VARIABLES
				int tlist_id;
				char tlist_host_name[40];
				char tlist_ip[32];
				int tlist_port;
				int tfd_socket;
				int trcv_msg;
				int tsnd_msg;
				char tstate[20];
				
				//COPING TO TEMP VARIABLES
				strcpy(tlist_host_name,list_ptr[j]->list_host_name);
				strcpy(tlist_ip,list_ptr[j]->list_ip);
				tlist_port=list_ptr[j]->list_port;
				tfd_socket=list_ptr[j]->fd_socket;
				trcv_msg=list_ptr[j]->rcv_msg;
				tsnd_msg=list_ptr[j]->snd_msg;
				strcpy(tstate,list_ptr[j]->state);
				//coping j+1 into j
				strcpy(list_ptr[j]->list_host_name,list_ptr[j+1]->list_host_name);
				strcpy(list_ptr[j]->list_ip,list_ptr[j+1]->list_ip);
				list_ptr[j]->list_port=list_ptr[j+1]->list_port;
				list_ptr[j]->fd_socket=list_ptr[j+1]->fd_socket;
				list_ptr[j]->rcv_msg=list_ptr[j+1]->rcv_msg;
				list_ptr[j]->snd_msg=list_ptr[j+1]->snd_msg;
				strcpy(list_ptr[j]->state,list_ptr[j+1]->state);
				
				// coping TEMP into j+1;
				strcpy(list_ptr[j+1]->list_host_name,tlist_host_name);
				strcpy(list_ptr[j+1]->list_ip,tlist_ip);
				list_ptr[j+1]->list_port=tlist_port;
				list_ptr[j+1]->fd_socket=tfd_socket;
				list_ptr[j+1]->rcv_msg=trcv_msg;
				list_ptr[j+1]->snd_msg=tsnd_msg;
				strcpy(list_ptr[j+1]->state,tstate);
			}
		}
	}

}
void remove_from_list(int sock_delete)
{
	int delete=0;
	int last=0;
	for(int i=0;i<5;i++)
	{
		if(list_ptr[i]->fd_socket==sock_delete)
		{	
			delete=i;
			break;
		}
	}
	for(int i=delete;i<5;i++)
	{last=last+1;
		if(list_ptr[i+1]->list_id!=0)
		{
			strcpy(list_ptr[i]->list_host_name,list_ptr[i+1]->list_host_name);
			strcpy(list_ptr[i]->list_ip,list_ptr[i+1]->list_ip);
			list_ptr[i]->list_port=list_ptr[i+1]->list_port;
			list_ptr[i]->fd_socket=list_ptr[i+1]->fd_socket;
			list_ptr[i]->rcv_msg=list_ptr[i+1]->rcv_msg;
			list_ptr[i]->snd_msg=list_ptr[i+1]->snd_msg;
			strcpy(list_ptr[i]->state,list_ptr[i+1]->state);
		}	

	}
	list_ptr[last]->list_id=0;

}
void print_list()
{
	for(int i=0;i<5;i++)
	{
		if(list_ptr[i]->list_id!=0)
		{	
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_ptr[i]->list_id, list_ptr[i]->list_host_name, list_ptr[i]->list_ip, list_ptr[i]->list_port);
		}
	}
}
void print_stats()
{
	for(int i=0;i<5;i++)
	{
		if(list_ptr[i]->list_id!=0)
		{	
			cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_ptr[i]->list_id, list_ptr[i]->list_host_name, list_ptr[i]->snd_msg, list_ptr[i]->rcv_msg,list_ptr[i]->state);
		}
	}
}
void getmyport()
{
	struct sockaddr_in portvalue;
	socklen_t len=sizeof(portvalue);
	if(getsockname(fdsocket,(struct sockaddr *)&portvalue, &len) == -1)
	{
		cse4589_print_and_log("[PORT:ERROR]\n");
	}
	else{
		cse4589_print_and_log("[PORT:SUCCESS]\n");
		cse4589_print_and_log("PORT:%d\n", ntohs(portvalue.sin_port));
	}
}

int isvalidIP(char *ip)
{
	struct sockaddr_in temp;
	int result= inet_pton(AF_INET, ip, &temp.sin_addr);
	if(result==1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void showIP()
{
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;
     
    struct sockaddr_in serv;
     
    int sock = socket ( AF_INET, SOCK_DGRAM, 0);
     
    if(sock < 0)
    {
        perror("Socket error");
    }
     
    memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr( google_dns_server );
    serv.sin_port = htons( dns_port );
 
    int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );
     
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
         
    char buffer[100];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
         
    if(p != NULL)
    {
    	cse4589_print_and_log("[IP:SUCCESS]\n");
        cse4589_print_and_log("IP:%s\n",buffer);
    }
    else
    {
    	cse4589_print_and_log("[IP:ERROR]\n");

    }
    close(sock);
}

