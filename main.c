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

#include "centry.h"

void ctlr_c(int signal)  
{
  //!TODO :  handle Ctrl^C 
}
int main(int ac , char **av , char **env)   
{
   int pstatus = EXIT_SUCCESS ;  
   setvbuf(stdout ,  __nptr ,  _IONBF , 0 ) ;  
   struct sigaction sa ; 
   *(void **) &sa.sa_handler = ctlr_c ; 
   
   char *id = centry_check_running_container("docker") ; 
   cpid_t chid =  container_process_id(id) ; 
   
   if( 0 == chid )
   {
     fprintf(stderr , "Not able to get docker container running process\n") ; 
     free(id) ; 
     pstatus = EXIT_FAILURE ; 
   } 
   free(id) ; 
   
   fprintf(stdout , "loading namespace for host container %i\n" , chid); 
   load_ns_for(chid); 
   defsh(__nptr); 

   return  pstatus ; 
}
