/**
 * @vchincho_assignment3
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

#include "../include/global.h"
#include "../include/init.h"

#include "../include/connection_manager.h"
#include <stdio.h>
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Start Here*/
    sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
    //printf("Running router");
routerID = (uint16_t*)malloc(sizeof(uint16_t)*router_count);    
 routerPort = (uint16_t*)malloc(sizeof(uint16_t)*router_count);
    dataPort = (uint16_t*)malloc(sizeof(uint16_t)*router_count);
    cost = (uint16_t*)malloc(sizeof(uint16_t)*router_count);
    nextHop = (uint16_t*)malloc(sizeof(uint16_t)*router_count);
    destIp = (uint32_t*)malloc(sizeof(uint32_t)*router_count);
   
 init(); // Initialize connection manager; This will block
	return 0;
}
