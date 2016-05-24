/********************************************************************\
Sockit-100 readout 
May 24, 2016

Uses UDP connection to sockit to get data.

 Thomas Lindner (TRIUMF)
\********************************************************************/


#include <vector>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include "midas.h"
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "fesockit";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 0;

/* maximum event size produced by this frontend */
INT max_event_size =  3 * 1024 * 1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) --- not really used here */
INT max_event_size_frag = 2 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 20 * 1000000;
 void **info;
char  strin[256];
HNDLE hDB, hSet;






/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_trigger_event(char *pevent, INT off);
INT read_scaler_event(char *pevent, INT off);


/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

   {"SOCKIT100",               /* equipment name */
    {1, 0,                   /* event ID, trigger mask */
     "SYSTEM",               /* event buffer */
#ifdef USE_INT
     EQ_INTERRUPT,           /* equipment type */
#else
     EQ_PERIODIC,              /* equipment type */
#endif
     LAM_SOURCE(0, 0xFFFFFF),        /* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_RUNNING,           /* read only when running */
     1000,                    /* poll for 1000ms */
     0,                      /* stop run after this event limit */
     0,                      /* number of sub events */
     0,                      /* don't log history */
     "", "", "",},
    read_trigger_event,      /* readout routine */
    },

   {""}
};

#ifdef __cplusplus
}
#endif

/********************************************************************\
              Callback routines for system transitions
  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:
  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.
  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.
  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.
  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.
  pause_run:      When a run is paused. Should disable trigger events.
  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <netdb.h>
#include <byteswap.h>

#define BUFSIZE 16384
/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{
  
  // Setup UDP connection
  int fd;
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { perror("cannot create socket"); return FE_ERR_HW; } 
  
  struct sockaddr_in myaddr; 
  memset((char *)&myaddr, 0, sizeof(myaddr)); 
  myaddr.sin_family = AF_INET; 
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connection from anywhere...  
  myaddr.sin_port = htons(43523); 
  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) { perror("bind failed"); return FE_ERR_HW; }


  // receive data...
  struct sockaddr_in remaddr; 
  socklen_t addrlen = sizeof(remaddr); 
  /* length of addresses */ 
  int recvlen;
  unsigned char buf[BUFSIZE];
  

  for(int i = 0; i < 10; i++){
    printf("waiting on port %d\n", 43523); 
    //  recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen); 
    recvlen = recvfrom(fd, buf, BUFSIZE, 0, 0, 0);
    printf("received %d bytes\n", recvlen);

    unsigned int *data = (unsigned int*)buf;
    
    for(int j = 0; j < 5; j++){
      std::cout <<  std::hex << "0x" << data[j] << " 0x" << __bswap_32 (data[j]) <<  std::dec << std::endl;
    }
    for(int j = recvlen/4-5; j < recvlen/4 + 1; j++){
      std::cout <<  std::hex << "0x" << data[j]  << " 0x" << __bswap_32 (data[j]) << std::dec << std::endl;
    }

    if (recvlen > 0) { 
      buf[recvlen] = 0; 
      printf("received message: \"%s\"\n", buf); 
    }
  }
   
  exit(0);

  return SUCCESS;

}




/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  printf("Exiting fedcrc!\n");
   return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/
// Upon run stasrt, read ODB settings and write them to DCRC
INT begin_of_run(INT run_number, char *error)
{

   return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
  usleep(10000);
  return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\
  Readout routines for different events
\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/
// Not currently used for DCRC readout
extern "C" { INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   int i;

   for (i = 0; i < count; i++) {
//      cam_lam_read(LAM_SOURCE_CRATE(source), &lam);

//      if (lam & LAM_SOURCE_STATION(source))
         if (!test)
            return 1;
   }

   usleep(1000);
   return 0;
}
}

/*-- Interrupt configuration ---------------------------------------*/
// This is not currently used by the DCRC readout
extern "C" { INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}
}


/*-- Event readout -------------------------------------------------*/
INT read_trigger_event(char *pevent, INT off)
{

  /* init bank structure */
  bk_init32(pevent);
  
  uint32_t *pdata32;
  /* create structured ADC0 bank of double words (i.e. 4-byte words)  */
  bk_create(pevent, "ADC0", TID_DWORD, (void **)&pdata32);


  bk_close(pevent, pdata32);    

  usleep(10000);
  return bk_size(pevent);
}
