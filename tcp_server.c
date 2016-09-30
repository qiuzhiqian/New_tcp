#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<sys/socket.h>  
#include<sys/types.h>  
#include<unistd.h>  
#include<netinet/in.h>  
#include <errno.h>   
#include <sys/stat.h>
#include "exch.h"
#include <fcntl.h>
#include <signal.h>
//#include <semaphore.h>

void sighandle(int sig);
void parsighandle(int sig);
void ReceHandle(unsigned char *buff,int len);

//#define PORT 6666  
unsigned char WaitResp(int sockfd);
unsigned char recvline[1024],sendline[1024];
int ser_sockfd,cli_sockfd;

//sem_t sem;
int msg[2];

int main(int argc,char **argv)  
{  
    int rece_fd;
    int send_fd;
    unsigned char ServerPorts[10];
    int ServerPort=0;
    int err,n;  
    int addlen;  
    int mode;
    char mode_c;
    struct sockaddr_in ser_addr;  
    struct sockaddr_in cli_addr;   

    //sem_init(&sem,0,0);
    pipe(msg);	//建立管道

    pid_t pid;

    ser_sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(ser_sockfd==-1)
    {
        printf("socket error:%s\n",strerror(errno));
        return -1;
    }

    printf("Input Listen Port:");
    scanf("%s",&ServerPorts);

    ServerPort=strtoint(ServerPorts);
    printf("ServerPort=%d\n",ServerPort);

    bzero(&ser_addr,sizeof(ser_addr));
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    ser_addr.sin_port=htons(ServerPort);
    err=bind(ser_sockfd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
    if(err==-1)
    {
        printf("bind error:%s\n",strerror(errno));
        return -1;
    }

    err=listen(ser_sockfd,5);
    if(err==-1)
    {
        printf("listen error\n");
        return -1;
    }

    printf("listen the port:\n");

    addlen=sizeof(struct sockaddr);
    cli_sockfd=accept(ser_sockfd,(struct sockaddr *)&cli_addr,&addlen);
    if(cli_sockfd==-1)
    {
        printf("accept error\n");
    }
    printf("%d\n",ntohs(cli_addr.sin_port));
    pid=fork();

    if(pid==0)	/*子进程，用来接受数据*/
    {
	unsigned char i=0;
	signal(SIGINT,sighandle);
        while(1)
	{
		n=recv(cli_sockfd,recvline,1024,0);
               	if(n==-1)
               	{
               		printf("recv error\n");
               	}
		ReceHandle(recvline,n);
	}
    }
    else if(pid>0)	/*父进程，用来发送数据*/
    {
	signal(SIGCHLD,parsighandle);
	while(1)
	{
		unsigned char sig[2];
		unsigned short datalen=0;
		unsigned char *datap=sendline+4;
		sendline[0]=0xFB;
		sendline[1]=0x20;
                scanf("%s",datap);
		datalen=strlen(datap);
		*(unsigned short *)(sendline+2)=datalen;
		*(unsigned short *)(sendline+datalen+4)=checksum(datap,datalen);
		*(sendline+datalen+6)=0x00;
		*(sendline+datalen+7)=0xFD;

                send(cli_sockfd,sendline,datalen+8,0);
		read(msg[0],sig,2);
		if(sig[0]==0x31)	//等待数据相应
		{
			//printf("get respons\n");
		}
	}
    }
    else
    {
	exit(0);
    }

    close(cli_sockfd);
    close(ser_sockfd);
    return 0;
}

void sighandle(int sig)
{
	printf("子结束进程\n");
	exit(0);
}

void parsighandle(int sig)
{
	printf("父进程结束\n");
	exit(0);
}

void ReceHandle(unsigned char *buff,int len)
{
	unsigned char tempbuff[8];
	int datalen;
	int i=0;
	unsigned char ack=0;
	if(buff[0]==0xFB)
	{
		datalen=*(unsigned short *)(buff+2);
	}
	else
	{
		for(i=0;i<len;i++)
		{
			printf("0x%02X ",buff[i]);
		}
		printf("\n");
		return;
	}

	if((datalen+8)>len)	return;
	if(buff[datalen+7]!=0xFD)	return;
	
	ack=*(buff+datalen+6);

	switch(buff[1])
	{
		case 0x01:	/*登录*/
		{
			if(ack==0)
				printf("login\n");
			break;
		}
		case 0x02:	/*下线*/
		{
			if(ack==0)
				printf("logout\n");
			break;
		}
		case 0x03:	/*心跳*/
		{
			if(ack==0)
				printf("heartbeat\n");
			break;
		}
		case 0x20:	/*数据包*/
		{
			if(ack==0)
			{
				printf("data\n");
				for(i=0;i<datalen;i++)
					printf("0x%02X ",buff[i+4]);
				printf("\n");

			}
			else if(ack==1)
			{
				//发送消息给宁外一个进程
				write(msg[1],"1",1);
			}
			break;
		}
		default:
		{
			break;
		}
	}
	if(ack==0)
	{
		tempbuff[0]=0xFB;
		tempbuff[1]=buff[1];
		tempbuff[2]=0x00;
		tempbuff[3]=0x00;
		tempbuff[4]=0x00;
		tempbuff[5]=0x00;
		tempbuff[6]=0x01;
		tempbuff[7]=0xFD;
		send(cli_sockfd,tempbuff,8,0);
	}
}

void SendHandle()
{
	
}

unsigned char WaitResp(int sockfd)
{
	int n=0;
	int i=0;
	int len=0;
	unsigned char endval=0;
	unsigned char recvline[1024];
	//unsigned char sendbuff[]
	n=recv(sockfd,recvline,1024,0);
        if(n==-1)
        {
                printf("recv error\n");
	}
        recvline[n]='\0';
        printf("recv data is:\n");
        for(i=0;i<n;i++)
        {
                printf("0x%02X ",recvline[i]);
        }
        printf("\n");

	len=*(unsigned short *)(recvline+2);
	printf("len=%d\n",len);
				
	endval=*(unsigned char *)(recvline+4+len+3);
	printf("firstval=0x%02X\n",recvline[0]);
	printf("endval=0x%02X\n",endval);
								
	if(recvline[0]==0xFB&&endval==0xFD)
	{
		return 1;
		//printf("AAAA\n");
	}
	return 0;
}
