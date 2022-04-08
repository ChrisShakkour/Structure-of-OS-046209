#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#define __USE_C99_MATH

#define FULL_BLOCK_SIZE 516
#define FULL_DATA_SIZE 512
#define FILE_NAME_SIZE 514 // tmp - for equal structs size
#define OTHER_SIZE 514
typedef unsigned short u_short;

#define WRQ_OPCODE 2
#define ACK_OPCODE 4
#define DATA_OPCODE 3
#define ERROR_OPCODE 5
#define CLIENTS 200
//globals 
int Block_Number = 0;
int timeoutExpiredCount = 0;

// define all structs

struct in_addr_struct
{
    unsigned long s_addr;
};

typedef struct in_addr_struct in_addr_struct_t;


struct sockaddr_in_struct 
{
    short int sin_family;
    unsigned short int sin_port; // Port 
    in_addr_struct_t sin_addr; // IP
    unsigned char sin_zero[8];
};

struct WRQ_struct 
{
    u_short Opcode;
    char File_Name[FILE_NAME_SIZE];
} __attribute__((packed));

struct ACK_struct 
{
    u_short Opcode;
    u_short Block_Number;
} __attribute__((packed));

struct DATA_struct 
{
    u_short Opcode;
    u_short Block_Number;
    char data[FULL_DATA_SIZE];
} __attribute__((packed));

struct ERROR_struct 
{
    u_short Opcode;
    u_short ErrorCode;
    char ErrMsg[FULL_DATA_SIZE];
} __attribute__((packed));


typedef struct sockaddr_in_struct sockaddr_in_t;


// client struct - in an array
struct client
{
	char sa_data[14]; 
	char File_Name[FULL_BLOCK_SIZE];
	int block_number;
	int num_of_timeouts;
	struct timeval client_tv;
	sockaddr_in_t addr;
	FILE* fd;
} __attribute__((packed));

struct buffer 
{
    u_short Opcode;
    char other[OTHER_SIZE];
} __attribute__((packed));


// define all typedefs
typedef struct WRQ_struct WRQ_msg_t;
typedef struct ACK_struct ACK_msg_t;
typedef struct DATA_struct DATA_msg_t;
typedef struct ERROR_struct ERROR_msg_t;
typedef struct client client_t;
typedef struct buffer buffer_t;

client_t clients_array[CLIENTS] = {0}; 
int client_index = 0;

bool add_client_to_array(char* filename, struct sockaddr* echoClntAddr, int filesize)
{
	for (int i = 0; i < CLIENTS; i++)
	{
		if (!strcmp(clients_array[i].sa_data, echoClntAddr->sa_data))
		{
			return false;
		}
	}
	memcpy(clients_array[client_index].sa_data, echoClntAddr->sa_data, 14);
	memcpy(clients_array[client_index].File_Name, filename, filesize);
	clients_array[client_index].block_number = 0;
	clients_array[client_index].fd = NULL;
	clients_array[client_index].num_of_timeouts = 0;
	memcpy(&clients_array[client_index].addr, echoClntAddr, sizeof(&echoClntAddr));
	gettimeofday(&(clients_array[client_index].client_tv), NULL);
	client_index++;
	return true;
}

bool is_filename_exist(char* filename)
{
	for (int i = 0; i < CLIENTS; i++)
	{
		if (!strcmp(clients_array[i].File_Name, filename))
		{
			return true;
		}
	}
	return false;
}

bool check_file_name(char* File_Name)
{
	int dots_counter = 1;
	char* file_name_copy = File_Name;
	char specials[] = {'^', '<', '>', ';', '|', '/', ',', ':', '=', '?', '\"', '*', '"', '@', '#', '!', '$', '%', '&', '(', ')', '-', '_', '+', '~'};
	if (!isalpha(*file_name_copy)) 
	{
		return false;
	} 
	while (*file_name_copy != '\0')
	{
		if (*file_name_copy == '.')
		{
			dots_counter--;
		}
		if (dots_counter < 0)
		{
			return false;
		}
		for (int i=0; i<25; i++)
		{
			if (*file_name_copy == specials[i])
			{
				return false;
			}
		}
		file_name_copy++;
	}
	return true;
}


int is_client_exist(buffer_t buffer, struct sockaddr* echoClntAddr)
{
	for (int i = 0; i < CLIENTS; i++)
	{
		if (!strcmp(clients_array[i].sa_data, echoClntAddr->sa_data))
		{
			return i;
		}
	}
	return -1;
}


int main(int argc, char* argv[]) {
    
	if (argc != 4)
    {
    	printf("Number of arguments isn't correct");
    	exit(1);
    }

    int echoServPort = atoi(argv[1]); /* Server port */
    int wait_for_packet_time = atoi(argv[2]); /* timeout */
    int number_of_failures = atoi(argv[3]);
    
    int sock;
    int index = -1;
    WRQ_msg_t WRQ_msg_inst;
    ACK_msg_t ACK_msg_inst;
    DATA_msg_t DATA_msg_inst;
    ERROR_msg_t ERROR_msg_inst;

    sockaddr_in_t echoServAddr;
    sockaddr_in_t echoClntAddr;
    unsigned int cliAddrLen;
    
    struct timeval current_select_tv;
    
    int recvMsgSize = FULL_BLOCK_SIZE;
    for (int i = 0; i < CLIENTS; i++)
	{
		clients_array[i].block_number = 0;
		clients_array[i].fd = NULL;
		clients_array[i].num_of_timeouts = 0;
	}

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("TTFTP_ERROR: socket creation error");
        exit(1);
    }
    memset(&echoServAddr, 0, sizeof(echoServAddr));

    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);

    if (bind(sock, (struct sockaddr*) & echoServAddr, sizeof(echoServAddr)) < 0) 
    {
        perror("TTFTP_ERROR: bind error");
        exit(1);
    }
    
    buffer_t client_buffer;

    for (;;) 
    {
    	
        int select_ret;
    	cliAddrLen = sizeof(echoClntAddr);
    	int size_client_buffer = sizeof(client_buffer);
    	
		fd_set FDSet;
		FD_ZERO(&FDSet);
		FD_CLR(sock, &FDSet);
		FD_SET(sock, &FDSet);
		struct timeval wait_time;
		wait_time.tv_sec = wait_for_packet_time;
		wait_time.tv_usec = 0;
    	
        if ((recvMsgSize = recvfrom(sock, &client_buffer, size_client_buffer, 0, (struct sockaddr*) & echoClntAddr, &cliAddrLen)) < 0) 
        {
            close(sock);
            perror("TTFTP_ERROR: recvfrom error");
            exit(1);
        }
            
        
///////////////////////////////////////////////////////////////////////////////
//WRQ
        
        // first msg from client
        int current_opcode = ntohs(client_buffer.Opcode);
        if (current_opcode == WRQ_OPCODE) // first msg from client
        {
        	memcpy(&WRQ_msg_inst, &client_buffer, recvMsgSize);
        	bool is_valid_check_client = is_filename_exist(WRQ_msg_inst.File_Name);
        	bool is_valid_add_client = add_client_to_array(WRQ_msg_inst.File_Name, (struct sockaddr*) & echoClntAddr, recvMsgSize-4);
        	
        	if (!is_valid_add_client)
			{
				int error_msg_size = sizeof(ERROR_msg_inst);
				char error_msg[] = "Unexpected packet";
				strcat(ERROR_msg_inst.ErrMsg, error_msg);
				ERROR_msg_inst.ErrorCode = htons(4); 
				int current_ack_opcode = htons(ERROR_OPCODE);
				ACK_msg_inst.Opcode = current_ack_opcode;
				if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
				   perror("TTFTP_ERROR: error while sending ack back to client");
				   exit(1);
				}            
				continue;
			}
        	
        	if (!check_file_name(WRQ_msg_inst.File_Name))
			{
				int error_msg_size = sizeof(ERROR_msg_inst);
				char error_msg[] = "Illegal WRQ";
				strcat(ERROR_msg_inst.ErrMsg, error_msg);
				ERROR_msg_inst.ErrorCode = htons(4); 
				int current_ack_opcode = htons(ERROR_OPCODE);
				ACK_msg_inst.Opcode = current_ack_opcode;
				if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
				   perror("TTFTP_ERROR: error while sending ack back to client");
				   exit(1);
				}      
				continue;
			}
        	
        	// check if there's a file with the same name
			if (is_valid_check_client)
			{
				int error_msg_size = sizeof(ERROR_msg_inst);
				char error_msg[] = "File already exists";
				strcat(ERROR_msg_inst.ErrMsg, error_msg);
				ERROR_msg_inst.ErrorCode = htons(6); 
				int current_ack_opcode = htons(ERROR_OPCODE);
				ACK_msg_inst.Opcode = current_ack_opcode;
				if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
				   perror("TTFTP_ERROR: error while sending ack back to client");
				   exit(1);
				}      
				continue;
			}
        	
            index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);
        	
        	// open new file
        	char* new_file_name = client_buffer.other; //only if we ignore Mode!!!! 514B
        	clients_array[index].fd = fopen(new_file_name, "w");
			if (clients_array[index].fd == NULL)
			{
				close(sock);
				perror("TTFTP_ERROR: error while opening the file");
				exit(1);
			}
			
        	// new client was added to array - sending ack
			int current_ack_opcode = htons(ACK_OPCODE);
			int current_ack_block_number = htons(0);
			ACK_msg_inst.Opcode = current_ack_opcode;
			ACK_msg_inst.Block_Number = current_ack_block_number;
			int ack_msg_size = sizeof(ACK_msg_inst);
			if (sendto(sock, &ACK_msg_inst, ack_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
			{
				fclose(clients_array[index].fd);
				close(sock);
				perror("TTFTP_ERROR: error while sending ack back to client");
				exit(1);
			}			
        } // first msg - wrq received
      
///////////////////////////////////////////////////////////////////////////////
//DATA
        
        else if (current_opcode == DATA_OPCODE)
        {
        	// make sure that client already exist
            index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);

        	if (index < 0)
        	{
        		int error_msg_size = sizeof(ERROR_msg_inst);
				char error_msg[] = "Unknown user";
				strcat(ERROR_msg_inst.ErrMsg, error_msg);
				ERROR_msg_inst.ErrorCode = htons(7); 
				int current_ack_opcode = htons(ERROR_OPCODE);
				ACK_msg_inst.Opcode = current_ack_opcode;
				if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
				   perror("TTFTP_ERROR: error while sending ack back to client");
				   exit(1);
				}            
				continue;
			}
        	
        	// receive data msg with client that is already exists
        	memcpy(&DATA_msg_inst, &client_buffer, recvMsgSize);
        	index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);
        	
        	// Block number unexpected
        	if (htons(DATA_msg_inst.Block_Number) != clients_array[index].block_number+1)
        	{
        		int error_msg_size = sizeof(ERROR_msg_inst);
				char error_msg[] = "Bad block number";
				strcat(ERROR_msg_inst.ErrMsg, error_msg);
				ERROR_msg_inst.ErrorCode = htons(0); 
				int current_ack_opcode = htons(ERROR_OPCODE);
				ACK_msg_inst.Opcode = current_ack_opcode;
				if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
				   perror("TTFTP_ERROR: error while sending ack back to client");
				   exit(1);
				}            
				continue;
        	}
        	
        	// ACK
			int current_ack_opcode = htons(ACK_OPCODE);
			ACK_msg_inst.Opcode = current_ack_opcode;
			int ack_msg_size = sizeof(ACK_msg_inst);
			index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);
        	// write to file
        	int lastWriteSize = fwrite(DATA_msg_inst.data, 1, recvMsgSize - 4, clients_array[index].fd);
			
        	// check if write fails
        	if (lastWriteSize != recvMsgSize - 4)
			{
				fclose(clients_array[index].fd);
				close(sock);
				perror("TTFTP_ERROR: fwrite was failed\n");
				exit(1);
			}
        	index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);

        	// last block
        	if (recvMsgSize < FULL_DATA_SIZE)
        	{
        		index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);
				clients_array[index].block_number++;
				int current_ack_block_number = htons(clients_array[index].block_number);
				ACK_msg_inst.Block_Number = current_ack_block_number;
        		
        		if (sendto(sock, &ACK_msg_inst, ack_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
					fclose(clients_array[index].fd);
					close(sock);
					perror("TTFTP_ERROR: error while sending ack back to client");
					exit(1);
				}
				
				// close file
				fclose(clients_array[index].fd);
				
				// delete client from clients array
				char empty_array[14] = {0};
				memcpy(clients_array[index].sa_data, empty_array, 14);
				clients_array[index].block_number = 0;
				clients_array[index].fd = NULL;
				// we don't remove File_Name - will be used to check if file exists
        	}

        	// not last block
        	if (recvMsgSize == FULL_BLOCK_SIZE)
			{
        		index = is_client_exist(client_buffer, (struct sockaddr*) & echoClntAddr);
				clients_array[index].block_number++;
				int current_ack_block_number = htons(clients_array[index].block_number);
				ACK_msg_inst.Block_Number = current_ack_block_number;
        		
        		if (sendto(sock, &ACK_msg_inst, ack_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
				{
					fclose(clients_array[index].fd);
					close(sock);
					perror("TTFTP_ERROR: error while sending ack back to client");
					exit(1);
				}				
			}
        }
        
///////////////////////////////////////////////////////////////////////////////
		
        // illegal opcode
		else
		{
			int error_msg_size = sizeof(ERROR_msg_inst);
			char error_msg[] = "Illegal TFTP operation";
			strcat(ERROR_msg_inst.ErrMsg, error_msg);
			ERROR_msg_inst.ErrorCode = htons(4); 
			int current_ack_opcode = htons(ERROR_OPCODE);
			ACK_msg_inst.Opcode = current_ack_opcode;
			if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & echoClntAddr, sizeof(echoClntAddr)) < 0) 
			{
			   perror("TTFTP_ERROR: error while sending ack back to client");
			   exit(1);
			}            
			continue;
		}
        
        
////////////////////////////////////////////////////////////////////////////////////////////
        
        select_ret = select(sock + 1, &FDSet, NULL, NULL, &wait_time);
		
		//error with select
		if (select_ret < 0)
		{
			perror("TTFTP_ERROR: select returns error\n");
			exit(1); // check if continue
		}
		
		//time expired 
		else if (select_ret == 0)
		{
			for (int i=0; i<CLIENTS; i++)
			{
				if (clients_array[i].fd != NULL)
				{
					clients_array[i].num_of_timeouts++;
					gettimeofday(&clients_array[i].client_tv, NULL);
					if(clients_array[i].num_of_timeouts == number_of_failures)
					{
						int error_msg_size = sizeof(ERROR_msg_inst);
						char error_msg[] = "Abandoning file transmission";
						strcat(ERROR_msg_inst.ErrMsg, error_msg);
						ERROR_msg_inst.ErrorCode = htons(0); 
						int current_ack_opcode = htons(ERROR_OPCODE);
						ACK_msg_inst.Opcode = current_ack_opcode;
						if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & clients_array[i].addr, sizeof(clients_array[i].addr)) < 0) 
						{
						   perror("TTFTP_ERROR: error while sending ack back to client");
						   exit(1);
						}            
						continue;
					}
					if (sendto(sock, &ACK_msg_inst, sizeof(ACK_msg_inst), 0, (struct sockaddr*) & clients_array[i].addr, sizeof(clients_array[i].addr)) < 0) 
					{
						fclose(clients_array[i].fd);
						close(sock);
						perror("TTFTP_ERROR: error while sending ack back to client");
						exit(1);
					}
				}
			}
		}
			
		else if (select_ret > 0)	
		{	
			gettimeofday(&current_select_tv, NULL);
			for (int i=0; i<CLIENTS; i++)
			{
				if (clients_array[i].fd != NULL)
				{
					if (((int)current_select_tv.tv_sec - (int)clients_array[i].client_tv.tv_sec) > wait_for_packet_time)
					{
						clients_array[i].num_of_timeouts++;
						gettimeofday(&clients_array[i].client_tv, NULL);
						if(clients_array[i].num_of_timeouts == number_of_failures)
						{
							int error_msg_size = sizeof(ERROR_msg_inst);
							char error_msg[] = "Abandoning file transmission";
							strcat(ERROR_msg_inst.ErrMsg, error_msg);
							ERROR_msg_inst.ErrorCode = htons(0); 
							int current_ack_opcode = htons(ERROR_OPCODE);
							ACK_msg_inst.Opcode = current_ack_opcode;
							if (sendto(sock, &ERROR_msg_inst, error_msg_size, 0, (struct sockaddr*) & clients_array[i].addr, sizeof(clients_array[i].addr)) < 0) 
							{
							   perror("TTFTP_ERROR: error while sending ack back to client");
							   exit(1);
							}            
							continue;
						}
						if (sendto(sock, &ACK_msg_inst, sizeof(ACK_msg_inst), 0, (struct sockaddr*) & clients_array[i].addr, sizeof(clients_array[i].addr)) < 0) 
						{
							fclose(clients_array[i].fd);
							close(sock);
							perror("TTFTP_ERROR: error while sending ack back to client");
							exit(1);
						}
					}
				}
			}						
		}
    }
    return 0; 
}

