/*Non-Canonical Input Processing*/
#include "noncanonical.h"


int esperado = 0;
struct termios oldtio, newtio;

//void mensagem(int signum){
// time_t tp;
//	time(&tp);
// printf("aqui %s",ctime(&tp));
//}



int main(int argc, char **argv){
		  
		int fd;
		int msgSize = 0;
		int sizeOfStart = 0;		

		unsigned char *msgReady;
		unsigned char *start;
		unsigned char *pinguim;

		off_t index = 0;
		off_t sizeOfPinguim = 0;


  if ((argc < 2) ||    (	(strcmp("/dev/ttyS0", argv[1]) != 0) && (strcmp("/dev/ttyS1", argv[1]) != 0)	)){
    printf("Calling Command Receiver by\n\tex: <ScriptName> /dev/ttyS1\n");
    exit(1);
  }
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
 printf("Application link layer \n");
  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0){
	perror("Error: Opening initial file descriptor line32\n");
	exit(EXIT_FAILURE);
  }


	

	if (tcgetattr(fd, &oldtio) == -1){ /* save current port settings */
		perror("tcgetattr");
	exit(EXIT_FAILURE);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

	/*
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	leitura do(s) próximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	printf("New termios structure set\n");

	if (tcsetattr(fd, TCSANOW, &newtio) == -1){
		perror("tcsetattr");
	exit(EXIT_FAILURE);
	}






//applicationLayer->RECEIVER
  if(llopen(fd) < 1){
    perror("Error on ApplicationLayer\n");
    exit(EXIT_FAILURE);
  };


  start = llread(fd, &sizeOfStart);

	//get info from Trama Start
  unsigned char *name_file_for_pinguim = pinguimFileNameOnTrama(start);
  sizeOfPinguim = sizeFileTramaStart(start);

  pinguim = (unsigned char *)malloc(sizeOfPinguim);

	while (1){
		msgReady = llread(fd, &msgSize);
		
		if (msgSize == 0){
			continue;
		}

		if (compareStart_End(msgReady, sizeOfStart, start, msgSize)){
			printf("End message received\n");
		break;
		}

		int sizeWithoutHeader = 0;

		msgReady = processing_header( msgSize, msgReady, &sizeWithoutHeader);

		memcpy(pinguim + index, msgReady, sizeWithoutHeader);
		index += sizeWithoutHeader;
	}

 // printf("message: \n");
 // 	int i = 0;
 // for (; i < sizeOfPinguim; i++){
 //   printf("%x", pinguim[i]);
 // }

  make_file(name_file_for_pinguim, pinguim, &sizeOfPinguim);

	
  if(llclose(fd) < 0){
    perror("Error on Aplication Layer llclose()");
    exit(EXIT_FAILURE);
  };
  
  sleep(1);
  if(close(fd) < 0) {
    perror("Error on Closing the descriptor");
    exit(EXIT_FAILURE);
  };
  return 0;
}



int llclose(int fd){
	printf("Checking DISC_C\n");
	check_control_Tramas_S_e_U(fd, DISC_C);
	printf("Received DISC\n");

	makeTrama_S_e_U(fd, DISC_C);
	printf("Sended DISC\n");

	check_control_Tramas_S_e_U(fd, UA_C);
	printf("Received UA\n");
	printf("Receiver terminated\n");

        if(tcsetattr(fd, TCSANOW, &oldtio) == -1){
		perror("Unable o set attr : tcsetattr ");
		exit(EXIT_FAILURE);
	}

return 1;
}



int llopen(int fd){
	if (check_control_Tramas_S_e_U(fd, SET_C)){
		printf("Received trama -> SET_C\n");
		makeTrama_S_e_U(fd, UA_C);
		printf("Sended UA\n\n\n");
	}
return 1;
}

unsigned char *llread(int fd, int *msgSize){
	  unsigned char *pacote = (unsigned char *)malloc(0);
	  *msgSize = 0;
	  unsigned char C_readed;
	  int counterNr = 0;
	  int booleanFlag = 0;
	  unsigned char readBits;
	  int state = 0;
//printf("170\n");
 //signal(SIGALRM, mensagem);
	usleep(DELAY);
  while (state != 6){
//	printf("173\n"); 
	    if(read(fd, &readBits, 1) < 0){
                perror("Error on opening descriptor -> llread line 188\n");
                exit(EXIT_FAILURE);
             }
//	printf("176\n");
//}
//		alarm(TIME_OUT_TEST);
//sleep(1);
	    switch (state){
//			printf("188\n");	
		    case 0:
		      if (readBits == FLAG)  //0x7E
			state = 1;
		      break;
		  
		    case 1:
		     
		      if (readBits == A){  ///recebe A 0x03
			state = 2;
		      }else{
				if (readBits == FLAG){ //0x7E
				  state = 1;
				}else{
				  state = 0;
				}
		      }
		      break;
		    //recebe readBits
		    case 2: 
//printf("208\n");     
		      if (readBits == C10){
				counterNr = 0;
				C_readed = readBits;
				state = 3;
		      }else if (readBits == C11){
				counterNr = 1;
				C_readed = readBits;
				state = 3;
		      }else{
			if (readBits == FLAG){
			  	state = 1;
			}else{
				  state = 0;
			}
		      }
		      break;
		    case 3:       //recebe BCC
		      if (readBits == (A ^ C_readed)){
			state = 4;
		      }else{
			state = 0;
			}
		      break;
		    
		    case 4:
//printf("234\n"); 
		      if (readBits == FLAG){ //recebe FLAG final
			if (parityBitBCC2(pacote, *msgSize)){
			  if (counterNr == 0){
			    makeTrama_S_e_U(fd, RR_C1);
			  }else{
			    makeTrama_S_e_U(fd, RR_C0);
			  }
			  state = 6;
			  booleanFlag = 1;
			  printf("Sended RR:  %d\n", counterNr);
			}else{
			  if (counterNr == 0){
			    makeTrama_S_e_U(fd, REJ_C1);
			  }else{
			    makeTrama_S_e_U(fd, REJ_C0);
			  }
			  state = 6;
			  booleanFlag = 0;
			  printf("Sended REJ:  %d\n", counterNr);
			}
		      }else if (readBits == ESCAPE){
			state = 5;
		      }else{
			pacote = (unsigned char *)realloc(pacote, ++(*msgSize));
			pacote[*msgSize - 1] = readBits;
		      }
		      break;
		    case 5:
//		     printf("263"); 
		      if (readBits == ESCAPE_FLAG) {
			pacote = (unsigned char *)realloc(pacote, ++(*msgSize));
			pacote[*msgSize - 1] = FLAG;
		      }else {
			if (readBits == ESCAPE_ESCAPE){
			  pacote = (unsigned char *)realloc(pacote, ++(*msgSize));
			  pacote[*msgSize - 1] = ESCAPE;
			}else{
			//  perror("Non valid character after escape character");
			//  exit(-1);
			}
		      }
		      state = 4;
		      break;
		    }
		  }
	  printf("Readed Size: %d\n", *msgSize);
	 
	  pacote = (unsigned char *)realloc(pacote, *msgSize - 1);  //pacote tem BCC2 no fim

	  *msgSize = *msgSize - 1;
	  if (booleanFlag){
	    if (counterNr == esperado){
	      esperado ^= 1;
	    }else{
	      *msgSize = 0;
	    }
	  }else{
	    *msgSize = 0;
	  }
  return pacote;
}

int parityBitBCC2(unsigned char *message, int msgSize){
	int i = 1;
	unsigned char BCC2 = message[0];
	
 	for (; i < msgSize - 1; i++){
		BCC2 ^= message[i];
	}

	if (BCC2 == message[msgSize - 1]){
		return 1;
	}else{
		return 0;
	}
}

void makeTrama_S_e_U(int fd, unsigned char campoControlo){
  unsigned char trama[5];
  trama[0] = FLAG;
  trama[1] = A;
  trama[2] = campoControlo;
  trama[3] = trama[1] ^ trama[2];
  trama[4] = FLAG;
  
	if( write(fd, trama, 5) < 0 ){
		 perror("Error in sending trama");
    		 exit(EXIT_FAILURE);
	}
}

int check_control_Tramas_S_e_U(int fd, unsigned char campoControlo){
  int state = 0;
  unsigned char readBits;

  while (state != 5){
    if(read(fd, &readBits, 1) <  0){
	perror("Error on opening descriptor\n");
     	exit(EXIT_FAILURE);
    }

    switch (state){
	  
	    case 0:
	      if (readBits == FLAG){
		state = 1;
	      }
	      break;	 
	    case 1:
	      if (readBits == A){
		state = 2;
	      }else{
		if (readBits == FLAG){
		  state = 1;
		}else{
		  state = 0;
		}
	      }
	      break;
	    case 2:
	      if (readBits == campoControlo){
		state = 3;	      
	      }else{
		if (readBits == FLAG){
		  state = 1;
		}else{
		  state = 0;
		}
	      }
	      break;
	 
	    case 3:
	      if (readBits == (A ^ campoControlo)){
		state = 4;
	      }else{
		state = 0;
	      }	      
	      break;
	 
	    case 4:
	      if (readBits == FLAG){
		state = 5;
	      }else{
		state = 0;
	      }
	      break;
	    }
	  }
  return 1;
}





unsigned char *processing_header( int sizeToRemove, unsigned char *to_Remove, int *sizeRemoved){
  int i = 0;
  int j = 4;
  unsigned char *rm_Header = (unsigned char *)malloc(sizeToRemove - 4);
  
  for (; i < sizeToRemove; i++, j++){
    rm_Header[i] = to_Remove[j];
  }

  *sizeRemoved = sizeToRemove - 4;
  return rm_Header;
}




int compareStart_End( unsigned char *end, int sizeStart, unsigned char *start, int sizeEnd){
  int s = 1;
  int e = 1;

  if (sizeStart != sizeEnd){
    return 0;
  }else{
    if (end[0] == C2End){
      for (; s < sizeStart; s++, e++){
        if (start[s] != end[e])
          return 0;
      }
      return 1;
    }else{
      return 0;
    }
  }
}




off_t sizeFileTramaStart(unsigned char *start){
  return (start[3] << 24) | (start[4] << 16) | (start[5] << 8) | (start[6]);
}


unsigned char *pinguimFileNameOnTrama(unsigned char *start){

  int L2 = (int)start[8];
  unsigned char *name = (unsigned char *)malloc(L2 + 1);

  int i;
  for (i = 0; i < L2; i++){
    name[i] = start[9 + i];
  }

  name[L2] = '\0';
  return name;
}



void make_file(unsigned char filename[], unsigned char *mensagem, off_t *sizeFile){
  FILE *file = fopen((char *)filename, "wb+");
	if(file == NULL ){
		perror("Error on creating file");
		exit(EXIT_FAILURE);
	}
  fwrite((void *)mensagem, 1, *sizeFile, file);
  printf("%zd\n", *sizeFile);
  printf("New file created\n");
  if ( fclose(file) < 0){
	perror("Error closing file");	
	}

}
