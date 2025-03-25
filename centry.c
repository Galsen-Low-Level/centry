/* @file  center.c
 * @brief entering a running process container  
 * @copyright(c) Umar Ba <jUmarB@protonmail.com> <github.com/Jukoo> 
 */ 

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
#include <sys/utsname.h> 
#include <assert.h> 

#include "centry.h" 

char docker_location[0x64]={0} ; 

struct __ns_info_t nsinfo[6] = NS_DEFINITIONS ; 

int has_command(const char * restrict  host_command) 
{
  int  p_index =~0, found=0; 

  while(__nptr != *(_PATH+ (++p_index) ))
  {
    char full_path[1<<5]={0};  
    sprintf(full_path , "%s%s", *(_PATH+p_index),  host_command ) ; 
    if(~0 == access(full_path , F_OK|X_OK)) 
      continue ; 
    
    memcpy(docker_location , full_path , strlen(full_path))  ; 
    found^=1 ;  
    break ; 
  }
  
  return  found ; 
}

cpid_t container_process_id(char * restrict container_id) 
{

  char  buffer[0xff]= {0} ; 
  char *const cmd[]= {DOCKER_INSPECT(container_id)} ; 

  if(spwn( cmd , buffer))
  {
    perr(spwn);  
    return 0 ; 
  }

  return strtol(buffer,  __nptr , 0xa ) ;  
 }

void  load_ns_for(cpid_t container_process_id)  
{
   unsigned int  i  = 0 ;
   while(  __nptr !=  (nsinfo+ i)->_ns_name)    
   { 
     char ns_file[200] ={0}; 
     sprintf(ns_file,  "/proc/%i/ns/%s", container_process_id ,  (nsinfo+i)->_ns_name); 
     int fd =open(ns_file , O_RDONLY); 
     if(!(~0 &~ fd)) 
     { 
       fprintf(stderr , " %s\n" , ns_file) ; 
       perr(open) ;  
     } 
    if(!(~0  & ~setns(fd ,0))) 
    {
      fprintf(stderr, "Trying to set  %s : %s\n",  (nsinfo+i)->_ns_name , strerror(errno)); 
    } 

    i=-~i;  
   }

}


static int spwn(char *const *  shcmd , char *  buffer_result ) 
{ 
   int pfd[2]; 
   if(pipe(pfd)) 
     return ~0 ; //!TODO : print message instead 
                 

   pid_t subprocess =  fork()  ; 
   if(~0 == subprocess)  
     return  ~0 ; //!  no spawn   
                
   __BEGIN_SPAWN_CONTEXT(subprocess) 
     close(*pfd) ; 
     dup2(*(pfd+1)  , STDOUT_FILENO) ; 
     int status  =  execvp(docker_location, shcmd) ; 
     if(~0  == status) 
       exit(EXIT_FAILURE) ; 

   __END_SPAWN_CONTEXT 
   __OTHERWISE {
    
     int sbp_stat =0 ; 
     wait(&(sbp_stat)) ; 
     if (  sbp_stat & 0x00ff ) 
       fprintf(stderr , "Error Occured while getting output\n"); 

     close(*(pfd+1)) ; 
     size_t rbytes = read(*pfd , buffer_result ,0xff) ;  
     assert(!(rbytes^strlen(buffer_result))) ;  

   }

   return 0 ; 
    
}
