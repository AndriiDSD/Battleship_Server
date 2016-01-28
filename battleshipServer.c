#include <stdlib.h>
#include <ctype.h>
#include "csapp.h"
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/select.h>

void playGame(int);
int checkLocation(int , int, int, int );
void printBoard();
void getIntLocation(int *row,int *col, int *dir, char *location);
int strToInt();


int socketFD;
int pcBoard[10][10];
int playerBoard[10][10];

int main(int argc, char **argv)
{
  
  /* Check the number of arguments */
  if (argc != 2) 
    {
      fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
      exit(1);
    }
  
  int port=strToInt(argv[1]);

  if(port<1024||port>65535) //if proxy port is not in the range of valid ports quit the program
    {
      perror("Non Valid Port\n");
      exit(1);
    }
  socketFD=0;
    if((socketFD=socket(AF_INET,SOCK_STREAM,0))==-1)
      {
	fprintf(stderr,"%s :socket error\n",argv[0]);
	exit(1);
      }
    int yes=1;
    if(setsockopt(socketFD,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1)//reuse address
      {
	perror("setsockopt error\n");//failed to set socket options
	close(socketFD);
	exit(1);
      }
    
    struct sockaddr_in servAddr; //set up the client connection
    bzero(&servAddr,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(port);

    if(bind(socketFD,(struct sockaddr *)&servAddr,sizeof(servAddr))==-1)//bind the socket to address
    {
      fprintf(stderr,"bind %s\n", strerror (errno));
      close(socketFD);
      exit(1);
    }

    if(listen(socketFD,5)==-1)//listen to requests
    {
      fprintf(stderr,"listen %s\n", strerror (errno));
      close(socketFD);
      exit(1);
    }

    while(1)
      {
	struct sockaddr_in clientAddr;
	socklen_t clientLen;
	int clientFD=-1;
	pid_t processID;
	int status;
	
	clientFD=accept(socketFD,(struct sockaddr *)&clientAddr,&clientLen);//accep
	if(clientFD==-1)
	  {
	    fprintf(stderr,"accept %s\n", strerror (errno));
	    exit(1);
	  }
	if((processID=fork())==0)//create child process
	  {
	    //int client=clientFD;
	    // while(1)
	    // {
		playGame(clientFD);//handle individual client requests
		//}
	  }
	//if((processID=waitpid(processID,&status,0))<0)
	// {
	//   fprintf(stderr," %s\n", strerror (errno));
	//   exit(1);
	// }
	
      }
}

void playGame(int clientFD)
{
  while(1)
    {
      int column,row,direction;
      char location[10];
      int i,j;
      for(i=0;i<10;i++)
	{
	  for(j=0;j<10;j++)
	    {
	      pcBoard[i][j]='~';
	      playerBoard[i][j]='~';
	    }
	}
      
      srand(time(NULL));
      
      for(i=1;i<11;i++)
	{
	  int size=-1;
	  switch(i)
	    {
	    case 1:size=5;break;
	    case 2:size=4;break;
	    case 3:size=4;break;
	    case 4:size=3;break;
	    case 5:size=3;break;
	    case 6:size=3;break;
	    case 7:size=2;break;
	    case 8:size=2;break;
	    case 9:size=2;break;
	    case 10:size=2;break;
	    }
	  
	  
	  //make random location
	  column=-1;
	  row=-1;
	  direction=-1;
	  
	  direction=rand()%2;
	  row=rand()%10+1;
	  column=rand()%10+1;
	  
	  int valid=checkLocation(row,column,direction,size);
	  if(valid)
	    {
	      //place current ship
	      if(direction==0)//horizontal
		{
	      int i;
	      for(i=column;i<=(size+column-1);i++)
		{
		  
		  pcBoard[row-1][i-1]='_';
		}
	      
		}
	  else//vertical
	    {
	      int i;
	      for(i=row;i<=(size+row-1);i++)
		{
		  
		  pcBoard[i-1][column-1]='|';
		}
	    }
	      //myBoard[2][3]='x';//4'th col, 3'rd row
	      // printBoard();
	    }
	  else
	    {
	      // printf("Non valid location.Enter again!\n");
	      i--;
	      continue;
	    }
	}
      //printBoard();
      //placed all ships now play with user
      while(1)
	{
	  recv(clientFD,location,sizeof(location),0);//get response
	  
	  //printf("1recv - %s \n",location);
	  if(strncmp(location,"quit",strlen("quit"))==0)
	    {
	      close(clientFD);
	      exit(0);
	    }
	  if(strncmp(location,"over",strlen("over"))==0)
	    {
	      break;
	    }
	  getIntLocation(&row,&column,&direction,location);
	  if(row<1&&row>10)
	    {
	      printf("Invalid location\n");
	      continue;
	    }
	  if(column<1&&column>10)
	    {
	      printf("Invalid location\n");
	      continue;
	    }
	  if(pcBoard[row-1][column-1]=='_'||pcBoard[row-1][column-1]=='|')
	    {
	      //printf("Player hit\n");
	      pcBoard[row-1][column-1]='X';
	      strncpy(location,"hit",strlen("hit"));
	      write(clientFD,location,sizeof(location));
	      // printf("2send %s\n",location);
	    }
	  else
	    {
	      // printf("Player missed\n");
	      pcBoard[row-1][column-1]='O';
	      strncpy(location,"miss",strlen("miss"));
	      write(clientFD,location,sizeof(location));
	      // printf("3send %s\n",location);
	    }
	  
	  //now the pc shoots randomly
	  bzero(location,sizeof(location));
	  while(1)
	    {
	      column=-1;
	      row=-1;
	      
	      row=rand()%10+1;
	      column=rand()%10+1;
	  
	      switch(row)
		{
		case 1:location[0]='a';break;
		case 2:location[0]='b';break;
		case 3:location[0]='c';break;
		case 4:location[0]='d';break;
		case 5:location[0]='e';break;
		case 6:location[0]='f';break;
		case 7:location[0]='g';break;
		case 8:location[0]='h';break;
		case 9:location[0]='i';break;
		case 10:location[0]='j';break;
		}
	      switch(column)
		{
		case 1:location[1]='1';break;
		case 2:location[1]='2';break;
		case 3:location[1]='3';break;
		case 4:location[1]='4';break;
		case 5:location[1]='5';break;
		case 6:location[1]='6';break;
		case 7:location[1]='7';break;
		case 8:location[1]='8';break;
		case 9:location[1]='9';break;
		case 10:location[1]='1';location[2]='0';break;
		}
	      // location[3]='\0';
	      
	      //check if shot at this location before
	      if(playerBoard[row-1][column-1]=='X'||playerBoard[row-1][column-1]=='O')
		continue;//already shot, choose different
	      else
		break;
	    }
	  
	  write(clientFD,location,sizeof(location));
	  //printf("4send %s\n",location);
	  playerBoard[row-1][column-1]='X';//mark as shot location
	}
      
      //printf("game over\n");
      recv(clientFD,location,sizeof(location),0);//get response
      //printf("5recv %s\n",location);
      if(strncmp(location,"quit",strlen("quit"))==0)
	{
	  //printf("close connection\n");
	  fflush(stdout);
	  close(clientFD);
	  exit(0);
	}
    }
}


int checkLocation(row,col,direction,size)
{
  // printf("row-%d,col-%d,dir-%d,size-%d \n",row,col,direction,size);
  if(row<1&&row>10)
    {
      // printf("out of bounds\n");
      return 0;
    }
  if(col<1&&col>10)
    {
      //printf("out of bounds\n");
      return 0;
    }
  if(direction!=0&&direction!=1)
    {
      //printf("out of bounds\n");
      return 0;
    }
  if(size<2&&size>5)
        {
	  //  printf("out of bounds\n");
	  return 0;
	}
  //printf("in bounds\n");
  int valid=1;
  if(direction==0)
    {
      //check for bounds
      if((col+size-1)>10)
	{
	  return 0;
	}
      int i;
      for(i=0;i<size;i++)
	{
	  if(pcBoard[row-1][col+i-1]=='_'||pcBoard[row-1][col+i-1]=='|')
	    return 0;
	}
    }
  else if(direction==1)
    {
      //check for bounds
      if((row+size-1)>10)
	{
	  return 0;
	}
      int i;
      for(i=0;i<size;i++)
	{
	   if(pcBoard[row+i-1][col-1]=='_'||pcBoard[row+i-1][col-1]=='|')
	    return 0;
	}
    }
  // printf("got here\n");
  return valid;
}

void printBoard()
{
    int i,j;
    printf("----------------------------------------\n");
    printf("pc's Board\n");
  for(i=0;i<10;i++)
    {
      for(j=0;j<10;j++)
	{
	  printf(" %c ",pcBoard[i][j]);
	}
      printf(" %c  | \n",(65+i));
      
    }
  printf(" 1  2  3  4  5  6  7  8  9  10    |\n");
  printf("----------------------------------------\n");
}
void getIntLocation(int *r,int *c, int *d, char *location)
{
  int column=*r;
  int row=*c;
  int direction=*d;
  
  char temp=location[0];
  if(temp>=65&&temp<=90)
    row=temp-64;
  else if(temp>=97&&temp<=122)
    row=temp-96;
  else
	row=-1;
  
  temp=location[1];
  temp=temp-48;
  int t=0;
  if(temp>=1&&temp<=9)//digit 1-9
    {
      if(temp==1)
	{
	  if(location[2]=='0')
	    {
	      column=10;
	      t=1;
	    }
	      else 
		column=1;
	}
      else
	column=temp;
    }
  else
    column=-1;
      if(!t)
	{
	  if(location[3]=='H'||location[3]=='h')//get direction
	    direction=0;
	  else if(location[3]=='V'||location[3]=='v')
	    direction=1;
	}
      else
	{
	  if(location[4]=='H'||location[4]=='h')//get direction
	    direction=0;
	  else if(location[4]=='V'||location[4]=='v')
	    direction=1;
	}
      *r=row;
      *c=column;
      *d=direction;
}

int strToInt(char *st)
{
  int x=0;
  if(st==NULL)
    return -1;
  if(*st=='\0')
    return -1;
  char *ch;
  for(ch=st;*ch!='\0';ch++)
    {
      if(!isdigit(*ch))
	x=-1;
    }
  if(x!=-1)
    x=(int)atoi(st);
  return x;
}
