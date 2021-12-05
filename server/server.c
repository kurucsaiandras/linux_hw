#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <errno.h>



int main(int argc , char *argv[])
{
	//port number for socket server
	int port_number;

	//variable for storing bytes from the client
    uint8_t intbuf[64];
	
    //store command in 2d array (8 bytes each)
    unsigned char cmd[8][8];

    //socket related variables
	int socket_desc , client_sock , address_len , read_size;
	struct sockaddr_in server , client;

    //file variable for opening the device file
	FILE *device_file;

	if(argc < 2){
		printf("\nNot enough parameters for startup. Run server like: ./server PORTNUMBER\n");
		return -1;
	}

	//create the socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Failed to create socket! Error is: %s\n", strerror(errno));
        return 0;
	}
	sscanf(argv[1], "%d", &port_number);
	printf("\nServer started on: %d\n", port_number);
	
	//set up sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port_number);
	
	//binding
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
        printf("Failed to bind socket! Error is: %s\n", strerror(errno));
		return 1;
	}
	//command variable from terminal
	char  terminal_command [20];
	
	//listen
	listen(socket_desc , 3);

	
	
	//wait for connection
	printf("Waiting for clients to connect\n");
	address_len = sizeof(struct sockaddr_in);
	
	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&address_len);
	
	//if failed to accept a connection
	if (client_sock < 0)
	{
		printf("Failed to accept connection! Error is: %s\n", strerror(errno));
		return 1;
	}
	printf("Connection accepted\n");
	
	//receive bytes from the client
	while( (read_size = recv(client_sock, &intbuf, 64, 0)) > 0 )
	{
		if(read_size == -1)
	{
		printf("Failed to receive bytes\n");
	}else if (read_size == 64){ 
		printf("Received screen data: \n");

		//storing the bytes in the cmd 2d array
		for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            cmd[i][j] = 0xFF & intbuf[i*8+j];
            printf("%d|", intbuf[i*8+j]);

        }
			printf("\n");
		}

		//write the bytes to the device file
    	device_file = fopen("/dev/max7219x4", "ab");
        fwrite( cmd, 1, 64, device_file );
        fclose(device_file);

    }else{
		printf("\nServer got less or more bytes than expected.\n");
	}
		
	}
	
	if(read_size == 0)
	{
		printf("Client disconnected\n");
		close(client_sock);

		//clear stdout
		fflush(stdout);
	}
	//close the socket
	close(client_sock);
	
	
	return 0;
}