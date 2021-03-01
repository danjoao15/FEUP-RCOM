#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define TIME_OUT_TEST 1

#define FLAG 0x7E
#define A 0x03
    //Bit paridade
#define SET_BCC A ^ SET_C
#define UA_BCC A ^ UA_C
#define DELAY 50   //500000 meio segundo

#define SET_C 0x03
#define UA_C 0x07
#define C10 0x00
#define C11 0x40
#define RR_C0 0x05
#define RR_C1 0x85
#define REJ_C0 0x01
#define REJ_C1 0x81
#define DISC_C 0x0B
#define C2End 0x03

#define ESCAPE 0x7D
#define ESCAPE_FLAG 0x5E
#define ESCAPE_ESCAPE 0x5D

/*--------------------------Application Link Layer --------------------------*/
/**
* @brief  Retrieve the file name from the packet control trama Start
* @param start
* @returm name of the file to open
*/
unsigned char *pinguimFileNameOnTrama(unsigned char *start);

/**
* @brief Retrieve the size of the file from START
* @param START.
* @return size of the file
*/
off_t sizeFileTramaStart(unsigned char *start);

/**
 * @brief remove headers from trama I
 * @param sizeToRemove
 * @param to_Remove
 * @param sizeRemoved
 * @return
*/
unsigned char *processing_header( int sizeToRemove,unsigned char *to_Remove, int *sizeRemoved);

/**
 * @brief Verification and comparison between start and end
 * @param end
 * @param sizeStart
 * @param sizeEnd
 * @return 1 on success, 0 otherwise
*/
int compareStart_End(  unsigned char *end, int sizeStart, unsigned char *start, int sizeEnd);

/**
 * @brief make the file pinguim with the data received in trama I
 * @param filename name of the file to be made
 * @param sizeFile size of the file to be made
 * @param mensagem data transmitted to go on the file
 * @return void
*/
void make_file(unsigned char *filename, unsigned char *mensagem, off_t *sizeFile );


/*--------------------------Data Link Layer --------------------------*/

/**
* @brief Read  the control SET and send UA
* @param file descriptor
* @return 1 on sucess
*/
int llopen(int fd); //applicationLayer->RECEIVER

/**
* @brief Read trama I and undo the byte stuffing
* @param fd descriptor
* @param length size of the data
* @return data read
*/
unsigned char *llread(int fd, int *msgSize);

/**
* @brief Read trama control DISC and send DISC back - order to disconnected and send Ua
* @param file descriptor
* @return 1 on success 
*/
int llclose(int fd);

/**
* @brief State machine to check  S ou U
* @param descriptor
* @param Campo de controlo: SET_C DISC_C UA RR REJ
* @return 0 sucessful, otherwise unsucessfull
*/
int check_control_Tramas_S_e_U(int fd, unsigned char C);

/**
* @brief Makes control tramas S and U
* @param fd descriptor
* @param  CampoControlo- : pode ser  SET DISC UA RR REJ
* @return 0 on sucess, otherwise unsucessfull
*/
void makeTrama_S_e_U(int fd, unsigned char C);

/**
* @brief check  bit parity
* @param message
* @param length
* @return 0 on sucess, otherwise unsucessfull
*/
int parityBitBCC2(unsigned char *message, int msgSize);



