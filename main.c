#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX_LEN_RECV 1024 // I'm not sure what should we use
#define MIN_PORT_VAL 1024
#define MAX_PORT_VAL 64000
#define BACKLOG 10

int ranged_rand(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

char* read_server_message(int server_socket){
    int counter = 0, i = 0;
    char *returned_string, *temp_returned;

    recv(server_socket, temp_returned, MAX_LEN_RECV, 0);
    strcat(returned_string,temp_returned);

    while(counter < 2){
        if (strcmp(temp_returned + i, '\r\n\r\n') == 0)
            counter++;
        if (temp_returned[i] == '\0') {
            recv(server_socket, temp_returned, MAX_LEN_RECV, 0);
            strcat(returned_string,temp_returned);
        }
        i++;
    }

    return returned_string;
}

int main() {
    int port, servers_array[3], client_connected; // the sockets returned from accept
    int addr_len = sizeof(struct sockaddr_in);
    char buff[1024], *full_request, *returned_string;
    FILE* server_port_number = fopen('server_port.txt', 'w');
    FILE* client_port_number = fopen('http_port.txt', 'w');

    //create a new socket for servers and for client
    int sock_client = socket(AF_INET, SOCK_STREAM, 0);
    int sock_server = socket(AF_INET, SOCK_STREAM, 0);

    //create sockaddr variable with random porn number
    port = ranged_rand(MIN_PORT_VAL, MAX_PORT_VAL);
    struct sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    service.sin_port = htons(port);

    //bind sever socket, keep trying until success
    while (bind(sock_server, (struct sockaddr*)&service, sizeof(service)) == -1) {
        port = ranged_rand(MIN_PORT_VAL, MAX_PORT_VAL);
        service.sin_port = htons(port);
    }
    if (listen(sock_server, BACKLOG) != -1)
        fprintf(server_port_number, "%d", port);


    //bind client socket, keep trying until success
    do {
        port = ranged_rand(MIN_PORT_VAL, MAX_PORT_VAL);
        service.sin_port = htons(port);
    } while (bind(sock_client, (struct sockaddr*)&service, sizeof(service)) == -1);
    if (listen(sock_client, BACKLOG) != -1)
        fprintf(client_port_number, "%d", port);

    // Waiting till 3 servers are connected
    int i = 0;
    while (i<3){
        servers_array[i] = accept(sock_server, (struct sockaddr*)&service, &addr_len);
        if(servers_array[i] > 0) {
            i++;
            listen(sock_server, BACKLOG);
        }
        else{
            printf("connection failed, try again");
        }
    }

    // Accepting http connection
    client_connected = accept(sock_client, (struct sockaddr *) &service, &addr_len);
    while(client_connected <= 0) { // failed to accept, try again
        listen(sock_client, BACKLOG);
        client_connected = accept(sock_client, (struct sockaddr *) &service, &addr_len);
    }

    i = 0;
    while (1){

        while (strcmp(buff +strlen(buff) - 4, '\r\n\r\n')) {
            recv(client_connected, buff, MAX_LEN_RECV, 0); // recives string from http untill \r\n\r\n
            full_request = strcat(full_request, buff);
        }

        send(servers_array[i], full_request, MAX_LEN_RECV, 0); //sends the string to next server by order
        returned_string = read_server_message(servers_array[i]);
        send(client_connected, returned_string, MAX_LEN_RECV, 0);
        i = (1==3) ? 0 : i + 1;

        /* not sure if break is needed */
    }



}