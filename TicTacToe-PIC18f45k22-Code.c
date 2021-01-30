// Libraries--------------------------------------------------------------------------
#include "pragmas.h"	
#include <stdlib.h>     
#include <stdio.h>	
#include <p18f45k22.h>
#include <usart.h>      

// Constants ---------------------------------------------------------------------------------
#define TRUE 		1	
#define	FALSE 		0

#define TMR0FLAG INTCONbits.TMR0IF
#define TMR01SEC 0x0BDC
#define ADRSSIZE 4

#define ROW 3
#define COL 3



#define LEDSOFFE TRISE
#define LEDSOFFD TRISD
#define LEDSOFFA TRISA
#define LEDSOFFC TRISC

#define LED21RED LATDbits.LATD0=0 			
#define LED21YELLOW LATDbits.LATD0=1		
#define LED21ON TRISDbits.TRISD0=0

#define LED22RED LATCbits.LATC3=0 			
#define LED22YELLOW LATCbits.LATC3=1		
#define LED22ON TRISCbits.TRISC3=0

#define LED20RED LATDbits.LATD1=0 			
#define LED20YELLOW LATDbits.LATD1=1		
#define LED20ON TRISDbits.TRISD1=0

#define LED12RED LATAbits.LATA7=0 			
#define LED12YELLOW LATAbits.LATA7=1		
#define LED12ON TRISAbits.TRISA7=0

#define LED11RED LATAbits.LATA6=0 			
#define LED11YELLOW LATAbits.LATA6=1		
#define LED11ON TRISAbits.TRISA6=0

#define LED10RED LATCbits.LATC0=0 			
#define LED10YELLOW LATCbits.LATC0=1		
#define LED10ON TRISCbits.TRISC0=0

#define LED02RED LATAbits.LATA5=0 			
#define LED02YELLOW LATAbits.LATA5=1		
#define LED02ON TRISAbits.TRISA5=0

#define LED01RED LATEbits.LATE0=0 			
#define LED01YELLOW LATEbits.LATE0=1		
#define LED01ON TRISEbits.TRISE0=0

#define LED00RED LATEbits.LATE1=0 			
#define LED00YELLOW LATEbits.LATE1=1		
#define LED00ON TRISEbits.TRISE1=0

#define PRESSED 0

#define PLAYER1		1
#define PLAYER2		2	

#define SECONDS		tttAB.time[0][1]
#define MINUTES		tttAB.time[0][0]

#define CONTROLLER 	1
#define SENDER		0

#define BUFFERSIZE 	30
#define TOKENSIZE	10

#define TURNTIME 6

#define RC2FLAG PIR3bits.RC2IF

// Global Variables ----------------------------------------------------------------
unsigned char counter=0;
char enterValue=0;
char rValue=0,cValue=0;
char rFlag=FALSE,cFlag = FALSE;
char manualResetTimer = 0;
char gameFlag = FALSE;
char resetFlag =FALSE;

char availableBlankFlag = FALSE;
char buffCount =0;
char buffFlag= FALSE;
char turnTime= TURNTIME;
char turnTimeFlag= FALSE;
char row=0,col=0;
char played = FALSE;

char *tokens[TOKENSIZE];
char rxBuff[BUFFERSIZE];
char txBuff[BUFFERSIZE];
char dataCollected=FALSE;



//Game Elements-----------------------------------------------------------------------
typedef struct gameBoard
{
	char ticTacToe[ROW][COL];
	char player;
  	char turnCounter;
	char winFlag;

}gameBoard_t;


typedef struct ticTacToe
{
	char address[ADRSSIZE];
	unsigned char select;
	unsigned char time[1][2];		
	char status;
	gameBoard_t game;
}ticTacToe_t;

ticTacToe_t tttAB;


/*--- set_osc__4MHz ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		Setting the oscillator to operate at 4MHz
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void set_osc__4MHz(void)
{
	OSCCON =  0x53; 			// Sleep on sleepp cmd,  4MHz
             
	OSCCON2 = 0x04;			


	OSCTUNE = 0x80;				
						 
	while (OSCCONbits.HFIOFS != 1); 	// wait for osccillator to become stable
}
//eo: set_osc__4MHz

		// --- port configuration ---
/*--- configPort ---------------------------------------------------------------------------
Author:		ABDUL BAASIT
Date:		19/01/2020
Desc:		Set port cofigurations.
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/

void configPort(void)
{
	
	ANSELA 				= 0X00;
	LATA   				= 0X00;
	TRISA              	= 0XE0;

	ANSELC 				= 0X00;  
	LATC 				= 0X00; 
	TRISC				= 0x09;

	ANSELD 				= 0x00;
	LATD				= 0x00;
	TRISD				= 0x03;

	ANSELE				= 0x00;
	LATE				= 0x00;
	TRISE				= 0x03;

	


}		//eo:configPort

// --- 	 ---
/*--- configSerial ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		Configuring pic for serial port-1 configuration
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void configSerial(void)
{
	SPBRG 		= 25;      //  Set baud rate value at 4Mhz
	RCSTA1 		= 0X90;   //  Receiver Status Register
	TXSTA1 		= 0X26;   //   Transmitter Status Register
	BAUDCON1 	= 0X40;  //  Baud rate generator
}		//eo:configSerial

// --- 	 ---
/*---configSerial2 ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		Configuring pic for serial port-2 configuration
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void configSerial2(void)
{
	SPBRG2 		= 12;      // Set baud rate value to 19200
	RCSTA2 		= 0X90;   // Set Receiver Status Register
	TXSTA2 		= 0X26;   //  Set Transmitter Status Register
	BAUDCON2 	= 0X40;  // Set Baud rate generator
}


/*--- resetTmr0 ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		reset the timer to false start 
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void resetTmr0(void)
{
	TMR0H = 0x0B;
	TMR0L = 0xDC;
	TMR0FLAG = FALSE;
} 		//eo:resetTmr0


/*--- timerConfig ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		setting the timer register for 1 second rollover
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/	
void timerConfig(void)
{
	resetTmr0();
	T0CON = 0x93;//93 for 1-sec, 91-.25sec

}		//eo:timerConfig


/*--- structureInitialzation ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		intializing the declared declared structure elements to zero
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void structureInitialzation(void)
{
	sprintf(tttAB.address,"AB");

	for(row=0; row<ROW; row++)								
	{
		for(col=0; col<COL; col++)
		{
			tttAB.game.ticTacToe[row][col]='\0';			//set game board elements
		}//eo for
	}//eo for

	tttAB.game.player=PLAYER1;
	tttAB.game.turnCounter=FALSE;
	tttAB.game.winFlag=FALSE;
	tttAB.select=FALSE;
	tttAB.time[0][0]=0,tttAB.time[0][1] = 0;
	tttAB.status=TRUE;
}


/*---winningRow---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		check the condition if any row is crossed by a player
Input: 		takes the array(board) as input
Returns:	true or false

--------------------------------------------------------------------------------------------*/
char winningRow(char ticTacToe[][COL])
{
	int rowElement=0;
	for(rowElement=0; rowElement<COL; rowElement++) 
    { 
        if(ticTacToe[rowElement][0] == ticTacToe[rowElement][1] && ticTacToe[rowElement][1] == ticTacToe[rowElement][2] &&  	
        	ticTacToe[rowElement][0] != '\0')
			{
            	return TRUE;
			}//eo if
    }//eo for
    return FALSE;
	
}

/*--- winningCol ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		check the condition if any col is crossed by a player
Input: 		takes the array(board) as input
Returns:	true or false

--------------------------------------------------------------------------------------------*/
char winningCol(char ticTacToe[][COL])
{
	int colElement=0;
	for(colElement=0; colElement<COL; colElement++) 
    { 
        if(ticTacToe[0][colElement] == ticTacToe[1][colElement] && ticTacToe[1][colElement] == ticTacToe[2][colElement] &&  		
            ticTacToe[0][colElement] != '\0') 
			{	
            	return TRUE;
			}//eo if
    }//eo for
    return FALSE;
	
}//eo winningCol

/*---  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		clears the buffer which saves the received instruction.
Input: 		address of an array
Returns:	none

--------------------------------------------------------------------------------------------*/
void clearBuffer(char *ptr)
{
	while(*ptr)
	{
		*ptr='\0';
		ptr++;
	}
}

/*---interruptConfig  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		interrupt configuration setup
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void interruptConfig(void)
{
	INTCON=0xC0;
	PIE3bits.RC2IE=1;

}

void ISR(void);

#pragma code interrupt_vector=0x08


/*--- interrupt_vector ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		directing towards the vector
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void interrupt_vector(void)
{
	_asm
		GOTO ISR
	_endasm
} //eo ISR

#pragma code


/*--- calcCheckSum ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		calculates the checksum 
Input: 		address of the array
Returns:	character value
--------------------------------------------------------------------------------------------*/
char calcCheckSum(char *ptr)
{
	char checkSum = 0;	
	while(*ptr)
	{
		checkSum^= *ptr;
		ptr++;
	}
	return checkSum;
}//eo-calcCheckSum




/*---gameEndSentence---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		upon game completion, packages a game sentence and transmits to mbed.
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void gameEndSentence(void)
{
		if(tttAB.game.winFlag && tttAB.game.player == PLAYER2)
		{	
			buffFlag = TRUE;
			sprintf(txBuff,"$GAMEND,%i,%i,%c,%c,%i,%i\0",CONTROLLER,SENDER,'W','X',MINUTES,SECONDS);
			sprintf(txBuff,"%s,%i^\0",txBuff,calcCheckSum(txBuff));
			puts2USART(txBuff);
			 
		}

		else if(tttAB.game.winFlag && tttAB.game.player == PLAYER1)
		{	
			buffFlag = TRUE;
			sprintf(txBuff,"$GAMEND,%i,%i,%c,%c,%i,%i\0",CONTROLLER,SENDER,'W','O',MINUTES,SECONDS);
			sprintf(txBuff,"%s,%i^\0",txBuff,calcCheckSum(txBuff));
			puts2USART(txBuff);
			 
		}

		else
		{
			buffFlag = TRUE;
			sprintf(txBuff,"$GAMEND,%i,%i,%c,%c,%i,%i\0",CONTROLLER,SENDER,'D','N',MINUTES,SECONDS);
			sprintf(txBuff,"%s,%i^\0",txBuff,calcCheckSum(txBuff));
			puts2USART(txBuff);
			 

		}
}


/*--- availableBlank ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		once the timer runs out automatically finds the next available space and fills the
		space, packages and transmits the sentence to mbed and change player turn.
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void availableBlank()
{	
	
	for(row=0; row<ROW; row++)								
	{
		for(col=0; col<COL; col++)
		{
			if (tttAB.game.ticTacToe[row][col] == '\0' && tttAB.game.player == PLAYER1)
			{
				tttAB.game.ticTacToe[row][col] = 'X';
				buffFlag = TRUE;
				sprintf(txBuff,"$PLYPMT,%i,%i,%i,%i,%i\0",CONTROLLER,SENDER,row,col,tttAB.game.player);
				sprintf(txBuff,"%s,%i^\0",txBuff,calcCheckSum(txBuff));
				puts2USART(txBuff);
				 
				return;
			}
			if (tttAB.game.ticTacToe[row][col] == '\0' && tttAB.game.player == PLAYER2)
			{
				tttAB.game.ticTacToe[row][col] = 'O';
				buffFlag = TRUE;
				sprintf(txBuff,"$PLYPMT,%i,%i,%i,%i,%i\0",CONTROLLER,SENDER,row,col,tttAB.game.player);
				sprintf(txBuff,"%s,%i^\0",txBuff,calcCheckSum(txBuff));
				puts2USART(txBuff);
				 
				return;
			}			

		}//eo for
	}

	
			
}


/*--- collectSentence ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		collect the sentence received from mbed platform byte by byte and saves it into
		an array
Input: 		address of receving buffer(array)
Returns:	none

--------------------------------------------------------------------------------------------*/
void collectSentence(char *ptr)
{
	char hold=0;
	if(RC2FLAG)
	{
		hold=RCREG2;
		if(hold=='$')
		{
			*ptr=hold; 
			while(hold!='^')
			{
				if(RC2FLAG)
				{
					hold=RCREG2;
					ptr++;
					*ptr=hold;
					if (hold=='^')
					{
						ptr++;
						*ptr = 0x00;
						dataCollected= TRUE;
					}
				}
			}
		}
	}
	
}

/*--- validateSentence ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:	 	finds the checksum in the received message and validates it upon successful
		validation returns true and if invalid returns false.
Input: 		address of the buffer(array)
Returns:	TRUE(1) or FALSE (0)

--------------------------------------------------------------------------------------------*/
char validateSentence(char *ptr)
{
	char csFlag = FALSE;
	char rxcs=0;
	char newcs=0;
	int charCount=strlen(ptr);
	while(csFlag==FALSE)
	{
		if(*(ptr+charCount)=='^')
		{
			*(ptr+charCount)=0x00;
		}

		if(*(ptr+charCount)==',')
		{
			*(ptr+charCount)=0x00;
			rxcs=atoi(ptr+charCount+1);
			csFlag=TRUE;
		}
		charCount--;
	}//eo-while
	newcs=calcCheckSum(ptr);

	if(rxcs==newcs)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*---parseSentence  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		parse the required information in buffer and address it to a address buffer
Input: 		address of buffer(array)
Returns:	none

--------------------------------------------------------------------------------------------*/
void parseSentence(char*ptr)
{
	int insert=0;
	while(*ptr)
	{
		if(*ptr=='$'||*ptr==',')
		{
			*ptr=0x00;
			tokens[insert]=ptr+1;
			insert++;
		}
		ptr++;
	}//eo-while
} 

/*---reset  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		reset the whole game
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void reset(void)
{
	
		LEDSOFFC = 0x09;
		LEDSOFFD = 0x03;
		LEDSOFFE = 0x03;
		LEDSOFFA = 0xE0;
		structureInitialzation();
		counter = 0;
		rValue=0,cValue=0,rFlag=0,cFlag=0,enterValue=0;
		
		printf("\033[2J\033[H");
	
}//eo reset


/*--- manualReset ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		resets the game upon receiving a manual reset instruction
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void manualReset()
{
		
			reset();
			gameFlag = FALSE;
			tttAB.game.winFlag = FALSE;
			resetFlag= TRUE;
			turnTimeFlag= FALSE;
			turnTime= TURNTIME;			
		
			
		
}


/*---executeSentence  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		execute the received instruction 
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void executeSentence(void)
{
	char plypmt[]=("PLYPMT");
	char gamrst[] =("GAMRST");
	if(SENDER ==atoi(tokens[2]))
	{
		if(CONTROLLER==atoi(tokens[1]))
		{
			if(strcmp(plypmt,tokens[0]) == 0)
			{
				rValue = atoi(tokens[3]);
				cValue = atoi(tokens[4]);
				tttAB.game.player = atoi(tokens[5]);
				gameFlag= TRUE;
				resetFlag = FALSE;
				tttAB.game.winFlag = FALSE;
				turnTimeFlag= TRUE;
				if(tttAB.game.player == PLAYER1 && tttAB.game.ticTacToe[rValue][cValue] != '/0' && tttAB.game.ticTacToe[rValue][cValue] != 'O' )
				{
					tttAB.game.ticTacToe[rValue][cValue] = 'X';
				}

				
				if(tttAB.game.player == PLAYER2 && tttAB.game.ticTacToe[rValue][cValue] != '/0' && tttAB.game.ticTacToe[rValue][cValue] != 'X' )
				{
					tttAB.game.ticTacToe[rValue][cValue]= 'O';
				}
			}

			if( strcmp(gamrst,tokens[0]) == 0)
			{
				if('Y' == *tokens[3])
				{
					manualReset();
				}
					
			}
					
		}
	}	
}
// --- 	 ---
/*---  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		check for diagnol crossing
Input: 		address of an array(board)
Returns:	true or false

--------------------------------------------------------------------------------------------*/
char diagonalWin(char ticTacToe[][COL])
{
	if(ticTacToe[0][0] == ticTacToe[1][1] && ticTacToe[1][1] == ticTacToe[2][2] &&  					//It will return true if the diagonals are same
        ticTacToe[0][0] != '\0')
		{ 
        	return TRUE;
		}//eo if
          
    if (ticTacToe[0][2] == ticTacToe[1][1] && ticTacToe[1][1] == ticTacToe[2][0] && 
         ticTacToe[0][2] != '\0') 
        {
			return TRUE; 
  		}//eo if
    return FALSE; 
}//diagonalWin	



/*--- turnChange ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		turns the change if player doesn't play and runs out of 30secs.
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void turnChange()
{
	if(turnTimeFlag)
	{ 
		turnTime--;
		if(turnTime == 0 && !played )
		{
			availableBlankFlag= TRUE;
			turnTime = TURNTIME;
			tttAB.game.player++;
			tttAB.game.turnCounter++;
			tttAB.select=0;
			rValue=0,cValue=0,rFlag=FALSE,cFlag=FALSE,enterValue=0;
			if( tttAB.game.player == 3)
			{
				tttAB.game.player =PLAYER1;
			}
		
		}
		else if(turnTime && played )
		{
			turnTime = TURNTIME;
			played = FALSE;
		}
			
	}
}
			
	
/*--- time ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		game clock
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/

void time(void)
{

	SECONDS++;
	if(SECONDS==60)
	{
			SECONDS =0;
			MINUTES++;			
	}//eo if(SECONDS==60)

}//eo time

/*---gameEnd  ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		check for win condition
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/

char gameEnd(char ticTacToe[][COL])
{
	
	if((winningRow(ticTacToe) || winningCol(ticTacToe) || diagonalWin(ticTacToe)))		//if any row/col/diagonal match, game will be end
	{
		tttAB.game.winFlag = TRUE;
		gameFlag = FALSE;
		resetFlag = FALSE;
		return tttAB.game.winFlag;
	}
	else
	{
	return FALSE;
	}
}//eo gameEnd



/*--- updateDisplay ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		real time GUI
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/
void updateDisplay()
{
	printf("\33[1;0HTicTacToe%s\n\r",tttAB.address);
	printf("\33[2;0HGame Time:%i:%2i",MINUTES,SECONDS);
	if(tttAB.game.winFlag && !gameFlag && !resetFlag)
	{
			printf("\33[2;40HStatus:WON     \n\r");
	}
	else if(gameFlag && !tttAB.game.winFlag && !resetFlag)
	{
			printf("\33[2;40HStatus:Playing\n\r");
	}
	else if(!gameFlag && !tttAB.game.winFlag && !resetFlag)
	{
			printf("\33[2;40HStatus:Ready    \n\r");
	}
	else
	{
			printf("\33[2;40HStatus:Reset      \n\r");
	}

	if(tttAB.game.player == PLAYER1)
	{
		printf("\33[3;0HPlayer:%c\n\r",'X');
	}
	else
	{
		printf("\33[3;0HPlayer:%c\n\r",'O');
	}
		
	printf("\33[3;40HTurn Count:%i\n\r",tttAB.game.turnCounter);
	turnChange();
	printf("Turn Time:%2i\33[4;40H\n\r",turnTime);
	printf("\n\33[5;6HCOLUMN\n\r");
	printf("\33[6;5H0\33[6;9H1\33[6;13H2\n\r");
	printf("\33[7;3H0\33[7;5H%c\33[7;7H|\33[7;9H%c\33[7;11H|\33[7;13H%c\n\r",tttAB.game.ticTacToe[0][0],tttAB.game.ticTacToe[0][1],tttAB.game.ticTacToe[0][2]);
	printf("\33[8;1HR\33[8;4H------------\n\r");
    printf("\33[9;1HO\33[9;3H1\33[9;5H%c\33[9;7H|\33[9;9H%c\33[9;11H|\33[9;13H%c\n\r",tttAB.game.ticTacToe[1][0],tttAB.game.ticTacToe[1][1],tttAB.game.ticTacToe[1][2]);
	printf("\33[10;1HW\33[10;4H------------\n\r");
	printf("\33[11;3H2\33[11;5H%c\33[11;7H|\33[11;9H%c\33[11;11H|\33[11;13H%c\n\r",tttAB.game.ticTacToe[2][0],tttAB.game.ticTacToe[2][1],tttAB.game.ticTacToe[2][2]);

	if(buffFlag == TRUE)//commsenetence display for 5seconds
	{
		buffCount++;
		if(buffCount<5  )
		{
				printf("\33[14;0H%s",txBuff);
			
		}

		if(buffCount<5 && PIE3bits.RC2IE  )
		{
				printf("\33[14;0H%s",rxBuff);
			
		}		
		else //if(buffCount>5)
		{	
			clearBuffer(txBuff);
			clearBuffer(rxBuff);
			printf("\33[14;0H\33[2K");
			buffFlag =FALSE;
			buffCount =0;
		}	
	}
}//eo updateDisplay

/*--- initializeSystem ---------------------------------------------------------------------------
Author:		Abdul Baasit
Date:		19/01/2020
Desc:		intialize all the fuction required for intial setup
Input: 		none
Returns:	none

--------------------------------------------------------------------------------------------*/

void initializeSystem(void)
{
	set_osc__4MHz();	
	configPort();		
	configSerial();
	configSerial2();         				
	timerConfig();
	structureInitialzation();
	interruptConfig();
	
	
}		//eo:initializeSystem

#pragma interrupt ISR


void ISR(void)
{
	if(RC2FLAG)
	{
		PIE3bits.RC2IE=0;
		collectSentence(rxBuff);
		PIE3bits.RC2IE=1;
	}

}
/*****************MAIN FUNCTION **************************************************************
 *********************************************************************************/
void main()
{

	
	initializeSystem();
	
	
	while(TRUE)
	{
		
		
		
		if(TMR0FLAG)
		{
			resetTmr0();
			if(gameFlag)
			{
				time();
			}		
			updateDisplay();
		}//eo if(TMR0FLAG)
		
		if(gameEnd(tttAB.game.ticTacToe))
		{
			gameEndSentence();
			reset();
			gameFlag=FALSE;
			resetFlag = FALSE;
			tttAB.game.winFlag = TRUE;
			turnTimeFlag= FALSE;
			turnTime= TURNTIME;
			
		}
		else if((tttAB.game.ticTacToe[0][0] != '\0') && (tttAB.game.ticTacToe[0][1] != '\0') &&
			(tttAB.game.ticTacToe[0][2] != '\0') && (tttAB.game.ticTacToe[1][0] != '\0') &&	(tttAB.game.ticTacToe[1][1] != '\0') &&
			 (tttAB.game.ticTacToe[1][2] != '\0') && (tttAB.game.ticTacToe[2][0] != '\0') && (tttAB.game.ticTacToe[2][1] != '\0') && 
				(tttAB.game.ticTacToe[2][2] != '\0') )
		{
			gameEndSentence();
			reset();
			gameFlag=FALSE;
			resetFlag = FALSE;
			tttAB.game.winFlag = FALSE;
			turnTimeFlag= FALSE;
			turnTime= TURNTIME;
			
		}//eo if
	
	if(dataCollected == TRUE)
        {
             
            dataCollected = FALSE;
            
            if(validateSentence(rxBuff))
            {
                
                parseSentence(rxBuff);
                executeSentence();
            }
         }
	if(availableBlankFlag)
	{
		availableBlank();
		availableBlankFlag = FALSE;
	}


	}//eo-while


}//eo main
