#include <stdlib.h>
#include <ctype.h>
#include "csapp.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

int strToInt();
void enterUserName();
void printOptions();
void playGame();
void initiateGameBoard();
void placeShips();
void printBoard();
int checkLocation(int,int,int,int);
void quitGame();
void getIntLocation(int *row,int *col, int *dir, char *location);


char userName[20];
int myBoard[10][10];
int enemyBoard[10][10];
int socketFD;


int main(int argc, char **argv)
{

    /* Check the number of arguments */
    if (argc != 3) {
	fprintf(stderr, "Usage: %s <ip> <port number>\n", argv[0]);
	exit(1);
    }
    int port=strToInt(argv[2]); // the first argument is the proxy port
    if(port<1024||port>65535) //if proxy port is not in the range of valid ports quit the program
	  {
	    fprintf(stderr,"%s :Non Valid Port\n",argv[0]);
	    exit(1);
	  }
    ////////////////////////////
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
    
    struct hostent *dest;
    dest=gethostbyname(argv[1]);
    if(dest==NULL) //check if the server address is correct
      {
	fprintf(stderr,"gethostbyname %s\n", strerror (errno));
	close(socketFD);
	exit(1);
      }
    //fill in address structure and connect
    struct sockaddr_in serverAddr; //set up the client connection
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    memcpy(&serverAddr.sin_addr, dest->h_addr, dest->h_length);
    serverAddr.sin_port=htons(port);
    //printf("trying to connect\n");
    if(connect(socketFD,(struct sockaddr *)&serverAddr,sizeof(serverAddr))==-1)//bind the socket to address
      {
	    fprintf(stderr,"%s : %s\n",argv[0], strerror (errno));
	    close(socketFD);
	    exit(1);
      }
    //printf("connected \n");
    
    enterUserName();//the user supplies a user name
    
    while(1)
      {	
	char y[10];
	playGame();
	printf("Do you want to play another game?(y/n)\n");
	fgets(y,5,stdin);
	if(y[0]=='y'||y[0]=='Y')
	  {
	    //recv(socketFD,y,sizeof(y),0);
	    char *over="continue";
	    int n=write(socketFD,over,sizeof(over));
	    continue;
	    
	  }
	else
	  {
	    quitGame();
	  }
      }
    close(socketFD);
    return 0;
}
void initiateGameBoard()
{
  int i,j;
  for(i=0;i<10;i++)
    {
      for(j=0;j<10;j++)
	{
	  myBoard[i][j]='~';
	  enemyBoard[i][j]='~';
	}
    }
}
void placeShips()
{
  printf("Place your ships!\n");
  printf("Input the location and direction(ex. a1 h). Input quit to quit\n");
  int i;
  for(i=1;i<11;i++)
    {
      int size=-1;
      printf("Place the ");
      fflush(stdout);
      switch(i)
	{
	case 1:printf("Aircraft Carrier(size 5)\n");size=5;break;
	case 2:printf("Battleship(size 4)\n");size=4;break;
	case 3:printf("Battleship(size 4)\n");size=4;break;
	case 4:printf("Submarine(size 3)\n");size=3;break;
	case 5:printf("Submarine(size 3)\n");size=3;break;
	case 6:printf("Submarine(size 3)\n");size=3;break;
	case 7:printf("Destroyer(size 2)\n");size=2;break;
	case 8:printf("Destroyer(size 2)\n");size=2;break;
	case 9:printf("Destroyer(size 2)\n");size=2;break;
	case 10:printf("Destroyer(size 2)\n");size=2;break;
	}
      printf("\n");

      char location[10];
      fgets(location,10,stdin);

      if(strncmp(location,"quit",strlen("quit"))==0)
	{
	  quitGame();
	}

      int column=-1;
      int row=-1;
      int direction=-1;
      
      getIntLocation(&row,&column,&direction,location);

      //printf("row - %d, col - %d, direction - %d, size - %d\n",row,column,direction,size);
      int valid=checkLocation(row,column,direction,size);
      //valid=1;
      if(valid)
	{
	  //place current ship
	  if(direction==0)//horizontal
	    {
	      int i;
	      for(i=column;i<=(size+column-1);i++)
		{
		  
		  myBoard[row-1][i-1]='_';
		}

	    }
	  else//vertical
	    {
	      int i;
	      for(i=row;i<=(size+row-1);i++)
		{
		  
		  myBoard[i-1][column-1]='|';
		}
	    }
	  //myBoard[2][3]='x';//4'th col, 3'rd row
	  printBoard();
	}
      else
	{
	  printf("Non valid location.Enter again!\n");
	  i--;
	  continue;
	}
    }
  
}
void printBoard()
{
    int i,j;
    printf("----------------------------------------");
    fflush(stdout);
    printf("----------------------------\n");
    printf("%s's Board              ",userName);
    fflush(stdout);
     printf("       PC's Board\n");
  for(i=0;i<10;i++)
    {
      for(j=0;j<10;j++)
	{
	  printf(" %c ",myBoard[i][j]);
	}
      printf(" %c  | ",(65+i));
      
      for(j=0;j<10;j++)
	{
	  printf(" %c ",enemyBoard[i][j]);
	}
      printf(" %c\n",(65+i));
    }
  printf(" 1  2  3  4  5  6  7  8  9  10");
  fflush(stdout);
  printf("    |  1  2  3  4  5  6  7  8  9  10\n");
  printf("----------------------------------------");
  fflush(stdout);
  printf("----------------------------\n");
}

void enterUserName()
{
  printf("Enter your user name(max 10 chars)\n");
  fgets(userName,10,stdin);
  int i;
  for(i=0;i<20;i++)
    {
      if(userName[i]=='\n')
	userName[i]='\0';
    }
  //fflush(stdin);
  // printf("%s\n",userName);
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

void playGame()
{
  initiateGameBoard();
  printBoard();
  placeShips();
  
  int gameOver=0;
  while(!gameOver)
    {
      printBoard();
      printf("Enter location to shoot(quit to quit)\n");
      char location[10];
      fgets(location,10,stdin);

      if(strncmp(location,"quit",strlen("quit"))==0)
	{
	  quitGame();
	}

      int row=-1,column=-1,direction=-1;
      getIntLocation(&row,&column,&direction,location);
      
      //printf("row - %d, col - %d, direction - %d\n",row,column,direction);
      
      //check if valid location was entered
      if(row<1&&row>10)
	{
	  printf("Invalid location. Enter again.\n");
	  continue;
	}
      if(column<1&&column>10)
	{
	  printf("Invalid location. Enter again.\n");
	  continue;
	}
      //check if already entered same location
      if(enemyBoard[row-1][column-1]=='O'||enemyBoard[row-1][column-1]=='X')
	{
	  printf("Already Shot at this location. Enter again.\n");
	  continue;
	}
      
      //send to server and get response
	//then server shoots and gets response
      
      int n=write(socketFD,location,sizeof(location));
      if (n < 0) 
	perror("ERROR writing to socket");
      // printf("send %s \n",location);
      recv(socketFD,location,sizeof(location),0);//get response
      // printf("recieved %s",location);
      if(strncmp(location,"hit",strlen("hit"))==0)
	{
	  enemyBoard[row-1][column-1]='X';
	  printf("You hit the enemy\n");
	}
      else if(strncmp(location,"miss",strlen("miss"))==0)
	{
	  enemyBoard[row-1][column-1]='O';
	  printf("You missed\n");
	}
      else
	{
	  //printf("%s\n",);
	  printf("Error.Shoot again.\n");
	  continue;
	}
      //printBoard();
      //check if game is over
      int i=checkGameOver();
      if(i==1)
	{
	  printf("Player lost\n");
	  gameOver=1;
	  continue;
	}
      else if(i==2)
	{
	  printf("Player wins\n");
	  gameOver=1;
	  continue;
	}
      //now the server shoots
      //fflush(socketFD);
      recv(socketFD,location,sizeof(location),0);//get response
      // printf("recieved %s\n",location);
      //printf("server - %s \n",location);
      getIntLocation(&row,&column,&direction,location);
      if(row<1&&row>10)
	{
	  //printf("Invalid location\n");
	  continue;
	}
      if(column<1&&column>10)
	{
	  // printf("Invalid location\n");
	  continue;
	}
      //check if it was a hit or miss
      //location[2]='\0';
      printf("The PC shoots at %s on your board\n",location);
      if(myBoard[row-1][column-1]=='_'||myBoard[row-1][column-1]=='|')
	{
	  printf("You are hit\n");
	  myBoard[row-1][column-1]='X';
	}
      else
	{
	  printf("The PC missed\n");
	  myBoard[row-1][column-1]='O';
	}
      
      //check if game is over
      i=checkGameOver();
      if(i==1)
	{
	  printf("Player lost\n");
	  gameOver=1;
	  continue;
	}
      else if(i==2)
	{
	  printf("Player wins\n");
	  gameOver=1;
	  continue;
	}
    }
  //game is over notify server
  char *over="over";
  int n=write(socketFD,over,sizeof(over));
  printf("send %s\n",over);
}

void quitGame()
{
  char *quit="quit";
  int n=write(socketFD,quit,sizeof(quit));
  if (n < 0) 
    perror("ERROR writing to socket");
  printf("send %s\n",quit);
  close(socketFD);
  exit(EXIT_SUCCESS);
}

int checkLocation(row,col,direction,size)
{
  //printf("row-%d,col-%d,dir-%d,size-%d \n",row,col,direction,size);
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
  // printf("in bounds\n");
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
	  if(myBoard[row-1][col+i-1]=='_'||myBoard[row-1][col+i-1]=='|')
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
	   if(myBoard[row+i-1][col-1]=='_'||myBoard[row+i-1][col-1]=='|')
	    return 0;
	}
    }
  // printf("got here\n");
  return valid;
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

int checkGameOver()
{
  int gameOver=0;//game is not over
  
  int myCount=0,enemyCount=0;

    int i,j;
  for(i=0;i<10;i++)
    {
      for(j=0;j<10;j++)
	{
	  if(myBoard[i][j]=='_'||myBoard[i][j]=='|')//count num of ships left
	    myCount++;
	  if(enemyBoard[i][j]=='X')//number of hits
	    enemyCount++;
	}
    }
  //there are 30 total locations for each player
  if(myCount==0)//no ships left, player looses
    return 1;
  if(enemyCount==30)//all ships sunk
    return 2;

  return gameOver;

}
