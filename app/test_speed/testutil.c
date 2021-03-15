#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "timer.h"
#include "riffa.h"

int main(int argc, char** argv) {
	fpga_t * fpga;
	fpga_info_list info;
	int option;
	int i;
	int id;
	int chnl;
	size_t numWords;
	int sent;
	int recvd;
	unsigned int * sendBuffer;
	unsigned int * recvBuffer;
	GET_TIME_INIT(3);

	if (argc < 2) {
		printf("Usage: %s <option>\n", argv[0]);
		return -1;
	}

	option = atoi(argv[1]);

	if (option == 0) {	// List FPGA info
		// Populate the fpga_info_list struct
		if (fpga_list(&info) != 0) {
			printf("Error populating fpga_info_list\n");
			return -1;
		}
		printf("Number of devices: %d\n", info.num_fpgas);
		for (i = 0; i < info.num_fpgas; i++) {
			printf("%d: id:%d\n", i, info.id[i]);
			printf("%d: num_chnls:%d\n", i, info.num_chnls[i]);
			printf("%d: name:%s\n", i, info.name[i]);
			printf("%d: vendor id:%04X\n", i, info.vendor_id[i]);
			printf("%d: device id:%04X\n", i, info.device_id[i]);
		}
	}
	else if (option == 1) { // Reset FPGA
		if (argc < 3) {
			printf("Usage: %s %d <fpga id>\n", argv[0], option);
			return -1;
		}

		id = atoi(argv[2]);

		// Get the device with id
		fpga = fpga_open(id);
		if (fpga == NULL) {
			printf("Could not get FPGA %d\n", id);
			return -1;
	    }

		// Reset
		fpga_reset(fpga);

		// Done with device
        fpga_close(fpga);
	}
	else if (option == 2) { // Send data, receive data
		if (argc < 5) {
			printf("Usage: %s %d <fpga id> <chnl> <num words to transfer>\n", argv[0], option);
			return -1;
		}

		id = atoi(argv[2]);
		chnl = atoi(argv[3]);
		numWords = atoi(argv[4]);

		// Get the device with id
		fpga = fpga_open(id);
		if (fpga == NULL) {
			printf("Could not get FPGA %d\n", id);
			return -1;
	    }

		// Malloc the arrays
		sendBuffer = (unsigned int *)malloc(numWords<<2);
		if (sendBuffer == NULL) {
			printf("Could not malloc memory for sendBuffer\n");
			fpga_close(fpga);
			return -1;
	    }
		recvBuffer = (unsigned int *)malloc(numWords<<2);
		if (recvBuffer == NULL) {
			printf("Could not malloc memory for recvBuffer\n");
			free(sendBuffer);
			fpga_close(fpga);
			return -1;
	    }

		// Initialize the data
		for (i = 0; i < numWords; i++) {
			sendBuffer[i] = i+1;
			recvBuffer[i] = 0;
		}

		GET_TIME_VAL(0);

		// Send the data
		sent = fpga_send(fpga, chnl, sendBuffer, numWords, 0, 1, 25000);
		printf("words sent: %d\n", sent);

		GET_TIME_VAL(1);

		if (sent != 0) {
			// Recv the data
			recvd = fpga_recv(fpga, chnl, recvBuffer, numWords, 25000);
			printf("words recv: %d\n", recvd);
		}

		GET_TIME_VAL(2);

		// Done with device
        fpga_close(fpga);

		// Display some data
        for (i = 0; i < 128; i++) {
			printf("recvBuffer[%d]: %d\n", i, recvBuffer[i]);
		}

		// Check the data
		if (recvd != 0) {
			for (i = 4; i < recvd; i++) {
				if (recvBuffer[i] != sendBuffer[i]) {
					printf("recvBuffer[%d]: %d, expected %d\n", i, recvBuffer[i], sendBuffer[i]);
					break;
				}
			}

			printf("send bw: %f MB/s %fms\n",
				sent*4.0/1024/1024/((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0))/1000.0),
				(TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) );

			printf("recv bw: %f MB/s %fms\n",
				recvd*4.0/1024/1024/((TIME_VAL_TO_MS(2) - TIME_VAL_TO_MS(1))/1000.0),
				(TIME_VAL_TO_MS(2) - TIME_VAL_TO_MS(1)) );
		}
	}

	return 0;
}
