#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>   
#include <thread>
#include <iostream>
#include <limits>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <unistd.h>

#define SERVER_PORT (12345)
#define LISTENNQ (5)
#define BUFFERSIZE (8192)
#define MAXTHREAD (5)



class HandleHttp{
    
    public:
        //map all files
        static const std::map<std::string, std::string> FILE_TYPES{
            {txt, text/plain}
        }
        std::string url_path;
        std::string version; 
        std::string method;
        HandleHttp(){
            url_path = "";
            version = "";
            method = "";
        }
        HandleHttp *parse(std::string message){
            HandleHttp *req = new HandleHttp();
            const std::string space = " ";
            const std::string nextLine = "\r\n";
            int start_get = 0;
            //look for end line, get whole line
            int end_line = message.find(nextLine);
            if(start_get >= message.length() || end_line == std::string::npos){
                std::cout<<"Error: at getting line";
                delete req;
                req = NULL;
                return req;
            }
            std::string line = message.substr(start_get, end_line);

            //get method
            int part_end = line.find(space);

            //check if there is are still items left in line
            if(part_end == std::string::npos){
                std::cout<<"Error: at getting method";
                delete req;
                req = NULL;
                return req;
            }
            req->method = line.substr(start_get, part_end);

            //get url path in request

            //check if there are still items left in line
            if(part_end + 1 >= line.length()){
                std::cout<<"Error: at getting url";
                delete req;
                req = NULL;
                return req;
            }
            //remove method from line
            std::string tmp = line.substr(part_end + 1, end_line);
            part_end = tmp.find(space);
            req->url_path = tmp.substr(start_get, part_end);

            //get version
            req->version = tmp.substr(part_end + 1, end_line);

            return req;
    
        }

        std::string getTime(){
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [80];

            time (&rawtime);
            timeinfo = gmtime(&rawtime);

            strftime (buffer,80,"%a, %d %b %G %H:%M:%S GMT",timeinfo);

            std::string timeString(buffer);
            return timeString;
        }

        std::string generate404(){

            std::string response;
            response = this->version;
            response +="  404 File Not Found\r\n";
            response += "Date: " + getTime() + "\r\n";
            response += "Server: tywuab \r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Content-Length: 23\r\n";
            response += "\r\n";
            response += "<h1>File Not Found</h1>";
            return response;
        }

        std::string generate501(){

                std::string response;
                response = this->version;
                response +="  501 Method Not Allowed\r\n";
                response += "Date: " + getTime() + "\r\n";
                response += "Server: tywuab \r\n";
                response += "Content-Type: text/html; charset=utf-8\r\n";
                response += "Content-Length: 27\r\n";
                response += "\r\n";
                response += "<h1>Method not allowed</h1>";
                return response;
        }

        void sendResponse(int connfd, std::string response){
            write(connfd, response.c_str(), response.length());
            return;
        }

        void generateResponse(int connfd){
            std::string response = " ";
            //bad request if not get
            if(this->method != "GET" && this->method == "POST"){
                sendResponse(connfd, this->generate501());
            }
            char buff[BUFFERSIZE];
            
            sendResponse(connfd, this->generate404());
            return;

         }

};


void threadHandler(int connfd){

    HandleHttp *req = NULL;
    int buffSize = 0;
    char buff[BUFFERSIZE] = {0};
    std::string message = "";

    //try to get response, break when get '\n'
    while(1){
        buffSize += recv(connfd, buff, BUFFERSIZE - 1, 0);
        message += buff;
        if(buffSize && buff[buffSize - 1] == '\n'){
            break;
        }
    }
    req = req->parse(message);
    std::cout << req->method;
    std::cout << "\n";
    std::cout << req->url_path;
    std::cout << "\n";
    std::cout << req->version;
    std::cout << "\n";

    req->generateResponse(connfd);

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
        std::cout << "Incoming connection from: " << ip_str << " with fd: \n" << ntohs(cliaddr.sin_port) << connfd;
        std::cout << "\n";
    
        std::thread newReq (threadHandler, connfd);
        newReq.detach();
    }
}