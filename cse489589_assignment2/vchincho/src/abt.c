#include "../include/simulator.h"
#define TIMEOUT 20.0
#define BUFFER_MSG 1000
#define DEFAULT_ACK 111
#define NULL 0
#define   A    0
#define   B    1

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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/


struct pkt cur_packet;

//sequence numbers
int A_seqnum = 0;
int B_seqnum = 0;

//states
int waiting_pkt=0;
int waititng_ack=1;
int sender_state=0;

//buffer
struct node {
  struct msg message;
  struct node *next;
};
struct node *list_head = NULL;
struct node *list_end = NULL;


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{ 
  int checksum = 0;
  struct node *n;

  append_msg(&message);//add msg to buffer
  if(sender_state != waiting_pkt)
  {
    //do nothing
    return;
  }
  /* set current package to not finished */
  sender_state = waititng_ack;
  
  n = pop_msg();
  
  if(n == NULL)
  {
    printf("No message need to process\n");
    return 0;
  }

  for (int i=0; i<20; i++)
  {
    cur_packet.payload[i] = n->message.data[i];
  }
  free(n);
  cur_packet.seqnum = A_seqnum;
  cur_packet.acknum = DEFAULT_ACK;
  checksum = calc_checksum(&cur_packet);
  cur_packet.checksum = checksum;
  tolayer3(A, cur_packet);

  starttimer(A, TIMEOUT);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(packet.checksum != calc_checksum(&packet))
  {
    return;
  }

   if(packet.acknum != A_seqnum)
  {
    return;
  }
  A_seqnum = (A_seqnum + 1) % 2;
  //stoptimer(A);
  sender_state = waiting_pkt;

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  if(sender_state == waititng_ack)
  {
    tolayer3(A, cur_packet);
    starttimer(A, TIMEOUT);
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  if(packet.checksum != calc_checksum(&packet))
  {
    return;
  }

  if(packet.seqnum != B_seqnum)
  {
   //Duplicated packet do nothing
  }
  /* normal package, deliver data to layer5 */
  else
  {
    B_seqnum = (B_seqnum + 1)%2;
    tolayer5(B, packet.payload);
    //Debug_Log(B, "Send packet to layer5", &packet, NULL);
  }

  /* send back ack */
  packet.acknum = packet.seqnum;
  packet.checksum = calc_checksum(&packet);

  tolayer3(B, packet);
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}

///////////////////////////////////////////////////
//basic checksum function
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
  struct node *n = malloc(sizeof(struct node));
  if(n == NULL) {
    printf("no enough memory\n");
    return;
  }
  n->next = NULL;
  for(i = 0; i < 20; ++i) {
    n->message.data[i] = m->data[i];
  }

  if(list_end == NULL) 
  {
    list_head = n;
    list_end = n;
    return;
  }
  list_end->next = n;
  list_end = n;
}
struct node *pop_msg()
{
  struct node *p;
  if(list_head == NULL) {
    return NULL;
  }
  p = list_head;
  list_head = p->next;
  if(list_head == NULL) {
    list_end = NULL;
  }
  return p;
}