ESP devices kernel module 
 
For this task, you need to create a kernel module which would allow to detect connected ESP devices to the computer/router. The kernel module should create necessary files for the ESP devices in the /dev directory if needed. 
This module should allow to communicate with the ESP devices. /sys directory could be used to create necessary files to communicate between computer/router and ESP device. Kernel module should be able to write to ESP device and read the response from it. 