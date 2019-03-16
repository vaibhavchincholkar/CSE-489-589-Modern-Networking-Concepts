#include "../include/simulator.h"
#define NULL 0
#define   A    0
#define   B    1
/* timeout for the timer */
#define TIMEOUT 30.0
#define DEFAULT_ACK 111

/*All function declarations*/
int calc_checksum(struct pkt *p);
void append_msg(struct msg *m);
struct node *pop_msg();
/*end*/
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int B_seqnum = 0;//sequence count for B
int nextseq=0;//sequence count for A
int pkt_in_window=0;// no of packets in A's window 
int  WINDOW=0;//WINDOW size it will be initialized in the init of the A
struct pkt *packets;//creating a pointer to array of packets which are equal to window size
//memmory will be allocted to packets in init method of A

//buffer
struct node {
  struct msg message;
  struct node *next;
};
struct node *list_head = NULL;
struct node *list_end = NULL;

int window_start = 0;//this is the packet for which we are waiting for ack
int last=0;//last tranmitted packet from the window
int waiting_ack=0;// Ack A's waiting for

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  printf("\n================================ Inside A_output===================================\n");
  struct node *n;
  append_msg(&message);
  if(pkt_in_window == WINDOW)//check if window is full
  {
    return;
  }
  n = pop_msg();
  if(n == NULL)
  {
    printf("No message need to process\n");
    return 0;
  }
  if(((last+1)%WINDOW)==window_start)
  {
    return;
  } 
  else
  {
    if(pkt_in_window!=0)//increment last pointer by 1 if there is already packet on last
    {
       last=(last+1)%WINDOW;
    }
  }
  packets[last];//this is the packet we selected
  
  for (int i=0; i<20; i++)
  {
    packets[last].payload[i] = n->message.data[i];
  }
  //free the memory of n
  free(n);
  packets[last].seqnum = nextseq;
  packets[last].acknum = DEFAULT_ACK;
  packets[last].checksum = calc_checksum(&packets[last]);
  nextseq++;
  //update the number of packets in window
  pkt_in_window++;
  tolayer3(A, packets[last]);
  if(window_start==last)
  {
    starttimer(A, TIMEOUT);
  }
  return 0;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  printf("\n================================ Inside A_input===================================\n");

  if(packet.checksum != calc_checksum(&packet))
  {
    printf("wrong checksum\n");
    return;
  }
  if(packet.acknum != packets[window_start].seqnum) //check if pkt ack is equal to waiting ack
  {
    printf("expected ack:%d \n wrong ack:%d\n",packets[window_start].seqnum,packet.acknum);
    return;
  }
  packets[window_start].seqnum=-1;//set seq no of that packet to -1
  stoptimer(A);
  pkt_in_window--;//decrement number of packets in window
  
  if(pkt_in_window==0)
  {
    struct node *n;
    n=pop_msg();
    while(n!=NULL)
    {  
      packets[last];
      for (int i=0; i<20; i++)
      {
        packets[last].payload[i] = n->message.data[i];
      }
      //free the memory of n
      free(n);

      packets[last].seqnum = nextseq;
      packets[last].acknum = DEFAULT_ACK;
      packets[last].checksum = calc_checksum(&packets[last]);
      nextseq++;
      //update the number of packets in window
      pkt_in_window++;
      tolayer3(A, packets[last]);
      starttimer(A, TIMEOUT);
    }
  }
  else
  {
    window_start=(window_start+1)%WINDOW;
    struct node *n;
    n=pop_msg();
    if(n!=NULL)
    {
      last=(last+1)%WINDOW;  
      packets[last];
      for (int i=0; i<20; i++)
      {
        packets[last].payload[i] = n->message.data[i];
      }
      //free the memory of n
      free(n);

      packets[last].seqnum = nextseq;
      packets[last].acknum = DEFAULT_ACK;
      packets[last].checksum = calc_checksum(&packets[last]);
      nextseq++;
      //update the number of packets in window
      pkt_in_window++;
      tolayer3(A, packets[last]);
    }
  }

  if(window_start != last || pkt_in_window==1)
  {
    starttimer(A, TIMEOUT);
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("\n================================ Inside A_timerinterrupt===================================\n");

  int i=window_start;
  printf("expecting ack:%d\n",packets[window_start].seqnum);
  while(i!=last)
  {
    printf("sending seq no:%d\n",packets[i].seqnum);
    tolayer3(A, packets[i]);
    i=(i+1)%WINDOW;
  }
   printf("sending seq no:%d\n",packets[i].seqnum);
  tolayer3(A, packets[i]);
 
  /* If there is still some packets, start the timer again */
  if(window_start != last || pkt_in_window==1)
  {
    starttimer(A, TIMEOUT);
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  WINDOW=getwinsize();//initialize the window varibale
  packets = malloc(sizeof(struct pkt) * WINDOW);//aalocate memory for packtes in window
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  printf("\n================================ Inside B_input===================================\n");

  if(packet.checksum != calc_checksum(&packet))
  {
    printf("Packet is corrupted");
    return;
  }
  printf("Expected seq:%d\n",B_seqnum);
  if(packet.seqnum == B_seqnum)
  {
    printf("Correct packet received sending it to layer 5");
    ++B_seqnum;
    tolayer5(B, packet.payload);
  }
  else
  {
    printf("pkt seq:%d\n",packet.seqnum);
    printf("out of order packet");
    if(packet.seqnum < B_seqnum)
    {
       printf("sent ack:%d\n",packet.seqnum);
      packet.acknum = packet.seqnum ;
      packet.checksum = calc_checksum(&packet);
      tolayer3(B, packet);
    }
  }
  /*if(B_seqnum - 1 >= 0)
  {
    packet.acknum = B_seqnum-1;  resend the latest ACK 
    packet.checksum = calc_checksum(&packet);
    tolayer3(B, packet);
  }*/
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}

//////////////////////////////////////////////////////////////////////////////
int calc_checksum(struct pkt *p)
{
  int i;
  int checksum = 0;

  if(p == NULL)
  {
    return checksum;
  }
  for (i=0; i<20; i++)
  {
    checksum += (unsigned char)p->payload[i];
  }
  checksum += p->seqnum;
  checksum += p->acknum;
  return checksum;
}
void append_msg(struct msg *m)
{
  int i;
  /*allocate memory*/
  struct node *n = malloc(sizeof(struct node));
  if(n == NULL) {
    printf("no enough memory\n");
    return;
  }
  n->next = NULL;
  /*copy packet*/
  for(i = 0; i < 20; ++i) {
    n->message.data[i] = m->data[i];
  }

  /* if list empty, just add into the list*/
  if(list_end == NULL) 
  {
    list_head = n;
    list_end = n;
    return;
  }
  /* otherwise, add at the end*/
  list_end->next = n;
  list_end = n;
}
struct node *pop_msg()
{
  struct node *p;
  /* if the list is empty, return NULL*/
  if(list_head == NULL) {
    return NULL;
  }

  /* retrive the first node*/
  p = list_head;
  list_head = p->next;
  if(list_head == NULL) {
    list_end = NULL;
  }
  return p;
}