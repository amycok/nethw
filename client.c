#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 1250
#define MAXDATASIZE 100

char sendbuf[1024];
char recvbuf[1024];
char name[100];
int fd;
int board[9];

void usage(){
    printf("1.User name, hint: 1 Your\n");
    printf("2.Show all user, hint: 2\n");
    printf("3.Invite player, hint: 3 Your Other\n");
    printf("4.logout, hint: quit\n\n");
}

void print_board(int *board){
    printf(" ┌───┬───┬───┐\n");
    printf(" │ %d │ %d │ %d │\n", board[0], board[1], board[2]);
    printf(" ├───┼───┼───┤\n");
    printf(" │ %d │ %d │ %d │\n", board[3], board[4], board[5]);
    printf(" ├───┼───┼───┤\n");
    printf(" │ %d │ %d │ %d │\n", board[6], board[7], board[8]);
    printf(" └───┴───┴───┘\n");
}

int choose_user_turn(int *board){
    int i=0;
    int inviter=0, invitee=0;
    for(i=0; i<9; i++){
        if(board[i] == 1){
            inviter++;
        }
        else if(board[i] == 2){
            invitee++;
        }
    }
    if(inviter > invitee)
        return 2;
    else
        return 1;
}


void write_on_board(int *board, int location){
    print_board(board);
    int user_choice = choose_user_turn(board);
    board[location] = user_choice;
    sprintf(sendbuf, "7  %d %d %d %d %d %d %d %d %d\n", board[0], \
        board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8]);
}



void pthread_recv(void* ptr)
{
    int instruction;
    while(1)
    {
        memset(sendbuf,0,sizeof(sendbuf));
        instruction = 0;
        if ((recv(fd,recvbuf,MAXDATASIZE,0)) == -1)
        {
            printf("recv() error\n");
            exit(1);
        }
        sscanf (recvbuf,"%d",&instruction);
        switch (instruction)
        {
            case 2: {
                printf("%s\n", &recvbuf[2]); 
                break;
            }
            case 4: {
                char inviter[100];
                sscanf(recvbuf,"%d %s",&instruction, inviter);
                printf("%s\n", &recvbuf[2]); 
                printf("Accept, hint:5 1 %s\n", inviter);
                printf("Not accept, hint:5 0 %s\n", inviter);
                break;
            }
            case 6: {
                printf("Game Start!\n");
                printf("Blank space is 0\n");
                printf("Inviter is 1\n");
                printf("Invitee is 2\n");
                printf("Inviter start.\n");
                printf("Please input:-number\n");
                print_board(board);
                break;
            }
            case 8: {
                char msg[100];
                sscanf (recvbuf,"%d %d%d%d%d%d%d%d%d%d %s",&instruction, \
                    &board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6], \
                        &board[7],&board[8], msg);
                print_board(board);
                printf("%s\n", msg);
                printf("Please input:-number\n");
                break;
            }
            case 9: {
                printf("%s\n", &recvbuf[2]);
                break;
            }
            default:
                break;
        }   

        memset(recvbuf,0,sizeof(recvbuf));
    }
}



int main(int argc, char *argv[])
{
    int  numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server;


    if (argc !=2)
    {
        printf("Usage: %s <IP Address>\n",argv[0]);
        exit(1);
    }


    if ((he=gethostbyname(argv[1]))==NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }

    if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
    {
        printf("connect() error\n");
        exit(1);
    }

   
    printf("connect success\n");
    char str[]=" have come in\n";
    printf("user name：");
    fgets(name,sizeof(name),stdin);
    char package[100];
    memset(package, 0, sizeof(package));
    strcat(package, "1 ");
    strcat(package, name);
    send(fd, package, (strlen(package)),0);
    name[strlen(name)-1] = '\0';
    usage();
    pthread_t tid;
    pthread_create(&tid, NULL, (void*)pthread_recv, NULL);
    while(1){
        memset(sendbuf,0,sizeof(sendbuf));
        fgets(sendbuf,sizeof(sendbuf),stdin); 
        int location;
        char chatting[1024];
        if(sendbuf[0] == '-'){
            sscanf(&sendbuf[1], "%d", &location);
            write_on_board(board, location);
        }
        
        send(fd,sendbuf,(strlen(sendbuf)),0);  
        
        if(strcmp(sendbuf,"quit\n")==0){          
            memset(sendbuf,0,sizeof(sendbuf));
            printf("You have Quit.\n");
            return 0;
        }
    }
    pthread_join(tid,NULL);
    close(fd);
    return 0;
}
