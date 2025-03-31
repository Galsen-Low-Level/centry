#define _GNU_SOURCE  
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>
#include <unistd.h> 
#include <sys/stat.h> 
#include <sys/cdefs.h> 
#include <sys/wait.h> 
#include <stdarg.h>
#include <sched.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <errno.h> 
#include <signal.h> 
#include <sys/utsname.h> 
#include <assert.h> 
#include <curses.h> 
#include "centry.h"

#define  CURSES_FLAGS   4 

/* !NOTICES :  this macro  take effect only when no arguments 
 * have been supplied 
 */ 
#define  curses_free(__expr) if ((ac >>8 ) == CURSES_FLAGS )  free(__expr) 

void ctlr_c(int signal)  
{
  //!TODO :  handle Ctrl^C and other  
} 

int main(int ac , char **av , char **env)   
{
   int pstatus = EXIT_SUCCESS ;  
   setvbuf(stdout ,  __nptr ,  _IONBF , 0 ) ;  
   struct sigaction sa ; 
   *(void **) &sa.sa_handler = ctlr_c ; 
   char *id  = __nptr; 
   if (!(ac^1))  
   { 
     id  = centry_check_running_container_for("docker") ;  
    /* NOTICE: if no arguments have been supplied, the ac flags will be set CURSES_FLAGS see(macro) 
     * This indicates that it uses curses allocation. This is just an indication*/ 
     ac= (4<<8)  | ac ; 
   }else  
     id  =*(av+ac-1); 

   cpid_t chid =  container_process_id(id) ; 
   
   if( 0 == chid )
   {
     fprintf(stderr , "Not able to get docker container running process\n") ; 
     curses_free(id);   
     pstatus = EXIT_FAILURE ; 
   } 
    curses_free(id);   
   
   fprintf(stdout , "loading namespace for host container %i\n" , chid); 
   load_ns_for(chid); 
   defsh(__nptr); 

   return  pstatus ; 
}
