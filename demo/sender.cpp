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

/*
    tcp 伪头部, 用于计算 checksum
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

/*
    计算checksum
*/
unsigned short csum(unsigned short *ptr,int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}

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

    //Datagram to represent the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;

    //zero out the packet buffer
    memset (datagram, 0, 4096);

    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (datagram);
    struct sockaddr_in destaddr, sourceaddr;
    struct pseudo_header psh;

    //Data part
    data = datagram + sizeof(struct tcphdr);
    strcpy(data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    int tot_len = sizeof(struct tcphdr) + strlen(data);

    //some address resolution
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(1234); // 目的端口
    destaddr.sin_addr.s_addr = inet_addr(argv[1]); // 目的ip

    socklen_t destaddr_len = sizeof(destaddr);

    sourceaddr.sin_family = AF_INET;
    sourceaddr.sin_port = htons(1234); // 本机端口
    sourceaddr.sin_addr.s_addr = inet_addr("192.168.1.19"); // 本机ip

    socklen_t sourceaddr_len = sizeof(sourceaddr);


    #ifdef __APPLE__ // macOS

    //TCP Header
    tcph->th_sport = sourceaddr.sin_port;
    tcph->th_dport = destaddr.sin_port;
    tcph->th_seq = 0;
    tcph->th_ack = 0;
    tcph->th_off = 5; //tcp header size
    tcph->th_flags = TH_SYN;
    // tcph->urg=0;
    tcph->th_win = htons (65535);    /* maximum allowed window size */
    tcph->th_sum = 0;    //leave checksum 0 now, filled later by pseudo header
    tcph->th_urp = 0;


    #elif __linux__ // linux

    //TCP Header
    tcph->source = sourceaddr.sin_port; // 发送端口
    tcph->dest = destaddr.sin_port;     // 目的端口
    tcph->seq = 0;                      // 序列号
    tcph->ack_seq = 0;                  // ack 序列号
    tcph->doff = 5;                     // tcp 头部大小
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (65535);    /* maximum allowed window size */
    tcph->check = 0;    //leave checksum 0 now, filled later by pseudo header
    tcph->urg_ptr = 0;

    #endif

    // Now the TCP checksum
    psh.source_address = sourceaddr.sin_addr.s_addr;
    psh.dest_address = destaddr.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data) );

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
    pseudogram = (char*)malloc(psize);

    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr) + strlen(data));

    #ifdef __APPLE__ // macOS

    tcph->th_sum = csum( (unsigned short*) pseudogram , psize);
    // tcph->th_sum = check_sum((unsigned short*)tcph, sizeof(struct tcphdr));

    #elif __linux__ // linux

    tcph->check = csum( (unsigned short*) pseudogram , psize);
    // tcph->check = check_sum((unsigned short*)tcph, sizeof(struct tcphdr));

    #endif

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    // int one = 1;
    // const int *val = &one;

    // if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    // {
    //     perror("Error setting IP_HDRINCL");
    //     exit(0);
    // }

    // if (bind(s, (struct sockaddr *) &sourceaddr, sourceaddr_len) < 0) {
    //     perror("bind failed");
    //     return 0;
    // }
    //loop if you want to flood :)
    for (int i = 0; i < 5; ++i)
    {
        //Send the packet
        if (sendto (s, datagram, tot_len , 0, (struct sockaddr *) &destaddr, destaddr_len) < 0) {
            perror("sendto failed");
        } else { //Data send successfully
            printf ("Packet Send. Length : %d \n" , tot_len);
        }
    }


    return 0;
}
