//
//  ViewController.m
//  broadcasttest
//
//  Created by Kresimir Spes on 01/01/14.
//  Copyright (c) 2014 Kresimir Spes. All rights reserved.
//

#import "ViewController.h"
/*
 
 multicast.c
 
 The following program sends or receives multicast packets. If invoked
 with one argument, it sends a packet containing the current time to an
 arbitrarily chosen multicast group and UDP port. If invoked with no
 arguments, it receives and prints these packets. Start it as a sender on
 just one host and as a receiver on all the other hosts
 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define EXAMPLE_PORT 1505
#define EXAMPLE_GROUP "239.5.0.5"

int kok(int argc)
{
    struct sockaddr_in addr;
    int addrlen, sock, cnt;
    struct ip_mreq mreq;
    char message[256] = {0};
    
    /* set up socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(EXAMPLE_PORT);
    addrlen = sizeof(addr);
    
    if (argc > 1) {
        /* send */
        addr.sin_addr.s_addr = inet_addr(EXAMPLE_GROUP);
        while (1) {
            time_t t = time(0);
            sprintf(message, "POZDRAV IZ C++ -a!");
            printf("sending: %s\n", message);
            cnt = sendto(sock, message, sizeof(message), 0,
                         (struct sockaddr *) &addr, addrlen);
            if (cnt < 0) {
                perror("sendto");
                exit(1);
            }
            break;
            
            
            //	 psleep(500);
        }
        while (1) {
            unsigned int addrlen1;
            memset(message, 0, 256);
            addrlen1 = sizeof(addr);

            cnt = recvfrom(sock, message, sizeof(message), 0,
                           (struct sockaddr *) &addr, &addrlen1);
            if (cnt < 0) {
                perror("recvfrom");
                exit(1);
            } else if (cnt == 0) {
                break;
            }
            printf("%s:%d - message = \"%s\"\n", inet_ntoa(addr.sin_addr), addr.sin_port, message);
        }
    }
    
    return 0;
}



@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    
    [super viewDidLoad];
    kok(2);
 	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
