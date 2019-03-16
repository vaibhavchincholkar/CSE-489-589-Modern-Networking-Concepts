#include "../include/simulator.h"
#define NULL 0
#define   A    0
#define   B    1
/* timeout for the timer */
#define TIMEOUT 30.0
#define INTERVAL 1.0
#define DEFAULT_ACK 111

/*All function declarations*/
int calc_checksum(struct pkt *p);
void append_msg(struct msg *m);
struct node *pop_msg();
/*end*/
/*buffer*/
struct node {
  struct msg message;
  struct node *next;
};
struct node *list_head = NULL;
struct node *list_end = NULL;
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

/* called from layer 5, passed the data to be sent to other side */
struct sr_window
{
  struct pkt pi;//packet item
  int ack;
  int timeover;
};
struct sr_window *A_packets;
struct sr_window *B_packets;
int pkt_in_window=0;
int pkt_in_window_B=0;
int window_start = 0;
int window_start_B = 0;
int last=0;
int last_B=0;

//sequence numbers
int A_seqnum = 0;
int B_seqnum = 0;

int  WINDOW=0;

int temp=0;
float current_time=0;
int waitng_ack=0;
int is_timer_off=0;
void A_output(message)
  struct msg message;
{
  printf("\n================================ Inside A_output================================\n");
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
    if(pkt_in_window!=0)
    {
      last=(last+1)%WINDOW;
    }
  }
  A_packets[last];//the selected packet of the window
  for (int i=0; i<20; i++)
    {
      A_packets[last].pi.payload[i] = n->message.data[i];
    }
    free(n);
    A_packets[last].pi.seqnum = A_seqnum;
    A_packets[last].pi.acknum = DEFAULT_ACK;
    A_packets[last].pi.checksum = calc_checksum(&A_packets[last].pi);
    A_seqnum++;
    A_packets[last].timeover=current_time+TIMEOUT;
    A_packets[last].ack=0;//set ack to not received
    pkt_in_window++;//increase the number of packets in the window
    printf("sending seq no:%d\n",A_packets[last].pi.seqnum);
    tolayer3(A, A_packets[last].pi);
    if(is_timer_off==0)
    {
      is_timer_off=1;
      printf("Timer on\n");
      starttimer(A,INTERVAL);
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
  printf("waiting ack:%d\n",A_packets[window_start].pi.seqnum);
  if(packet.acknum == A_packets[window_start].pi.seqnum)//packets[window_start].seqnum)
  {
    printf("Received correct ack:%d of WS:%d\n",A_packets[window_start].pi.seqnum,window_start);
    A_packets[window_start].ack=1;
    pkt_in_window--;
    if(pkt_in_window==0)
    {
      window_start=(window_start+1)%WINDOW;
      last=(last+1)%WINDOW;
      printf("Window is empty nw\n");
      if(list_head!=NULL)
      {
        struct node *n;
         n=pop_msg();
         if(n!=NULL)
         {  
          A_packets[last];
          for (int i=0; i<20; i++)
          {
            A_packets[last].pi.payload[i] = n->message.data[i];
          }
          free(n);

          A_packets[last].pi.seqnum = A_seqnum;
          A_packets[last].pi.acknum = DEFAULT_ACK;
          A_packets[last].pi.checksum = calc_checksum(&A_packets[last].pi);
          printf("sending packet:%d\n",A_seqnum);
          printf("packets in window:%d\n",pkt_in_window);
          A_seqnum++;
          A_packets[last].ack=0;//set ack to not received
          A_packets[last].timeover=current_time+TIMEOUT;
          pkt_in_window++;//increase the number of packets in the window

          tolayer3(A, A_packets[last].pi);
        }
      }
      else
      {
        printf("Timer off");
        is_timer_off=0;
        stoptimer(A);
      } 
    }
    else
    {

      int i=window_start;
      while(i!=last)
      {
        int temp=(i+1)%WINDOW;
        if(A_packets[temp].ack!=1)
        {
          break;
        }
        pkt_in_window--;
        i=(i+1)%WINDOW;
        if(i==last)
        {
          last=i;
        }
      }
      window_start=(i+1)%WINDOW;
     if(pkt_in_window==0)
     {
      last=window_start;
     }
      printf("new WS:%d\n",window_start);
      printf("last:%d\n",last);
       printf("pkt in window:%d\n",pkt_in_window);
      //send packet from buffer
        struct node *n;
        n=pop_msg();
        if(n!=NULL)
        {  
          A_packets[last];
          for (int i=0; i<20; i++)
          {
            A_packets[last].pi.payload[i] = n->message.data[i];
          }
          free(n);

          A_packets[last].pi.seqnum = A_seqnum;
          A_packets[last].pi.acknum = DEFAULT_ACK;
          A_packets[last].pi.checksum = calc_checksum(&A_packets[last].pi);
          printf("sending packet:%d\n",A_seqnum);
          printf("packets in window:%d\n",pkt_in_window);
          A_seqnum++;
          A_packets[last].ack=0;//set ack to not received
          A_packets[last].timeover=current_time+TIMEOUT;
          pkt_in_window++;//increase the number of packets in the window

          tolayer3(A, A_packets[last].pi);
        }
      //done  
    }
  }
  else if (packet.acknum <= A_packets[window_start].pi.seqnum)
  {
    //duplicate acks
    printf("Received old ack:%d\n",packet.acknum);
  }
  else if (packet.acknum > A_packets[window_start].pi.seqnum )
  {
    //further widow packet ack    
    printf("Received future ack:%d\n",packet.acknum);
    int i=window_start;
    while(i!=last)
    {
      temp=(i+1)%WINDOW;
      if(packet.acknum == A_packets[temp].pi.seqnum)
      {
         printf("acked:%d\n",A_packets[temp].pi.seqnum);
        A_packets[temp].ack=1;
        break;
      }
      i=(i+1)%WINDOW;
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{ 
  
  current_time=current_time+INTERVAL;
  if(pkt_in_window != 0)
  {

    //printf("pkt in windw:%d\n",pkt_in_window);
    int i=window_start;
   // printf("Window start:%d\n",pkt_in_window);
    while(i!=last)
    {
      if(A_packets[i].ack==0&& A_packets[i].timeover<current_time)
      {
        printf("\n================================ Inside A_timerinterrupt===================================\n");
        printf("sending seq no:%d\n",A_packets[i].pi.seqnum);
        A_packets[i].timeover=current_time+TIMEOUT;
        tolayer3(A, A_packets[i].pi);
      }
      i=(i+1)%WINDOW;
    }
     if(A_packets[i].ack==0&& A_packets[i].timeover<current_time)
      {
        printf("\n========= Inside A_timerinterrupt==========\n");
        printf("sending seq no:%d\n",A_packets[i].pi.seqnum);
        A_packets[i].timeover=current_time+TIMEOUT;
        tolayer3(A, A_packets[window_start].pi);
      }
  }
  starttimer(A, INTERVAL);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  WINDOW=getwinsize();
  A_packets= malloc(sizeof(struct sr_window) * WINDOW);
  for(int i=0;i<WINDOW;i++)
  {
    A_packets[i].ack==0;
  }
  is_timer_off=1;
  starttimer(A, INTERVAL);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
   printf("\n================================ Inside B_input===================================\n");
   printf("Expected seq no:%d\n",B_seqnum);
  if(packet.checksum != calc_checksum(&packet))
  {
    printf("Packet is corrupted");
    return;
  }

  if(packet.seqnum == B_seqnum)
  {

    printf("Correct packet received sending it to layer 5");
    B_seqnum=B_seqnum+1;
    tolayer5(B, packet.payload);
    packet.acknum = B_seqnum-1; /* resend the latest ACK */
    packet.checksum = calc_checksum(&packet);
    tolayer3(B, packet);
    
    B_packets[window_start_B].timeover=(B_seqnum)+WINDOW-1;

    window_start_B=(window_start_B+1)%WINDOW;

    while(B_packets[window_start_B].pi.seqnum == B_seqnum)
    {
      tolayer5(B, B_packets[window_start_B].pi.payload);
      B_seqnum=B_seqnum+1;
      B_packets[window_start_B].timeover=(B_seqnum)+WINDOW-1;
      window_start_B=(window_start_B+1)%WINDOW;
    }
  }
  else
  {
    if(packet.seqnum>B_seqnum)
    {
      printf("Ack future window packets\n"); 
      if(packet.seqnum <= B_seqnum+WINDOW)
      { 
        for(int m=0;m<WINDOW;m++)
        {
           printf("timeover:%d and seqnum of pk:%d\n",B_packets[m].timeover,packet.seqnum);
          if(B_packets[m].timeover==packet.seqnum)
          {
             printf("storing seqnum:%d",packet.seqnum);
            B_packets[m].pi=packet;
            printf("stored seqnum:%d",B_packets[m].pi.seqnum);
            packet.acknum = packet.seqnum ; 
            packet.checksum = calc_checksum(&packet);
            tolayer3(B, packet);
            break;
          }
        }
      } 
    }
    else
    {
      printf("old packet sending old ack");
      packet.acknum = packet.seqnum ; 
      packet.checksum = calc_checksum(&packet);
      tolayer3(B, packet);
    }
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  WINDOW=getwinsize();
    B_packets= malloc(sizeof(struct sr_window) * WINDOW);
    for(int i=0;i<WINDOW;i++)
    {
      B_packets[i].timeover=i;//this is the sequence number here
    }
}


/*===============function definations==========================*/
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