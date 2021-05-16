#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>   
#include <thread>
#include <limits>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <zlib.h>
#include <iostream>

#define SERVER_PORT (12345)
#define LISTENNQ (5)
#define BUFFERSIZE (8192)
#define MAXTHREAD (5)


std::map<std::string, std::string> FILE_TYPES{
            {"css", "text/css"},
            {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {"html", "text/html"},
            {"gif", "image/gif"},
            {"jpg", "image/jpg"},
            {"js", "text/javascript"},
            {"mp3", "audio/mpeg"},
            {"mp4", "video/mp4"},
            {"pdf", "application/pdf"},
            {"png", "image/png"},
            {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            {"txt", "text/plain"},
            {"wav", "audio/wav"},
            {"weba", "audio/webm"},
            {"webm", " video/webm"},
            {"webp", "image/webp"},
            {"xhtml", "application/xhtml+xml"},
            {"xls", "application/vnd.ms-excel"},
            {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {"zip", "application/zip"},
            {"7z", "application/x-7z-compressed"}
            
};

class HandleHttp{
    
    public:
        //map all files

        std::string url_path;
        std::string version; 
        std::string method;
        std::string ip_str;
        std::string errorMessage;
        bool error;
        HandleHttp(){
            url_path = "";
            version = "";
            method = "";
            ip_str = "";
            errorMessage = "";
            error = false;
        }

        std::string compress_string(const std::string& fileContent,int compressionlevel = Z_BEST_COMPRESSION){

            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            //init zip and settings
            if (deflateInit2(&zs, compressionlevel, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
                throw(std::runtime_error("deflateInit failed while compressing."));
                 }

            //configure gzip input
            zs.next_in = (Bytef*)fileContent.data();
            zs.avail_in = fileContent.size();

            //define var
            int ret;
            char outbuffer[32768];
            std::string compressedContent;

            do {
                    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                    zs.avail_out = sizeof(outbuffer);

                    ret = deflate(&zs, Z_FINISH);

                    if (compressedContent.size() < zs.total_out) {
                        //append to output buffer
                        compressedContent.append(outbuffer, zs.total_out - compressedContent.size());
                     }
            } while (ret == Z_OK);
            //stop when ret != Z_OK, means reached the end

            deflateEnd(&zs);


            //check if ended compression successfully
            if (ret != Z_STREAM_END) {
                std::ostringstream oss;
                oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
                throw(std::runtime_error(oss.str()));
            }

            return compressedContent;

        }

        void writeToLog(std::string header){
            std::ofstream logFile;
            logFile.open("log.txt", std::ios::app);
            if(!logFile.good()){
                std::cout <<"Error: unable to open log";
            }
            logFile << header << "\n";
            logFile.close();
            return;
        }




        HandleHttp *parse(std::string message, std::string(ip_str)){
            HandleHttp *req = new HandleHttp();
            req->ip_str = ip_str;
            std::cout << "PRINT IP\nPRINT IP\nPRINT IP\n"<< req->ip_str;
            const std::string space = " ";
            const std::string nextLine = "\r\n";
            int start_get = 0;
            //look for end line, get whole line
            int end_line = message.find(nextLine);
            if(start_get >= message.length() || end_line == std::string::npos){
                std::cout<<"Error: at getting line";
                req->errorMessage = "Error at getting line";
                req->error = true;
                return req;
            }
            std::string line = message.substr(start_get, end_line);

            //get method
            int part_end = line.find(space);

            //check if there is are still items left in line
            if(part_end == std::string::npos){
                std::cout<<"Error: at getting method";
                req->errorMessage = "Error at getting method";
                req->error = true;
                return req;
            }
            
            req->method = line.substr(start_get, part_end);
            
            //get url path in request

            //check if there are still items left in line
            if(part_end + 1 >= line.length()){
                std::cout<<"Error: at getting url";
                req->errorMessage = "Error at getting url";
                req->error = true;
                return req;
            }
            //remove method from line
            std::string tmp = line.substr(part_end + 1, end_line);
            part_end = tmp.find(space);
            req->url_path = tmp.substr(start_get, part_end);
            if(req->url_path == "/"){
                req->url_path = "/index.html";
            }

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





        std::string generate404(int connfd, std::string fileRequested){
            std::string displayMessage = std::string("<!doctype html><head><title>404 File Not Found</title><h1>404 File Not Found<h1></head><body><h2>The file requested: ") +  fileRequested + std::string(" is not found</h2></body></html>");
            std::string extraLogInfo = std::string("Extra info\nConnfd: ") + std::to_string(connfd) + std::string("\nIP: ") + this->ip_str + std::string("\nFile Requested: " )+ fileRequested + std::string("\n");

            std::string response;
            response = this->version;
            response +="  404 File Not Found\r\n";
            response += "Date: " + getTime() + "\r\n";
            response += "Server: tywuab \r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Content-Length: " + std::to_string(displayMessage.length()) + "\r\n";
            this->writeToLog(response + extraLogInfo);
            response += "\r\n";
            response += displayMessage;
            return response;
        }





        std::string generate501(int connfd){

            std::string displayMessage = std::string("<!doctype html<head><title>501 Method Not Allowed</title><h1>501 Method Not Allowed<h1></head><body><h2>The method: ") +  this->method + std::string(", is not supported, only GET is supported</h2></body></html>");
            std::string extraLogInfo = std::string("Extra info\nConnfd: ") + std::to_string(connfd) + std::string("\nIP: ") + this->ip_str + std::string("\nMethod: ") + this->method + std::string("\n");

            std::string response;
            response = this->version;
            response +="  501 Method Not Allowed\r\n";
            response += "Date: " + getTime() + "\r\n";
            response += "Server: tywuab \r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Content-Length: " + std::to_string(displayMessage.length()) + "\r\n";
            this->writeToLog(response + extraLogInfo);
            response += "\r\n";
            response += displayMessage;
            return response;
        }





        std::string generate415(int connfd, std::string extension){

            std::string displayMessage = std::string("<!doctype html<head><title>415 Unsupported Media Type</title><h1>415 Unsupported Media Type<h1></head><body><h2>The extension specified: ") +  extension + std::string(", is not supported</h2></body></html>");
            std::string extraLogInfo = std::string("Extra info\nConnfd: ") + std::to_string(connfd) + std::string("\nIP: ") + this->ip_str + std::string("\nRequested Extension: ") + extension + std::string("\n");

            std::string response;
            response = this->version;
            response +="  415 Unsupported Media Type\r\n";
            response += "Date: " + getTime() + "\r\n";
            response += "Server: tywuab \r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Content-Length: " + std::to_string(displayMessage.length()) + "\r\n";
            this->writeToLog(response + extraLogInfo);
            response += "\r\n";
            response += displayMessage;
            return response;
        }
        




        std::string generate400(int connfd){
            std::string displayMessage = std::string("<!doctype html<head><title>400 Bad Request</title><h1>400 Bad Request<h1></head><body><h2>There is something wrong with your request, please try again</h2></body></html>");
            std::string extraLogInfo = std::string("Extra info\nConnfd: ") + std::to_string(connfd) + std::string("\nIP: ") + this->ip_str + std::string("\nRequested method: ") + this->method + std::string("\nRequested URL:") + this->url_path + std::string("\nError Message:") + this->errorMessage + std::string("\n");

            std::string response;
            response = this->version;
            response +="  400 Bad Request\r\n";
            response += "Date: " + getTime() + "\r\n";
            response += "Server: tywuab \r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Content-Length: " + std::to_string(displayMessage.length()) + "\r\n";
            this->writeToLog(response + extraLogInfo);
            response += "\r\n";
            response += displayMessage;
            return response;
        }





        void sendResponse(int connfd, std::string response){
            write(connfd, response.c_str(), response.length());
            close(connfd);
            return;
        }





        void generateResponse(int connfd){
            std::string response = " ";

            if(this->error){
                sendResponse(connfd, this->generate400(connfd));
                close(connfd);
                return;
            }

            //bad request if not get
            if(this->method != "GET"){
                sendResponse(connfd, this->generate501(connfd));
                close(connfd);
                return;
            }


            //start file response
            char buff[BUFFERSIZE];

            //get file type
            int extension_start = this->url_path.find(".");
            std::string fileType = this->url_path.substr(extension_start+1, this->url_path.length());

            //check if file extension is supported
            if(FILE_TYPES.find(fileType) == FILE_TYPES.end()){
                sendResponse(connfd, this->generate415(connfd, fileType));
                close(connfd);
                return;
            }

            std::string contentType = FILE_TYPES[fileType];
            int content_end = contentType.find("/");
            std::string filePath = this->url_path.substr(1, this->url_path.length());

            //check if open as binary and set if want to compress binary only 
            bool compress = true;
            auto flags = std::ifstream::in;
            if(contentType.substr(0, content_end) != "text"){
                flags |= std::ifstream::binary;
                compress = true;
            }

            //open file
             std::ifstream file(filePath.c_str(), flags);

            //check if file is found
            if(!file.good()){
                std::cout<<"Error: file not found";
                std::cout << "\n";
                sendResponse(connfd, this->generate404(connfd, filePath));
                close(connfd);
                return;

            }else{

                //read file
                std::stringstream fileContent;
                fileContent << file.rdbuf();
                file.close();

                //get file content in string
                std::string stringContent = fileContent.str();
                std::string contentLength = std::to_string(stringContent.length());
                
                //check if need to compress, this is for when you only want to compress binary or text files
                if(true){
                    std::string compressed_content = compress_string(stringContent);
                    std::string compressed_size = std::to_string(compressed_content.length());


                    //=========[Debug]=============
                    //check if compression is effective

                    std::cout<< "COMPRESSED \n\n";
                    if(std::stoi(compressed_size) < std::stoi(contentLength)){
                        std::cout<< "COMPRESSION OK \n";
                        std::cout<< "COMPRESSION OK \n";
                        std::cout<< "COMPRESSION OK \n";
                    }else{
                        std::cout<< "COMPRESSION FAILED \n";
                        std::cout<< "COMPRESSION FAILED \n";
                        std::cout<< "COMPRESSION FAILED \n";
                    }

                    //Write response for compressed
                    std::string response = this->version;
                    response += " 200 OK\r\n";
                    response += "Date: " + getTime() + "\r\n";
                    response += "Server: tywuab\r\n";
                    response += "Content-Encoding: gzip\r\n";
                    response += "Content-Length: " + compressed_size + "\r\n";

                    //write header to log
                    this->writeToLog(response + "Compression: TRUE \r\n");

                    //complete response
                    response += "\r\n";
                    response += compressed_content;
                    sendResponse(connfd, response);
                    close(connfd);
                    return;

                }else{
                    //generate response if don't need to compress
                    std::string response = this->version;
                    response += " 200 OK\r\n";
                    response += "Date: " + getTime() + "\r\n";
                    response += "Server: tywuab\r\n";
                    response += "Content-Type: " + contentType + "\r\n";
                    response += "Content-Length: " + contentLength + "\r\n";
                    
                    //write header to log
                    this->writeToLog(response + "Compression: TRUE \r\n");

                    //comlete response
                    response += "\r\n";
                    response += stringContent;
                    sendResponse(connfd, response);
                    close(connfd);
                    return;
                }
            }
         }

};


void threadHandler(int connfd, std::string(ip_str)){

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

    req = req->parse(message, ip_str);
    //======[Debug]=======
    std::cout << req->method;
    std::cout << "\n";
    std::cout << req->url_path;
    std::cout << "\n";
    std::cout << req->version;
    std::cout << "\n";
    std::cout << req->ip_str;
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
    
        std::thread newReq (threadHandler, connfd, std::string(ip_str));
        newReq.detach();
    }
}