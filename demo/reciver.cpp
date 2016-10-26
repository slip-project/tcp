#include <cstdio>   //for printf
#include <cctype>
#include <cstring> //memset
#include <sys/socket.h>  //for socket ofcourse
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>  //Provides declarations for ip header
#include <netinet/in.h>
#include <arpa/inet.h>

void print(void *buf, int length) {
    char *bp = (char *) buf;
    for (int i = 0; i < length; ++i)
        putchar( isprint(bp[i]) ? bp[i] : '.' );
    putchar('\n');
}

int main(int argc, char const *argv[]) {

    // 创建 raw socket
    int s = socket (AF_INET, SOCK_RAW, IPPROTO_TCP);

    if(s == -1) {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }

    char datagram[4096] , source_ip[32] , *data , *pseudogram;

    memset (datagram, 0, 4096);

    struct sockaddr_in destaddr, sourceaddr;

    //some address resolution
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(1234); // 目的端口
    // destaddr.sin_addr.s_addr = inet_addr(argv[1]); // 目的ip

    socklen_t destaddr_len = sizeof(destaddr);

    sourceaddr.sin_family = AF_INET;
    sourceaddr.sin_port = htons(1234); // 本机端口
    sourceaddr.sin_addr.s_addr = inet_addr("172.18.181.160"); // 本机ip

    socklen_t sourceaddr_len = sizeof(sourceaddr);

    if (bind(s, (struct sockaddr *) &sourceaddr, sourceaddr_len) < 0) {
        perror("bind failed");
        return 0;
    }
    while (1)
    {
        int tot_len;
        if ((tot_len = recvfrom (s, datagram, sizeof(datagram), 0, (struct sockaddr *) &destaddr, &destaddr_len)) < 0) {
            perror("recvfrom failed");
        } else {
            printf("================================\n");
            struct ip *iphd = (struct ip *) (datagram);
            struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
            char *data = (char *) (datagram + sizeof(ip) + sizeof(struct tcphdr));


            #ifdef __APPLE__ // macOS
            printf("sender ip: %s\n", inet_ntoa(iphd->ip_src));

            printf("source port: %hu\n", ntohs(tcph->th_sport));
            printf("destination port: %hu\n", ntohs(tcph->th_dport));
            printf("sequence number: %hu\n", ntohs(tcph->th_seq));

            #elif __linux__ // linux
            printf("sender ip: %s\n", inet_ntoa(iphd->ip_src));

            printf("source port: %hu\n", ntohs(tcph->source));
            printf("destination port: %hu\n", ntohs(tcph->dest));
            printf("sequence number: %hu\n", ntohs(tcph->seq));

            #endif

            printf("data: %s\n", data);
        }
    }


    return 0;
}
