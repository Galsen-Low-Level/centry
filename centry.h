/* @file 
 * 
 */
#if !defined(__centry_h) 
#define  __centry_h   

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

#define __BEGIN_SPAWN_CONTEXT(__sbp_identifier) if( 0 ==__sbp_identifier ) { 
#define __END_SPAWN_CONTEXT } 
#define __OTHERWISE else 

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
} ;  

extern struct __ns_info_t nsinfo[6]; 

typedef pid_t   cpid_t  ; 

/* @fn has_command(const char*) 
 * @brief  check if the host has the commmand 
 * @param  const char * -- command string 
 * @return 0 OK ; Othewise error 
 */
int has_command(const char *__restrict__ __host_command ) ;   

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
static  __always_inline __ctor void check_docker_is_available(void) {
   
  if(!has_command("docker")) 
    die("Require Docker") ; 
} 

/* @fn defsh(const char * _Nullable) 
 * @brief execute default shell. But can execute command 
 * @param  const char *  - shell or command  
 * */
static   __always_inline void defsh(const char * _Nullable  custom)
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
void   load_ns_for(cpid_t)  ; 


/*  @fn  spawn(const char *)
 *  @brief spwan   a  shell command 
 *  @param const char *  command 
 *  @parm  char *  output buffer result 
 **/
static int spwn(char *const * __sh_cmd ,  char * __buffer_result) ; 

/* @fn centry_check_running_container(const char *) 
 * @brief check running container  depending wich engine is use  
 *        for now docker is supported  [LXC/LXD] w'll be supported soon 
 * @param  const char *  container engine name 
 * */
char *centry_check_running_container_for(const char * __container_engine) ; 

/* @fn centry_curses_select(char * __buffer_result) 
 * @brief make curse  interaction selection  using term and curse  
 * @param  char *  the buffer to be writen  on term 
 *         but the select frame will be writen on first index of  buffer_result  
 *         you can access it by deferencing  the 1st index address 
 */

void centry_curses_select(char (*__buffer_result) [0xff] ,  int size ) ; 
#endif  //!__centry_h 
