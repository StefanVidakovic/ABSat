/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 GomSpace ApS (http://www.gomspace.com)
Copyright (C) 2012 AAUSAT3 Project (http://aausat3.space.aau.dk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
//#include "FreeRTOS.h"
//#include "task.h"
//#include "csp\csp_types.h"
//#include "csp\csp_platform.h"
//#include "csp\csp_error.h"
//#include <csp\csp_debug.h>
//#include "csp\csp_buffer.h"
//#include <csp\csp_rtable.h>
//#include <csp\csp_iflist.h>
//#include "csp\csp_types.h"
///* SiLabs includes. */
//#include "em_chip.h"
//#include "bsp.h"
//#include "bsp_trace.h"
//#include "sleep.h"
//#include <stdio.h>
//#include <string.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <stdlib.h>
//
//#include "FreeRTOSConfig.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"
//#include "croutine.h"
//#include "em_device.h"
//#include "em_chip.h"
//#include "em_i2c.h"
//#include "em_rtc.h"
//#include "em_cmu.h"
//#include "em_emu.h"
//#include "em_gpio.h"
//#include "em_chip.h"
//#include "bsp.h"
//#include "bsp_trace.h"
#include "makePrintfWork.h"
//#include "sleep.h"
//#include "csp\csp.h"
//
///* Using un-exported header file.
// * This is allowed since we are still in libcsp */
//#include "csp\arch\csp_thread.h"

/** Example defines */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <csp/csp.h>

/* Using un-exported header file.
 * This is allowed since we are still in libcsp */
#include <csp/arch/csp_thread.h>
#define MY_ADDRESS  1			// Address of local CSP node
#define MY_PORT		10			// Port to send test traffic to

CSP_DEFINE_TASK(task_server) {

	/* Create socket without any socket options */
	csp_socket_t *sock = csp_socket(CSP_SO_NONE);

	/* Bind all ports to socket */
	csp_bind(sock, CSP_ANY);

	/* Create 10 connections backlog queue */
	csp_listen(sock, 10);

	/* Pointer to current connection and packet */
	csp_conn_t *conn;
	csp_packet_t *packet;

	/* Process incoming connections */
	while (1) {

		/* Wait for connection, 10000 ms timeout */
		if ((conn = csp_accept(sock, 10000)) == NULL)
			continue;

		/* Read packets. Timout is 100 ms */
		while ((packet = csp_read(conn, 100)) != NULL) {
			switch (csp_conn_dport(conn)) {
			case MY_PORT:
				/* Process packet here */
				printf("Packet received on MY_PORT: %s\r\n", (char *) packet->data);
				csp_buffer_free(packet);
				break;

			default:
				/* Let the service handler reply pings, buffer use, etc. */
				csp_service_handler(conn, packet);
				break;
			}
		}

		/* Close current connection, and handle next */
		csp_close(conn);

	}

	return CSP_TASK_RETURN;

}

CSP_DEFINE_TASK(task_client) {

	csp_packet_t * packet;
	csp_conn_t * conn;

	while (1) {

		/**
		 * Try ping
		 */

		csp_sleep_ms(1000);

		int result = csp_ping(MY_ADDRESS, 100, 100, CSP_O_NONE);
		puts("Ping result %d [ms]\r\n");

		csp_sleep_ms(1000);

		/**
		 * Try data packet to server
		 */

		/* Get packet buffer for data */
		packet = csp_buffer_get(100);
		if (packet == NULL) {
			/* Could not get buffer element */
			printf("Failed to get buffer element\n");
			return CSP_TASK_RETURN;
		}

		/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
		conn = csp_connect(2, MY_ADDRESS, MY_PORT, 1000, CSP_O_NONE);
		if (conn == NULL) {
			/* Connect failed */
			printf("Connection failed\n");
			/* Remember to free packet buffer */
			csp_buffer_free(packet);
			return CSP_TASK_RETURN;
		}

		/* Copy dummy data to packet */
		char *msg = "Hello World";
		strcpy((char *) packet->data, msg);

		/* Set packet length */
		packet->length = strlen(msg);

		/* Send packet */
		if (!csp_send(conn, packet, 1000)) {
			/* Send failed */
			printf("Send failed\n");
			csp_buffer_free(packet);
		}

		/* Close connection */
		csp_close(conn);

	}

	return CSP_TASK_RETURN;
}

int main(int argc, char * argv[]) {
	SWO_SetupForPrint();
//CHIP_Init();
	puts("dfg");

	puts("main");
	/**
	 * Initialise CSP,
	 * No physical interfaces are initialised in this example,
	 * so only the loopback interface is registered.
	 */

	/* Init buffer system with 10 packets of maximum 300 bytes each */
	puts("Initializing CSP\r\n");
	csp_buffer_init(5, 300);

	/* Init CSP with address MY_ADDRESS */
	csp_init(MY_ADDRESS);
puts("just initialized csp");
	/* Start router task with 500 word stack, OS task priority 1 */
	csp_route_start_task(500, 1);
puts("just initialized the router task.");
	/* Enable debug output from CSP */
	if ((argc > 1) && (strcmp(argv[1], "-v") == 0)) {
		puts("Debug enabed\r\n");
		csp_debug_toggle_level(3);
		csp_debug_toggle_level(4);

		puts("Conn table\r\n");
		csp_conn_print_table();

		puts("Route table\r\n");
		csp_route_print_table();

		puts("Interfaces\r\n");
		csp_route_print_interfaces();

	}

	/**
	 * Initialise example threads, using pthreads.
	 */

	/* Server */
	puts("Starting Server task\r\n");
	csp_thread_handle_t handle_server;
//	csp_thread_return_t task_server;
	csp_thread_create(task_server, "SERVER", 1000, NULL, 0, &handle_server);

	/* Client */
	puts("Starting Client task\r\n");
	csp_thread_handle_t handle_client;
	csp_thread_create(task_client, "SERVER", 1000, NULL, 0, &handle_client);

	/* Wait for execution to end (ctrl+c) */
	while(1) {
		csp_sleep_ms(100000);
	}

	return 0;

}
