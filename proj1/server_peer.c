#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/stat.h>

#define SERVER_URL "127.0.0.1"
#define PROTOCOL "HTTP/1.0"
#define BUF_SIZE 1024
#define LISTENQ 128

int client_socket;
int server_socket;
char msg[100];
struct sockaddr_in server_addr;

pthread_mutex_t wlock;
pthread_mutex_t mlock;
pthread_cond_t cond;

void *worker(void *ptr){
    char method[32], file[32], protocol[32];
    char buf[BUF_SIZE];
    char tmp;
    int num;
    int i;
    int addrlen;
    unsigned int count;
    struct epoll_event ev, events[LISTENQ];
	int e_fd;

    struct stat sb;
	
    FILE *fe;
    FILE *f;

    if( (e_fd=epoll_create(LISTENQ))<0){
		printf("Error : Epoll Creation.\n"); return 0;
	}
	ev.events=EPOLLIN;
	ev.data.fd=server_socket;

	epoll_ctl(e_fd,EPOLL_CTL_ADD,server_socket,&ev);

    while(1){
        if((num=epoll_wait(e_fd, events, LISTENQ, -1)) <0){
			printf("Error : Epoll Wait.\n"); 
            return 0;
		}

        for(i=0;i<num;i++){
			if(events[i].data.fd==server_socket){ 
				addrlen=sizeof(server_addr);
				if( (client_socket=accept(server_socket, (struct sockaddr*)&server_addr,&addrlen)) <0){
					printf("connection to client failed in server\n");
					return 0;
				} else {
                    printf("Connection Request : %s:%d\n", 
			        inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port)); // inet_ntoa : 네트워크 바이트 순서를 dotted-10진수로 변경 
			        recv(client_socket, msg, sizeof(msg), 0);
                }
				ev.events=EPOLLIN;
				ev.data.fd=client_socket;
				epoll_ctl(e_fd,EPOLL_CTL_ADD, client_socket, &ev);
			}
			else{	
				client_socket=events[i].data.fd;
				fe=fdopen(client_socket,"r+");
    
				recv(client_socket, msg, sizeof(msg), 0);
                if (sscanf(msg, "%[^ ] %[^ ] %[^ ]", method, file, protocol) != 3){
                    sprintf(buf, "400 Bad Request\n");
                }   
                if (stat(file, &sb) < 0 ) {
                    sprintf(buf, "404 Not Found\n");
                }
                f = fopen(file, "r");
                if ( f == (FILE*) 0 ){
                    sprintf(buf, "403 Forbidden\n");
                }

                for(count = 0; fgets(buf, BUF_SIZE, f) != NULL; count++){}

                fclose(f);
                send(client_socket, &count, sizeof(count), 0);
                f = fopen(file, "r");
                
                for(i=0; i<count; i++){
                    fgets(buf, BUF_SIZE, f);
                    send(client_socket, buf, BUF_SIZE, 0);
                }
                char end_msg[]="quit";
		        send(client_socket, end_msg, sizeof(end_msg), 0);
                
                fclose(f);
                ev.events=EPOLLIN;
				ev.data.fd=client_socket;
				epoll_ctl(e_fd,EPOLL_CTL_DEL, client_socket, &ev);

				fclose(fe);
				printf("success\n");
			}
		}

    }
}

int main(int argc, char **argv){
    int port;
    int threads_num;
    
    int addrlen;
    int i;
    int status;
    
    struct sockaddr_in client_addr;
    pthread_t tid;
     
    if(argc != 3){
		printf("Usage: ./server_pear [port num] [the number of threads]\n");
		return -1;
	}
    port = atoi(argv[1]); //printf("port: %d\n", port);
	threads_num = atoi(argv[2]);

    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error: Unable to open socket in server.\n");
        return -1;
    }
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    /* Disable the setsockopt function 
    * setsockopt(server_socket , SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
    */

    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("Error: Unable to bind socket in server.\n");
        return -1;
    }
    if(listen(server_socket, LISTENQ) < 0){
		printf("Error: Unable to listen socket in server\n");
		return -1;
	}
    

    for(i=0; i<threads_num; i++){
        pthread_create(&tid, NULL, worker, NULL);
    }
    pthread_join(tid, (void**)&status);

    while(1){
        //recv(client_socket, msg, sizeof(msg), 0);
        //pthread_cond_broadcast(&cond);
        //pthread_cond_signal(&cond);
        //if(count++ == threads_num) count = 0;
    }

    close(server_socket);
    close(client_socket);

    return 0;
}

