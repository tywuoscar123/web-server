#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>   
#include <thread>
#include <iostream>
#include <limits>

#define SERVER_PORT (12345)
#define LISTENNQ (5)
#define BUFFERSIZE (8192);
#define MAXTHREAD (5)



class HandleHttp{
    
    public:
        std::string url;
        std::string version; 
        HandleHttp(){
            
        }
        //in 

}

void threadHandler(void *args){

    int connfd = (int)args;
    char rcv_buff[BUFFERSIZE] = {0};
    

}

int main(int argc, char **argv){

    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);

    char ip_str[INET_ADDRSTRLEN] = {0};

    //initialize server socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        std::cout << "Error: init socket";
        return 0;
    }

    /* initialize server address (IP:port) */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; /* IP address: 0.0.0.0 */
    servaddr.sin_port = htons(SERVER_PORT); /* port number */

        /* bind the socket to the server address */
        if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
                std::cout << "Error: bind";
                return 0;
        }


        if (listen(listenfd, LISTENNQ) < 0) {
            std::cout << "Error: listen";
                return 0;
        }

    //keep processing incoming requests

    while (1) {
        /* accept an incoming connection from the remote side */
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0) {
            std::cout << "Error: accept";
            return 0;
        }

        /* print client (remote side) address (IP : port) */
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
        std::cout << "Incoming connection from: " << ip_str << "with fd: \n" <<   << ntohs(cliaddr.sin_port) << connfd;
    
        std::thread newReq ()
        if (thread(&threads[threads_count], NULL, request_func, (void *)connfd) != 0) {
        printf("Error when creating thread %d\n", threads_count);
        return 0;
        }
    }
}