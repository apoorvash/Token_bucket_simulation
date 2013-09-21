#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include "my402list.h"

typedef struct myPacketInfoStructure {
	 
	int packet_no; 
	double pkt_arrival;
	int P;
	double pkt_service;
	double Q1_entry_time;
	double Q2_entry_time;
	
}MyPacketInfo;

int i = 1;
int num = 20;
int num_afterdrop = 0;
int B = 10;
int P = 3;
double r = 1.5;
double lambda = 0.5;
double mu = 0.35;

int packet_no = 0;
int packet_count_Q1exit = 0;
int packet_Q1enter = 0;
int packet_count_Q2exit = 0;
int token_no = 0;
int Q2_packet_count = 0;
int token_counter = 0;
int drop_packets = 0;
int drop_pkts = 0;
int drop_tokens = 0;
int Q2_length = 0;
int server_count = 0;

FILE *fp = NULL;
int file_flag = 0;
char buf[1026];
char *a[3];
char *str = NULL;
char *file_name = NULL;


double interarr_time = 0;
double service_time = 0;
double time_in_Q1 = 0;
double time_in_Q2 = 0;
double time_in_S = 0;
double time_sd = 0;
double time_in_sys = 0;
double time_in_sys_sq = 0;


pthread_mutex_t mut_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mut_lock2 = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t signal_server = PTHREAD_COND_INITIALIZER;
pthread_cond_t terminate_server = PTHREAD_COND_INITIALIZER;

My402List Queue_one;
My402List Queue_two;

struct timeval tim;
double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14;

pthread_t arrival_thread;
pthread_t token_thread;
pthread_t server_thread;
pthread_t end_server;

void *packet_arrival();
void *token_bucket_func();
void *server_func();
void *server_termination_func();


void print_statistics();
void make_packets();
MyPacketInfo *Queue_data = NULL;


//Arrival Thread
void *packet_arrival()
{
	My402ListElem *Queue_elem = NULL;
	MyPacketInfo *Q1_head_data = NULL;
	num_afterdrop = num;
	//printf("num :%d\n",num);
	for ( i = 1 ; i <= num ; i++)
	{
		
		make_packets();
		
		gettimeofday(&tim,NULL);
		t2 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		
		double arrival_sleep = (Queue_data->pkt_arrival)*1000;
		int arrival_sleep1 = (int)arrival_sleep;
		usleep(arrival_sleep1);
		
		gettimeofday(&tim,NULL);
		t3 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		Queue_data->Q1_entry_time = t3;
		
		double inter_time = (t3-t2);
		interarr_time = interarr_time + inter_time;
		if ( Queue_data->P > B )
		{
			printf("%012.3fms: Packet p%d arrives, needs %d tokens, dropped\n", (t3-t1)*1000, Queue_data->packet_no, Queue_data->P);
			num_afterdrop --;
			pthread_mutex_lock(&mut_lock2);
			drop_packets ++;
			pthread_mutex_unlock(&mut_lock2);
		}
		else
		{
					printf("%012.3fms: Packet p%d arives, needs %d tokens, inter-arrival time: %.3f\n",(t3-t1)*1000, Queue_data->packet_no, Queue_data->P, Queue_data->pkt_arrival );
					My402ListAppend(&Queue_one, (void*)Queue_data);
					packet_Q1enter ++;
					gettimeofday(&tim,NULL);
					t4 = tim.tv_sec + (tim.tv_usec)/1000000.0;
					printf("%012.3fms: Packet p%d enters Q1\n",(t4-t1)*1000, Queue_data->packet_no);
					Queue_elem = My402ListFirst(&Queue_one);
					Q1_head_data = (MyPacketInfo*)Queue_elem->obj;
					
					pthread_mutex_lock(&mut_lock);
					if ( Q1_head_data->P <= token_counter )   //check for tokens
					{
						
						
						token_counter = token_counter - Q1_head_data->P;
						
						My402ListUnlink(&Queue_one, Queue_elem);
						packet_count_Q1exit ++;
	
						gettimeofday(&tim,NULL);
						t5 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		
						double time_Q1 = (t5-Q1_head_data->Q1_entry_time);
						time_in_Q1 = time_in_Q1 + time_Q1; 
						
						printf("%012.3fms: Packet p%d leaves Q1, time in Q1: %.3fms, token bucket now has %d tokens\n",(t5-t1)*1000, Q1_head_data->packet_no, (t5-(Q1_head_data->Q1_entry_time))*1000, token_counter);
						
						Q1_head_data->Q2_entry_time = t5;
						
						My402ListAppend(&Queue_two,(void*)Q1_head_data);
						pthread_cond_signal(&signal_server);		//signal server when appending to Q2
					
						
						gettimeofday(&tim,NULL);
						t6 = tim.tv_sec + (tim.tv_usec)/1000000.0;
						printf("%012.3fms: Packet p%d enters Q2\n", (t6-t1)*1000, Q1_head_data->packet_no);
						
					}	
					pthread_mutex_unlock(&mut_lock);
		}
	}
	pthread_mutex_lock(&mut_lock2);
	drop_pkts = drop_packets;
	if ( drop_pkts == num )
	{
		pthread_mutex_unlock(&mut_lock2);
		pthread_cond_signal(&terminate_server);
	}	
	else 
	{
		pthread_mutex_unlock(&mut_lock2);
		int ret7 = pthread_cancel(end_server);
		if( ret7 != 0 )
		{	
			fprintf(stderr,"Pthread_cancel failed\n");
			exit(1);
		}
	}
	
	return 0;
}

//puts packet information into the structure
void make_packets()    
{
	if( file_flag == 1)
	{
		int j = 0;
		char delim[] = " \t";
		fgets( buf, sizeof(buf), fp );
		str = strtok(buf,delim);
		while( str != NULL)
		{	
			a[j] = str;
			j++;
			str = strtok( NULL,delim);
		}
		
		char *p_a = a[0];
		char *r_t = a[1];
		char *p_s = a[2];
		
		double pkt_arr = atof(p_a);
		int token_req = atoi(r_t);
		double pkt_ser = atof(p_s);
		
		Queue_data = (MyPacketInfo*)malloc(sizeof(MyPacketInfo));
		packet_no = packet_no + 1;
		Queue_data->packet_no = packet_no;
		Queue_data->pkt_arrival = pkt_arr;
		Queue_data->P = token_req;
		Queue_data->pkt_service = pkt_ser;
		Queue_data->Q1_entry_time = 0.0;
		Queue_data->Q2_entry_time = 0.0;
		
	}
	else
	{
		Queue_data = (MyPacketInfo*)malloc(sizeof(MyPacketInfo));
		packet_no = packet_no + 1;
		Queue_data->packet_no = packet_no;
		double packet_arr = (1000)/lambda;
		if( packet_arr > 10000 )
		{	
			packet_arr = 10000.0;
		}
		Queue_data->pkt_arrival = packet_arr;
		Queue_data->P = P;
		double s_time = (1000)/mu;
		if( s_time > 10000 )
		{
			s_time = 10000.0;
		}
		Queue_data->pkt_service = s_time;
		Queue_data->Q1_entry_time = 0.0;
		Queue_data->Q2_entry_time = 0.0;
	}
}


//Token Bucket Thread
void *token_bucket_func()
{
	My402ListElem *Queue1_elem = NULL;
	MyPacketInfo *Queue1_head_data = NULL;
	
	double token_arrival = (1000/r);
	
	double token_sleep = token_arrival *1000;
	int token_sleep1 = (int)token_sleep; 
	
	while(1)
	{	

			usleep(token_sleep1);
			
			gettimeofday(&tim,NULL);
			t7 = tim.tv_sec + (tim.tv_usec)/1000000.0;
			if ( token_counter > (B-1) )
			{
				printf("%012.3fms: Token t%d arrives, dropped\n",(t7-t1)*1000,token_no);
				drop_tokens ++;
				token_no ++;
			}
			else
			{
				pthread_mutex_lock(&mut_lock);
				token_counter ++;	
				token_no ++;
	
				printf("%012.3fms: Token t%d arrives, token bucket now has %d tokens\n", (t7-t1)*1000, token_no, token_counter);
				pthread_mutex_unlock(&mut_lock);
			
				if(!My402ListEmpty(&Queue_one))
				{
					Queue1_elem = My402ListFirst(&Queue_one);
					Queue1_head_data = (MyPacketInfo*)Queue1_elem->obj;
				
					pthread_mutex_lock(&mut_lock);
					if ( Queue1_head_data->P <= token_counter )
					{	
						token_counter = token_counter - Queue1_head_data->P;
					
						My402ListUnlink(&Queue_one, Queue1_elem);
						packet_count_Q1exit ++;
	
						gettimeofday(&tim,NULL);
						t9 = tim.tv_sec + (tim.tv_usec)/1000000.0;
						double time_Q1 = (t9-Queue1_head_data->Q1_entry_time);
						time_in_Q1 = time_in_Q1 + time_Q1; 
			
						printf("%012.3fms: Packet p%d leaves Q1, time in Q1: %.3fms, token bucket now has %d tokens\n",(t9-t1)*1000, Queue1_head_data->packet_no, (t9-(Queue1_head_data->Q1_entry_time))*1000, token_counter);
						
						Queue1_head_data->Q2_entry_time = t9;
						My402ListAppend(&Queue_two,(void*)Queue1_head_data);
						pthread_cond_signal(&signal_server); //signals to the server when appending to Q2
						
						gettimeofday(&tim,NULL);
						t10 = tim.tv_sec + (tim.tv_usec)/1000000.0;
						printf("%012.3fms: Packet p%d enters Q2\n", (t10-t1)*1000, Queue1_head_data->packet_no);
						
					}
					pthread_mutex_unlock(&mut_lock);
				}
			}	
			if( packet_count_Q1exit == num_afterdrop )
			{
				break;
			}
	}
	return 0;
	
}


//Server Thread
void *server_func()
{
	while(1)
	{	
		
		pthread_mutex_lock(&mut_lock);
		
		if (My402ListEmpty(&Queue_two))
			pthread_cond_wait(&signal_server, &mut_lock);
		My402ListElem *Queue2_elem = NULL;
		MyPacketInfo *Q2_head_data = NULL;
			
		Queue2_elem = My402ListFirst(&Queue_two);
		Q2_head_data = (MyPacketInfo*)Queue2_elem->obj;
		My402ListUnlink(&Queue_two, Queue2_elem);
		gettimeofday(&tim,NULL);
		t14 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		printf("%012.3fms: Packet p%d begins service at S, time in Q2 = %.3fms\n",(t14-t1)*1000, Q2_head_data->packet_no, (t14-(Q2_head_data->Q2_entry_time))*1000);
		double time_Q2 = (t14-Q2_head_data->Q2_entry_time);
		time_in_Q2 = time_in_Q2 + time_Q2;
		gettimeofday(&tim,NULL);
		t12 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		
		packet_count_Q2exit ++;
		double server_sleep = (Q2_head_data->pkt_service) * 1000;
		int server_sleep1 = (int)server_sleep;
		pthread_mutex_unlock(&mut_lock);
		usleep(server_sleep1);
		gettimeofday(&tim,NULL);
		t8 = tim.tv_sec + (tim.tv_usec)/1000000.0;
		double serv_time = t8-t12;
		service_time = service_time + serv_time;
		server_count ++;
		double time_sys = (t8-(Q2_head_data->Q1_entry_time));
		time_in_sys = time_in_sys + time_sys;
		
		time_sd = time_sd + (( time_sys * time_sys )/( server_count * server_count ));
		
		printf("%012.3fms: Packet p%d departs S, service time: %.3fms, time in system: %.3fms\n",(t8-t1)*1000, Q2_head_data->packet_no, Q2_head_data->pkt_service,(t8-(Q2_head_data->Q1_entry_time))*1000);
		
		double time_S = (t8-t12);
		time_in_S = time_in_S + time_S;
		if( packet_count_Q2exit == num_afterdrop )
		{
			break;
		}
		else if( packet_Q1enter == 0)
		{
			break;
		}
	}
	return 0;
}


//Server And Token Termination Thread
void *server_termination_func()
{	
	pthread_mutex_lock(&mut_lock2);
	pthread_cond_wait(&terminate_server,&mut_lock2);
	pthread_mutex_unlock(&mut_lock2);	
		int ret5 = pthread_cancel(server_thread);
		if( ret5 != 0 )
		{
			fprintf(stderr,"Pthread_cancel failed\n");
			exit(1);
		}
		int ret6 = pthread_cancel(token_thread);
		if ( ret6 != 0 )
		{
			fprintf(stderr, "Pthread_cancel failed\n");
			exit(1);
		}
		return 0;
}	
	
	
//Display Statistics 
void print_statistics()
{
	double avg_time_sys = 0;
	printf("\nStatistics\n\n");
	double total_em_time = (t11-t1);

	double avg_inter_time = interarr_time/num;
	printf("Average packet inter-arrival time: %.6fs\n",avg_inter_time);
	if ( server_count == 0)
	{
		printf("Average packet service time: NA All packets are dropped\n\n");
	}
	else
	{
		double avg_service_time = service_time/server_count;
		printf("Average packet service time: %.6fs\n\n",avg_service_time);
	}
	double avg_pkts_Q1 = time_in_Q1/total_em_time;
	printf("Average number of packets in Q1: %.6f\n",avg_pkts_Q1);
	
	double avg_pkts_Q2 = time_in_Q2/total_em_time;
	printf("Average number of packets in Q2: %.6f\n",avg_pkts_Q2);
	
	double avg_pkts_S = time_in_S/total_em_time;
	printf("Average number of packets in S: %.6f\n\n",avg_pkts_S);
	
	if ( server_count == 0)
	{
		printf("Average time spent in system: NA All packets are dropped\n");
		printf("Standard Deviation for time spent in the system: NA All packets are dropped\n\n");
	}
	else
	{	
		avg_time_sys = time_in_sys/server_count;
		printf("Average time spent in system: %.6fs\n",avg_time_sys);
		double varience = (time_sd -((avg_time_sys)*(avg_time_sys)));
		double std_dev = sqrt(varience);
		printf("Standard Deviation for time spent in the system: %.6fms\n\n",std_dev);
	}
	
	

	if( token_no == 0)
	{
		printf("Token drop probability: NA Zero tokens are generated\n");
	}
	else
	{
		double token_drop_prob = (double)drop_tokens/(double)token_no;
		
		printf("Token drop probability: %.6f\n",token_drop_prob);
	}
		double packet_drop_prob = drop_packets/num;
		printf("Packet drop probability: %.6f\n\n",packet_drop_prob);
	
	
}


int main(int argc, char *argv[])
{
	
	int opt = 0;
	My402ListInit(&Queue_one);
	My402ListInit(&Queue_two);
	/*
     * I did not write this code. This code is derived from 
	 * http://linuxprograms.wordpress.com/2012/06/22/c-getopt_long_only-example-accessing-command-line-arguments/
     */
	static struct option long_options[] = {
        {"lambda", required_argument, 0, 'l' },
        {"mu", required_argument, 0,  'm' },
        {"r", required_argument, 0,  'r' },
        {"B", required_argument, 0,  'b' },
		{"P", required_argument, 0, 'p'},
		{"n", required_argument, 0, 'n'},
		{"t", required_argument, 0, 't'},
        {0,           0,                 0,  0   }
    };

	int long_index =0;
    while ((opt = getopt_long_only(argc, argv,"", 
                   long_options, &long_index )) != -1) {
        switch (opt) {
             case 'l' : lambda = atof(optarg);	
                 break;
             case 'm' : mu = atof(optarg);
                 break;
             case 'r' : r = atof(optarg); 
                 break;
             case 'b' : B = atoi(optarg);
                 break;
			 case 'p' : P = atoi(optarg);
				break;
			 case 'n' : num = atoi(optarg);
						num_afterdrop = num;
						if(num == 0)
						{
							fprintf(stderr,"Number of packets are 0\n");
							exit(1);
						}
				break;
			 case 't' : fp = fopen(optarg, "r");
						file_name = optarg;	

						if ( fp == NULL)
						{
						 fprintf(stderr,"Cannot open file %s to read\n",optarg);
						 exit(1);
						}
						file_flag = 1;
						fgets(buf, sizeof(buf), fp);
						num = atoi(buf);
						num_afterdrop = num;
				break;
             
        }
    }
	 /*
     * End code that I did not write.
     */
	if( file_flag == 1)
	{
		printf("\nEmulation Parameters:\n");
		printf("	r:%.2f\n",r);
		printf("	B:%d\n",B);
		printf("	tsfile:%s\n\n",file_name);
	}
	else
	{
		printf("\nEmulation Parameters:\n");
		printf("	lambda = %.2f\n",lambda);
		printf("	mu = %.2f\n",mu);
		printf("	r = %.2f\n",r);
		printf("	B = %d\n",B);
		printf("	P = %d\n",P);
		printf("	number to arrive = %d\n\n",num);

	}
	gettimeofday(&tim,NULL);
	t1 = tim.tv_sec + (tim.tv_usec)/1000000.0;

	printf("%012.3fms: Emulation Begins\n",(t1-t1)*1000);
	
	int ret1 = pthread_create(&arrival_thread, 0, packet_arrival, NULL);
	int ret2 = pthread_create(&token_thread, 0, token_bucket_func, NULL);
	int ret3 = pthread_create(&server_thread, 0, server_func, NULL);
	int ret4 = pthread_create(&end_server, 0, server_termination_func, NULL);
	
	if( ret1 != 0 )
	{
		fprintf(stderr,"Arrival thread creation failed\n");
		exit(1);
	}
	if( ret2 != 0 )
	{
		fprintf(stderr,"Token Thread creation failed\n");
		exit(1);
	}
	if( ret3 != 0 )
	{
		fprintf(stderr,"Server Thread creation failed\n");
		exit(1);
	}
	if( ret4 != 0 )
	{
		fprintf(stderr,"Server Thread creation failed\n");
		exit(1);
	}
	
	
	pthread_join(arrival_thread,0);
	pthread_join(token_thread,0);
	pthread_join(server_thread,0);
	pthread_join(end_server,0);
	
	gettimeofday(&tim,NULL);
	t11 = tim.tv_sec + (tim.tv_usec)/1000000.0;
	print_statistics();
	
	return 0;
}

