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

#if defined(__ptr_t) 
# define __nptr  (__ptr_t) 0 
#else 
# define __nptr   (void *) 0 
#endif  

#if __glibc_has_attribute(constructor)
# define __ctor  __attribute__((constructor)) 
#else 
# define __ctor /* Nothing */ 
#endif

#if __glibc_has_attribute(__noreturn)  
# define  __noreturn  __attribute__((__noreturn)) 
#else 
# define __noreturn /* Nothing */
#endif 

/* !NOTICE : For unknow reason i try to  use the strtok and strtok_r to parse quickly  the PATH 
 *           but i have weird behavior when i'm inside  the container, the basic command like 
 *           'ls' and others command do not  work  as i expected <need to be investigate later> 
 * #FIXME!! TODO:: use strtok or strtok_r instead  but need to be investigate first */

/* !NOTICE:  For Quick Patching*/
#define   USUALSYS_UNIX_PATH  {\
  "/usr/bin/",                 \
  "/usr/sbin/",                \
  "/usr/local/bin/",           \
  "/sbin/",                    \
  "/bin/",                     \
  __nptr                       \
}

static char *const _PATH[]=USUALSYS_UNIX_PATH ; 

#define _Nullable
#define DEFAULT_SHELL  "/bin/sh"

//#TODO:  puts this  in meson configuration 
#define  DOCKER_INSPECT(__cid)\
  "docker","inspect","--format","{{.State.Pid}}",__cid,__nptr 

#define perr(__fcall)\
  do { perror(#__fcall); exit(EXIT_FAILURE); }while(0) 

#define  __inline_char(__charval)  (const char[]){ (__charval) & 0xff}

  //{"user", CLONE_NEWUSER}, 
#define  NS_DEFINITIONS {  \
  {"ipc" , CLONE_NEWIPC} , \
  {"uts" , CLONE_NEWUTS} , \
  {"net" , CLONE_NEWNET} , \
  {"pid" , CLONE_NEWPID} , \
  {"mnt" , CLONE_NEWNS}  , \
  {__nptr,~0}\
}

typedef struct __ns_info_t  ns_info_t ; 
struct __ns_info_t{
  const char *_ns_name ;
  int  _ns_flags_type;  
} nsinfo[] = NS_DEFINITIONS ; 

typedef pid_t   cpid_t  ; 

/* @fn has_command(const char*) 
 * @brief  check if the host has the commmand 
 * @param  const char * -- command string 
 * @return 0 OK ; Othewise error 
 */
static int has_command(const char *__restrict__ __host_command ) ;   

/* @fn die(const char *)  
 * @brief  just die 
 * @param  const char * - mesg 
 * */
static __noreturn void  die(const char *mesg) { 
  fprintf(stderr , "%s\n",  mesg) ; 
  exit(EXIT_FAILURE) ; 
}

/* @fn check_docker_is_available(void) 
 * @brief  check if docker is available  on the system 
 * this function  will  run  befor the main function 
 * */
static __always_inline __ctor void check_docker_is_available(void) {
   
  if(!has_command("docker")) 
    die("Require Docker") ; 
} 

/* @fn defsh(const char * _Nullable) 
 * @brief execute default shell. But can execute command 
 * @param  const char *  - shell or command  
 * */
static __always_inline void defsh(const char * _Nullable  custom)
{ 
	const char *shell = (custom)? custom  : getenv("SHELL"); 
	
    if (!shell)
		shell = DEFAULT_SHELL; 
  
	if(execlp(shell, shell , __nptr)) 
       perr(execl) ; 
}


/* @fn container_process_id(const char *)  
 *
 * @brief  get running docker container process identifier 
 * @param  const char  * -   container id  {2-3 characters} 
 * @return running container process id  
 */ 
cpid_t  container_process_id(char * __container_id );  


/* @fn load_ns_for(cpid_t) 
 * @brief load namespace for corresponding process relate to running docker process 
 * @param  cpid_t - container running process  
 */
static void   load_ns_for(cpid_t)  ; 


char docker_location[0x64]={0} ; 

int main(int ac , char **av , char **env)   
{
   int pstatus = EXIT_SUCCESS ;  
   setvbuf(stdout ,  __nptr ,  _IONBF , 0 ) ; 
   char *id= *(av+(ac-1)) ; 
   cpid_t chid =  container_process_id(id) ; 
   
   if( 0 == chid )
   {
     fprintf(stderr , "Not able to get docker container running process\n") ; 
     pstatus = EXIT_FAILURE ; 
   }
   
   fprintf(stdout , "loading namespace for host container %i\n" , chid); 
   load_ns_for(chid); 
   defsh(__nptr); 

   return  pstatus ; 
}


static int has_command(const char * restrict  host_command) 
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
   int internal_ipc_tunnel[2] ; 
   cpid_t  container_host_process_id  = 0 ;  
   if(pipe(internal_ipc_tunnel))
     perr(pipe) ; 
  
   int spawn  = fork() ; 
   if(~0 == spawn) perr(fork) ;  
   
   if (0 == spawn) 
   {
     close(*internal_ipc_tunnel) ;
     dup2(*(internal_ipc_tunnel+1),  STDOUT_FILENO) ; 
     int status = execvp( docker_location , ((char *const[]){DOCKER_INSPECT(container_id)}))    ;  
     if (~0 ==status) 
       perr(execvp);  
    
     exit(status) ;   
   }else{ 
     int status = 0 ; 
     wait(&status);  
     if (status & 0x00ff) 
     {
       fprintf(stderr , "Cannot get container process id \n") ; 
        exit(EXIT_FAILURE); 
     } 
     close(*(internal_ipc_tunnel+1));  
     char cpid[10]={0};  
     size_t b = read(*(internal_ipc_tunnel), cpid , 10);  
     assert(!b^strlen(cpid)) ; 
     close(*(internal_ipc_tunnel)) ; 
     container_host_process_id = strtol(cpid , __nptr  , 0xa ) ;  
    
   }
   return  container_host_process_id ; 
     
}

static void  load_ns_for(cpid_t container_process_id)  
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
