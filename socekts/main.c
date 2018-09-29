#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#if 0
typedef struct 
{
	int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
	int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
	int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
	int              ai_protocol;  // use 0 for "any"
	size_t           ai_addrlen;   // size of ai_addr in bytes
	struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
	char            *ai_canonname; // full canonical hostname
	struct addrinfo *ai_next;      // linked list, next node
} addrinfo ;


typedef struct 
{
	unsigned short    sa_family;    // address family, AF_xxx
	char              sa_data[14];  // 14 bytes of protocol address
}sockaddr ; 






// (IPv4 only--see struct in6_addr for IPv6)

// Internet address (a structure for historical reasons)
typedef struct {
	uint32_t s_addr; // that's a 32-bit int (4 bytes)
}in_addr ;

// (IPv4 only--see struct sockaddr_in6 for IPv6)

typedef struct {
	short int			sin_family;  // Address family, AF_INET
	unsigned short int	sin_port;    // Port number
	in_addr				sin_addr;    // Internet address
	unsigned char		sin_zero[8]; // Same size as struct sockaddr
}sockaddr_in ;

/* IPv6 versions!
// (IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)

struct sockaddr_in6 {
u_int16_t       sin6_family;   // address family, AF_INET6
u_int16_t       sin6_port;     // port number, Network Byte Order
u_int32_t       sin6_flowinfo; // IPv6 flow information
struct in6_addr sin6_addr;     // IPv6 address
u_int32_t       sin6_scope_id; // Scope ID
};

struct in6_addr {
unsigned char   s6_addr[16];   // IPv6 address
};


struct sockaddr_storage {
sa_family_t  ss_family;     // address family

// all this is padding, implementation specific, ignore it:
char      __ss_pad1[_SS_PAD1SIZE];
int64_t   __ss_align;
char      __ss_pad2[_SS_PAD2SIZE];
};


*/



#endif


#define IPADDR	"ADDRES"
#define PORT	"666" 
int main()
{
	printf("hello cia !\n");
	struct addrinfo* linkedListAddrInfo = NULL; 
	struct addrinfo hints;
	int status = 0;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;		// dont spec v4 or v6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream socekts
	hints.ai_flags = AI_PASSIVE;		// fill in ip for me


	//fill linkedListAddrInfo struct
	if ((status = getaddrinfo(NULL, PORT, &hints, &linkedListAddrInfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 0;
	}

	
	


	free(linkedListAddrInfo);
	return 0;
}

