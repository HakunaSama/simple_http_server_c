//
// Created by apple on 2023/10/11.
//
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <process.h>

#define BUF_SIZE 2048
#define BUF_SMALL 100

unsigned WINAPI RequestHandler(void *arg);
char * ContentType(char * file);
void SendData(SOCKET sock,char * ct,char * fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char *message);

int main(int argc ,char * argv[]) {
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAdr, clntAdr;

    HANDLE hThread;
    DWORD dwThreadId;
    int clntAdrSize;

    if (argc != 2) {
        printf("Usage:%s<port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup Error");
    //获取并设置服务器套接字信息
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAdr, 0, sizeof(servAdr));
    lisnAdr.sin_family = AF_INET;
    lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
    lisnAdr.sin_port = htons(atoi(argv[1]));
    //绑定服务器套接字并开始监听
    if (bind(hServSock, (SOCKADDR * ) & servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind error");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen error");

    /* 请求及响应 */
    //循环接受连接请求
    while (1) {
        clntAdrSize = sizeof(clntAdr);
        hClntSock = accept(hServSock, (SOCKADDR * ) & clntAdr, &clntAdrSize);
        printf("Connection Request:%s:%d\n", inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
        //开启RequestHandler线程来处理请求
        hThread = (HANDLE) _beginthreadex(NULL, 0, RequestHandler, (void *) hClntSock, 0, (unsigned *) &dwThreadID);
    }
    closesocket(hServSock);
    WSACleanup();
    return 0;
}

unsigned WINAPI RequestHandler(void *arg) {
    SOCKET hClntSock=(SOCKET)arg;//客户端套接字
    char buf[BUF_SIZE];
    char method[BUF_SMALL];
    char ct[BUF_SMALL];
    char fileName[BUF_SMALL];

    recv(hClntSock,buf,BUF_SIZE,0);//接收数据
    if(strstr(buf,"HTTP/")==NULL) {//查看是否是HTTP提出的请求
        SendErrorMSG(hClntSock);
        closesocket(hClntSock);
        return 1;
    }

    strcpy(method,strtok(buf," /"));
    if(strcmp(method,"GET"))//查看是否是GET方式的请求
        SendErrorMSG(hClntSock);

    strcpy(fileName,strtok(NULL," /"));//查看请求文件名
    strcpy(ct,ContentType(fileName));//查看content-type
    SendData(hClntSock,ct,fileName);//响应：传输该文件
    return 0;
};

void SendData(SOCKET sock,char * ct,char * fileName) {
    char protocol[]="HTTP/1.0 200 OK\r\n";
    char servName[]="Server:simple web server\r\n";
    char cntLen[]="Content-length:2048\r\n";
    char cntType[BUF_SMALL];
    char buf[BUF_SIZE];
    FILE * sendFile;

    sprintf(cntType,"Content-type:%s\r\n\r\n",ct);
    if ((sendFile= fopen(fileName,"r"))==NULL) {
        SendErrorMSG(sock);
        return;
    }

    /* 传输头信息 */
    send(sock,protocol, strlen(protocol),0);
    send(sock,servName, strlen(servName),0);
    send(sock,cntLen, strlen(cntLen),0);
    send(sock,cntType, strlen(cntType),0);

    /* 传输请求数据 */
    while(fgets(buf,BUF_SIZE,sendFile)!=NULL)
        send(sock,buf, strlen(buf),0);

    closesocket(sock);//由HTTP协议响应后断开
}

void SendErrorMSG(SOCKET sock){//发生错误时传递信息
    char protocol[]="HTTP/1.0 400 Bad Request\r\n";
    char servName[]="Server:simple web server\r\n";
    char cntLen[]="Content-length:2048\r\n";
    char cntType[]="Content-type:text/html\r\n\r\n";
    char content[]="<html><head><title>NETWORK</title></head>"
                   "<body><font size=+5><br> 发生错误！查看请求文件名和请求方式！"
                   "</font></body></html>";
    send(sock,protocol, strlen(protocol),0);
    send(sock,servName, strlen(servName),0);
    send(sock,cntLen, strlen(cntLen),0);
    send(sock,cntType, strlen(cntType),0);
    send(sock,content, strlen(content),0);
    closesocket(sock);
}

char * ContentType(char *file){//区分content type
    char extension[BUF_SMALL];
    char fileName[BUF_SMALL];
    strcpy(fileName,file);
    strtok(fileName,".");
    strcpy(extension, strtok(NULL,"."));
    if(!strcmp(extension,"html")||!strcmp(extension,"htm"))
        return "text/html";
    else
        return "text/plain";
}

void ErrorHandling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}