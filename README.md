#Julia set computation gui

###Recommended steps:
* Compile with help of makefile (complex numbers library must be included)
  
  `make`
* Create pipes with _create_pipes.sh_
  
    `./create_pipes.sh`
* Run module and main files.

    `./prga-main`
`./prgsem-comp_module`
  
* In arguments while executing the files, you can specify your own pipes. Keep the same order of arguments for both files
  
* Program can be controlled from command line or gui window (only quitting is possible just from terminal)

* Have fun with following commands:
    
case 'g': Get version

case 'a': Abort computation

case 's': Set computation parameters

case '1': Compute via module

case 'r': Reset chunk for computation

case 'c': Compute via local PC

case 'p': Refresh gui

case 'l': Clear computation buffer

case 'z': Zoom

case 'h': Decrease Zoom

case 'd': Move view down

case 'n': Move view left

case 'm': Move view right

case 'u': Move view up

case 'o': Set original parameters

case '2': Set different parameters

case '3': Set different parameters

case '4': Set different parameters

case '5': Set different parameters

case '6': Set different parameters

case 'q': Quit the 


_Program contains some mistakes, for example module computation returns different output then PC computation and d fck idk why... please let me know if you see the bug:) _
