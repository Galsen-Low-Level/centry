# CENTRY 

### What is Centry 

Centry is a simple, lightweight program that lets you access the inside of a running Docker container just like in interactive tty mode. 
The only prerequisite is to have Docker already available on your system. 

### Quick demo 
```bash 
#  center  <short-id-container{2-3}> 
```
Exemple : 
```bash 
$> docker ps 
CONTAINER ID   IMAGE                     COMMAND           CREATED        STATUS        PORTS     NAMES
c3784c6459b9   <image>                   "sleep 9999999"   24 hours ago   Up 24 hours            festive_ramanujan
-------------------------------------------------------------------------------------------------------------------

#> center c37   
```
For more option  use the -h or --help command 

### COMPILE  

Quick and easy to compile 
```bash 
$> gcc  centry.c  -o <your_final_exec> 
```

### OPTION 
If you have meson 

```bash 
$>  meson setup build 
$>  meson compile -C build 
$>  meson install -C build  #To install localy 
``` 

To uninstall 
```bash 
#> ninja uninstall -C build 
```  
