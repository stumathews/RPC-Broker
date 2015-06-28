
    struct in_addr address = peerp->sin_addr; 
    unsigned long srcAddr = address.s_addr; // load with inet_pton();
    unsigned short sin_port = peerp->sin_port; //load with htons
