/*Non-Canonical Input Processing*/
#include "writenoncanonical.h"

unsigned char * packetToSend;//used for the alarm function
int LENGTH;
int lastFD;
int sumAlarms = 0;
int flagAlarm = 0;
int trama = 0;
int paragem = 0;
unsigned char numMensagens = 0;
int numTotalTramas = 0;
int DONE;

struct termios oldtio, newtio;

void sendPacket(){

  // instalar handler do alarme
  (void) signal(SIGALRM, sendPacket);

    DONE = 0;
    if(sumAlarms == 3){
      printf("\nTIMEOUT: Failed 3 times, exiting...\n");
      exit(-1);
    }
    printf("Attempt %d/3:\n", (sumAlarms + 1));
    int sentBytes = 0;
    while(sentBytes != LENGTH){
      sentBytes = write(lastFD, packetToSend, LENGTH);
      printf("sentBytes: %d\n", sentBytes);
    }
    sumAlarms++;
    if(!DONE){
      alarm(3);
    }else{
      sumAlarms = 0;//reset the attempt count
    }
}


int main(int argc, char **argv){
  int fd;
  off_t sizeFile; //tamanho do ficheiro em bytes
  off_t indice = 0;
  int sizeControlPackageI = 0;

  if ((argc < 3) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0)))  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\nor\t    nserial /dev/ttyS1\n");
    exit(1);
  }
  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)  {
    perror("Error: Opening Initial file descriptor line32\n");
    exit(EXIT_FAILURE);
  }

  unsigned char *mensagem = openReadFile( &sizeFile, (unsigned char *)argv[2]);

  //inicio do relógio
  struct timespec requestStart, requestEnd;
  clock_gettime(CLOCK_REALTIME, &requestStart);



    if (tcgetattr(fd, &oldtio) == -1)  { /* save current port settings */
      perror("tcgetattr");
      exit(EXIT_FAILURE);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-unsigned character timer unused */
    newtio.c_cc[VMIN] = 1;  /* blocking read until 5 unsigned chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a

    leitura do(s) pr�ximo(s) caracter(es)

    */

    tcflush(fd, TCIOFLUSH);


    if (tcsetattr(fd, TCSANOW, &newtio) == -1)  {
      perror("tcsetattr");
      exit(EXIT_FAILURE);
    }

    printf("New termios structure set\n");



  //se nao conseguirmos efetuar a ligaçao atraves do set e do ua o programa termina
  if (llopen(fd)<1)  {
    perror("Error on ApplicationLayer\n");
    exit(EXIT_FAILURE);
  }

  int sizeOfFileName = strlen(argv[2]);
  unsigned char *fileName = (unsigned char *)malloc(sizeOfFileName);
  fileName = (unsigned char *)argv[2];
  unsigned char *start = controlPackageI( sizeFile, C2Start,fileName, sizeOfFileName, &sizeControlPackageI);

  llwrite(start, sizeControlPackageI, fd);
  printf("START frame sent\n");

  int sizePacket = sizePacketConst;
  srand(time(NULL));

  while (sizePacket == sizePacketConst && indice < sizeFile)  {
    //split mensagem
    unsigned char *packet = splitMsg(mensagem, &indice,  sizeFile, &sizePacket);
    printf("sent packet no. %d\n", numTotalTramas);
    //header nivel aplicação
    int headerSize = sizePacket;
    unsigned char *mensagemHeader = headerAL(packet,  &headerSize, sizeFile);
    //envia a mensagem
    if (!llwrite(mensagemHeader, headerSize, fd))    {
      printf("alarm count limit reached\n");
      return -1;
    }
  }

  unsigned char *end = controlPackageI(sizeFile,C2End,  fileName, sizeOfFileName, &sizeControlPackageI);
  llwrite(end, sizeControlPackageI, fd);
  printf("END frame sent\n");

  llclose(fd);

  //fim do relógio
  clock_gettime(CLOCK_REALTIME, &requestEnd);

  double accum = (requestEnd.tv_sec - requestStart.tv_sec) + (requestEnd.tv_nsec - requestStart.tv_nsec) / 1E9;

  printf("Seconds gone by: %f\n", accum);

  sleep(1);

  close(fd);
  return 0;
}

unsigned char *headerAL(unsigned char *mensagem,  int *sizePacket, off_t sizeFile){
  unsigned char *mensagemFinal = (unsigned char *)malloc(sizeFile + 4);
  mensagemFinal[0] = HEADER_C;
  mensagemFinal[1] = numMensagens % 255;
  mensagemFinal[2] = (int)sizeFile / 256;
  mensagemFinal[3] = (int)sizeFile % 256;
  memcpy(mensagemFinal + 4, mensagem, *sizePacket);
  *sizePacket += 4;
  numMensagens++;
  numTotalTramas++;
  return mensagemFinal;
}

unsigned char *splitMsg(unsigned char *mensagem, off_t *indice,  off_t sizeFile, int *sizePacket){
  unsigned char *packet;
  int i = 0;
  off_t j = *indice;
  if (*indice + *sizePacket > sizeFile)
  {
    *sizePacket = sizeFile - *indice;
  }
  packet = (unsigned char *)malloc(*sizePacket);
  for (; i < *sizePacket; i++, j++)
  {
    packet[i] = mensagem[j];
  }
  *indice = j;
  return packet;
}

void stateMachineUA(unsigned char *c, int *state){

  switch (*state)
  {
  //recebe flag
  case 0:
    if (*c == FLAG)
      *state = 1;
    break;
  //recebe A
  case 1:
    if (*c == A)
      *state = 2;
    else
    {
      if (*c == FLAG)
        *state = 1;
      else
        *state = 0;
    }
    break;
  //recebe C
  case 2:
    if (*c == UA_C)
      *state = 3;
    else
    {
      if (*c == FLAG)
        *state = 1;
      else
        *state = 0;
    }
    break;
  //recebe BCC
  case 3:
    if (*c == UA_BCC)
      *state = 4;
    else
      *state = 0;
    break;
  //recebe FLAG final
  case 4:
    if (*c == FLAG)
    {
      paragem = 1;
      alarm(0);
      printf("UA received\n");
    }
    else
      *state = 0;
    break;
  }
}

int llopen(int fd){

  unsigned char c;

  do{
    makeMessage(SET_C, fd);
    printf("SET sent\n");
    int state = 0;

    while (!paragem && !flagAlarm){
      if(read(fd, &c, 1)<0){
        perror("Error on opening descriptor -> llopen line 233\n");
        exit(EXIT_FAILURE);
        }
      stateMachineUA(&c, &state);
    }
  }while (flagAlarm && sumAlarms < NUMMAX);

  printf("flag alarm %d\n", flagAlarm);
  printf("sum %d\n", sumAlarms);

  if (flagAlarm && sumAlarms == 3)  {
    return 0;
  }
  else  {
    flagAlarm = 0;
    sumAlarms = 0;
    return 1;
  }
}


void makeFrame(unsigned char *mensagem, int size){

  unsigned char BCC2;
  unsigned char *BCC2Stuffed = (unsigned char *)malloc(sizeof(unsigned char));
int sizeMensagemFinal = size + 6;
  int sizeBCC2 = 1;
  BCC2 = calcBCC2(size, mensagem);
  BCC2Stuffed = stuffingBCC2(&sizeBCC2, BCC2);
  //cria trama I

  unsigned char *mensagemFinal = (unsigned char *)malloc((size + 6) * sizeof(unsigned char));
    mensagemFinal[0] = FLAG;
    mensagemFinal[1] = A;
    if (trama == 0)  {
      mensagemFinal[2] = C10;
    }
    else  {
      mensagemFinal[2] = C11;
    }
    mensagemFinal[3] = (mensagemFinal[1] ^ mensagemFinal[2]);

    int i = 0;
    int j = 4;
    for (; i < size; i++)  {
      if (mensagem[i] == FLAG)
      {
        mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
        mensagemFinal[j] = ESC;
        mensagemFinal[j + 1] = ESC_FLAG;
        j = j + 2;
      }
      else    {
        if (mensagem[i] == ESC)
        {
          mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
          mensagemFinal[j] = ESC;
          mensagemFinal[j + 1] = ESC_ESC;
          j = j + 2;
        }
        else      {
          mensagemFinal[j] = mensagem[i];
          j++;
        }
      }
    }

    if (sizeBCC2 == 1)
      mensagemFinal[j] = BCC2;
    else  {
      mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
      mensagemFinal[j] = BCC2Stuffed[0];
      mensagemFinal[j + 1] = BCC2Stuffed[1];
      j++;
    }
    mensagemFinal[j + 1] = FLAG;
    //fim criacao trama I



    unsigned char *copia;
    packetToSend = mutationBCC1( sizeMensagemFinal, mensagemFinal); //altera bcc1
    packetToSend = mutationBCC2( sizeMensagemFinal, packetToSend);         //altera bcc2
    LENGTH=sizeMensagemFinal;

}


int llwrite(unsigned char *mensagem, int size, int fd){

    lastFD=fd;
  int rejeitado = 0;

  makeFrame(mensagem,  size);

  flagAlarm = 1;

  //mandar mensagem
while ((flagAlarm && sumAlarms < NUMMAX) || rejeitado)  {

    //escreve trama
    sendPacket();
    alarm(TIMEOUT);
    flagAlarm=0;

    //ciclo de leitura
    unsigned char C = readControlMessageC(fd);

    if ((C == CRR1 && trama == 0) || (C == CRR0 && trama == 1))    {

      printf("Received rr %x, frame= %d\n", C, trama);
      rejeitado = 0;
      sumAlarms = 0;
      trama ^= 1;
      alarm(0);

    } else {
      if (C == CREJ1 || C == CREJ0)      {
        rejeitado = 1;
        printf("Received rej %x, frame=%d\n", C, trama);
        alarm(0);
      }
    }
  }

  if (sumAlarms >= NUMMAX)
    return 0;
  else
    return 1;
}

void makeMessage(unsigned char C, int fd){
  unsigned char message[5];
  message[0] = FLAG;
  message[1] = A;
  message[2] = C;
  message[3] = message[1] ^ message[2];
  message[4] = FLAG;

  lastFD=fd;
  packetToSend=message;
  LENGTH=5;
  sendPacket();
  alarm(TIMEOUT);
  flagAlarm=0;
}

unsigned char readControlMessageC(int fd){
  int state = 0;
  unsigned char c;
  unsigned char C;

  while (!flagAlarm && state != 5)  {

       if(read(fd, &c, 1)<0){
       perror("Error on opening descriptor");
       exit(EXIT_FAILURE);
       }


  switch (state){
    //recebe FLAG
    case 0:
      if (c == FLAG)
        state = 1;
      break;
    //recebe A
    case 1:
      if (c == A)
        state = 2;
      else
      {
        if (c == FLAG)
          state = 1;
        else
          state = 0;
      }
      break;
    //recebe c
    case 2:

      if (c == CRR0 || c == CRR1 || c == CREJ0 || c == CREJ1 || c == DISC)
      {
        C = c;
        state = 3;
      }
      else
      {
        if (c == FLAG)
          state = 1;
        else
          state = 0;
      }
      break;
    //recebe BCC
    case 3:

      if (c == (A ^ C))
        state = 4;
      else
        state = 0;
      break;
    //recebe FLAG final
    case 4:

      if (c == FLAG)
      {
        alarm(0);
        state = 5;
        return C;
      }
      else
        state = 0;
      break;
    }
  }

  DONE=1;

  return 0xFF;
}

unsigned char calcBCC2(int size, unsigned char *mensagem){
  unsigned char BCC2 = mensagem[0];
  int i;
  for (i = 1; i < size; i++)
  {
    BCC2 ^= mensagem[i];
  }
  return BCC2;
}

unsigned char *stuffingBCC2(int *sizeBCC2, unsigned char BCC2){
  unsigned char *BCC2Stuffed;
  if (BCC2 == FLAG)
  {
    BCC2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
    BCC2Stuffed[0] = ESC;
    BCC2Stuffed[1] = ESC_FLAG;
    (*sizeBCC2)++;
  }
  else  {
    if (BCC2 == ESC)    {
      BCC2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
      BCC2Stuffed[0] = ESC;
      BCC2Stuffed[1] = ESC_ESC;
      (*sizeBCC2)++;
    }
  }

  return BCC2Stuffed;
}

unsigned char *openReadFile(off_t *sizeFile, unsigned char *fileName){
  FILE *f;
  struct stat metadata;
  unsigned char *fileData;

f = fopen((char *)fileName, "rb");
  if (f == NULL)  {
    perror("error opening file!");
    exit(EXIT_FAILURE);
  }
  stat((char *)fileName, &metadata);
  (*sizeFile) = metadata.st_size;
  printf("This file has %ld bytes \n", *sizeFile);

  fileData = (unsigned char *)malloc(*sizeFile);

  fread(fileData, sizeof(unsigned char), *sizeFile, f);
  return fileData;
}

unsigned char *controlPackageI(off_t sizeFile,unsigned char state,  unsigned char *fileName, int sizeOfFileName, int *sizeControlPackageI){
  *sizeControlPackageI = 9 * sizeof(unsigned char) + sizeOfFileName;
  unsigned char *package = (unsigned char *)malloc(*sizeControlPackageI);

  if (state == C2Start)
    package[0] = C2Start;
  else
    package[0] = C2End;
  package[1] = T1;
  package[2] = L1;
  package[3] = (sizeFile >> 24) & 0xFF;
  package[4] = (sizeFile >> 16) & 0xFF;
  package[5] = (sizeFile >> 8) & 0xFF;
  package[6] = sizeFile & 0xFF;
  package[7] = T2;
  package[8] = sizeOfFileName;
  int k = 0;
  for (; k < sizeOfFileName; k++)  {
    package[9 + k] = fileName[k];
  }
  return package;
}

void llclose(int fd){
  makeMessage(DISC, fd);
  printf("DISC sent\n");
  unsigned char C;
  //espera ler o DISC
  C = readControlMessageC(fd);
  while (C != DISC)  {
    C = readControlMessageC(fd);
  }
  printf("DISC read\n");
  makeMessage(UA_C,fd);
  printf("final UA sent\n");
  printf("Writer terminated \n");

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)  {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

unsigned char *mutationBCC2( int sizePacket, unsigned char *packet){
  unsigned char *copia = (unsigned char *)malloc(sizePacket);
  memcpy(copia, packet, sizePacket);
  int r = (rand() % 100) + 1;
  if (r <= bcc2Ratio)  {
    int i = (rand() % (sizePacket - 5)) + 4;
    unsigned char randomLetter = (unsigned char)('A' + (rand() % 26));
    copia[i] = randomLetter;
    printf("BCC2 changed\n");
  }
  return copia;
}

unsigned char *mutationBCC1(int sizePacket, unsigned char *packet){
  unsigned char *copia = (unsigned char *)malloc(sizePacket);
  memcpy(copia, packet, sizePacket);
  int r = (rand() % 100) + 1;
  if (r <= bcc1Ratio)  {
    int i = (rand() % 3) + 1;
    unsigned char randomLetter = (unsigned char)('A' + (rand() % 26));
    copia[i] = randomLetter;
    printf("BCC1 changed\n");
  }
  return copia;
}
