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
#include <err.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <errno.h>
#include <sys/utsname.h> 
#include <assert.h> 
#include <poll.h> 

#include <term.h> 
#include <curses.h>
#include <termios.h> 

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
   int internal_pdf[2]; 
   if(pipe(internal_pdf))
   {
     perr(pipe) ;  
     return ~0 ; 
   }

   pid_t subprocess =  fork()  ; 
   if(~0 == subprocess) 
   {
     perr(fork); 
     return ~0 ; 
   }
                
   __BEGIN_SPAWN_CONTEXT(subprocess) 
     close(*internal_pdf) ; 
     dup2(*(internal_pdf+1)  , STDOUT_FILENO) ; 
     int status  =  execvp(docker_location, shcmd) ; 
     if(~0  == status) 
       exit(EXIT_FAILURE) ; 

   __END_SPAWN_CONTEXT 
   __OTHERWISE {
    
     int sbp_stat =0 ; 
     wait(&(sbp_stat)) ; 
     if (  sbp_stat & 0x00ff ) 
       fprintf(stderr , "Error Occured while getting output\n"); 

     close(*(internal_pdf+1)) ; 
     size_t rbytes = read(*internal_pdf , buffer_result,10000) ;  
     assert(!(rbytes^strlen(buffer_result))) ;  

   }

   return 0 ; 
}


char  *  centry_check_running_container(const char * container_name) 
{
  char *_oah  = __nptr ; 
  ssize_t  bytes_erase =0,
           size = 0 , 
           to_be_copied=0; 
  char *const cmd[] = {(char*)container_name , "ps" , __nptr};  
  char  outbuffer[10000] ={0} ; 
  if(spwn(cmd , outbuffer))
  {
    perr(centry_check_running_container) ; 
    return ; 
  } 

  /*  NOTICE : Formating the output buffer */   
  _oah = strchr(outbuffer, 0xa) ; // Escape header
  bytes_erase =  (_oah - outbuffer) ;  
  size = strlen(outbuffer) ; 
  to_be_copied =  size - bytes_erase; 

  memcpy(outbuffer , (outbuffer+bytes_erase+1) , to_be_copied) ; 
  memset((outbuffer+to_be_copied) , 0 ,  size - to_be_copied) ; 

  char container_list[10][0xff] = {0} ;  
  char *  ob = (char *) outbuffer ; 
  int i =0; 
  while( ++i )  
  { 
    char * data =  ob; 
    //!  shorten outbuffer i just need the  container id and images name 
    //!TODO : put this in  speciale function 
    char *token  = __nptr , 
       *buff = strdup(ob) , 
       cid_img[0xff]={0}; 
    int token_limit =3;   
    while ( __nptr !=  (token = strtok(buff,__inline_char(0x20))))  
    {
       buff = __nptr ; 
      char s[100]={0} ;
      sprintf(s , "%s " , token); 
      strcat(cid_img , s ); 
      bzero(s, 100) ; 
      if(0 ==  (token_limit+=~(token_limit^token_limit)) )
        break; 
          

    } 
    printf(">>> %s \n",  cid_img) ;
    char *n  = strchr(data , 0xa) ;  
    if (!n)
    {
      free(buff) ;
      break ; 
    }
    ob=  (n+1) ;
    free(buff) ; 
    memcpy( (container_list+i-1) , cid_img ,strlen(cid_img)) ; 
  } 
    
   //printf(" %i , %s " ,  i ,  *container_list) ; 
   //printf("%s " ,  *(container_list+1))  ; 
   bzero(outbuffer , 10000) ; 
   centry_curses_select(container_list  , i-1) ; 
   char *id_token = strtok(*container_list , __inline_char(0x20))  ; 
   tputs(clear_screen ,  1, putchar) ; 
   if (!id_token) 
   {
     return  __nptr ;   
   }
   return  strdup(id_token) ;  
}



void centry_curses_select(char (*buffer_result) [0xff] ,   int size  ) 
{  //*TODO : all the logic of this function will be explode  in multiple function later  
  /*  Initial termios  */  
  struct termios  termsetup[2] ;
  unsigned int tcfg_status = 0 ; 
  tcfg_status |=  tcgetattr(STDIN_FILENO , (termsetup)) ; 
  tcfg_status |=  tcgetattr(STDIN_FILENO, (termsetup+1)) ;  

  if (tcfg_status)  
    goto __termio_setup_failor ; 

  (termsetup)->c_lflag &=~(ICANON)  ;
  if(tcsetattr(STDOUT_FILENO ,TCSANOW ,  termsetup)) 
    goto __termio_setup_failor; 
   
  goto  __setup_term;  
__termio_setup_failor: 
    warn("Fail to setup the terminal Sorry \nTerm io broken");  
    return;  

__setup_term : 
    
    int erret = 0 ; 
    if (ERR == setupterm(__nptr ,  STDOUT_FILENO , &erret)) 
    {
      switch(erret) 
      { 
        case 1 :
          warn("Cannot use  curses") ; break ; 
        case 0: 
          warn("Too few information to run curses"); break ; 
        case ~0: 
          warn("Term curse database not found") ; break ;   
      } 
      goto __termio_setup_failor; 
    } 

    //!  Initialising polling on keyboard  
    struct pollfd  kbd = { 
      .fd =  STDIN_FILENO, 
      .events = POLLIN,
      0
    }; 
  
    //! Term dimension 
    unsigned int termscope=0; 
    termscope|=(lines<<8)|columns ;  
    
    //! Initial setup 
    tputs(clear_screen , 1 , putchar) ;  
    tputs(cursor_invisible , 1  , putchar)  ; 
    tputs(tgoto(cursor_address , 0, 0) , 1 , putchar) ;  
    tputs(tparm(set_a_background, COLOR_WHITE+1) , 1 , putchar) ; 
    //! start listening in keyboard 
    printf("Centry") ; 
    tputs(tparm(set_a_background,1), 1, putchar) ; 
    tputs(tgoto(cursor_address , 0, termscope & 0xff) , 1 , putchar) ;  
    printf("Hint: <w s a d> to move arround, Enter  or Space  to select ,q/Q to Quit") ; 
    char *version =  "Version: 0.0.1-a";
    tputs(tgoto(cursor_address , abs((termscope &0xff) - strlen(version)), termscope>>8 ), 1, putchar) ; 
    printf("%s",version) ; 

    int select_cursor  = 0; 
    int index_selected = 1;
    char  result[1000] = {0}; 
    while(index_selected & 0xf) 
    {
      int i = ~0 ;   
      if(0 == strlen(*buffer_result)) 
      {
        tputs(tgoto(cursor_address , 32 ,10+i ), 1, putchar) ;  
        tputs(tparm(set_a_foreground, COLOR_WHITE+3+i) , 1 , putchar) ; 
        printf("!No Running container found") ;
        tputs(tgoto(cursor_address ,0,termscope >>8 ), 1 , putchar) ;   
        continue ; 
      } 
      while( ++i< size  ) 
      {
        tputs(tgoto(cursor_address , 32 ,10+i )   , 1, putchar) ;  
        tputs(tparm(set_a_foreground, COLOR_WHITE+3+i) , 1 , putchar) ;
        if ( select_cursor == i )  
        {
          tputs(tparm(set_a_background , COLOR_WHITE+1) , 1, putchar) ;  
          sprintf(result ,  "%s", *(buffer_result+select_cursor)) ; 
          printf("[*]\t%s", result ) ; 

        }else {
          tputs(tparm(set_a_background ,0), 1, putchar) ;  
          printf("[ ]\t%s", *(buffer_result+i)) ;  
        }
       
      } 
      if(~0 == poll(&kbd ,  1 ,  ~0))
        break ; 
      if( kbd.revents & POLLIN) 
      { 
         char kchar= getc(stdin) ; 
         //!Big apologies to azerty users
         //!TODO :Implement  navigation keys for AZERTY users or 
         //       Implement  key_arrows navigation: 
         //      -> detect keyboard layout  [Feat] 
         //      
         switch (  (kchar & 0xff)) 
         {
           case  'w': 
           case  'd': 
             select_cursor+=~(select_cursor^select_cursor) ;
             if(select_cursor  < 0 ) select_cursor = size-1 ;
             break;  
           case  's': 
           case  'a': 
             select_cursor=-~select_cursor ; 
             select_cursor%=size ; 
               break;
          case  0x20: /*Space  or */ 
          case  0x0a: /*Enter  : to Validate */
               index_selected^=1; 
               break ; 
          case 0x71:
          case 0x51: 
               //!TODO : Make more clear 
               tputs(cursor_normal ,1,putchar); 
               tputs(clear_screen , 1 , putchar) ;  
               tputs(exit_attribute_mode, 1, putchar) ; 
               tcsetattr(STDIN_FILENO , 0   , (termsetup+1))  ; 
               exit(1) ;
               break ; 
         }
       
       }
    } 
    bzero(*(buffer_result) , 0xff) ; 
    memcpy(*(buffer_result)  , result , strlen(result)) ; 

    tputs(cursor_normal ,1,putchar); 
    tputs(exit_attribute_mode, 1, putchar) ; 
    tcsetattr(STDIN_FILENO , 0   , (termsetup+1))  ; 

    return ; 
    
}
