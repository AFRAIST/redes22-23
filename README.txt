There are some macros you can tweak in "./include/defines_specific.h",
"./include/defines.h" and other header files to change the timers,
default ip, etc..

The server does no timeouts because we create a process for each
request, so we can always be listening. The OS will take care of
handling the processes and used memory by using swapping mechanisms.

To select the words haphazardly, you should disable the FOR_TEST
macro. You can add the flag -DFOR_TEST to the Makefile CFLAGS
variable to enable the macro and remove the flag to keep it disabled.
The project ships with the macro enabled and picks the words
sequentially to avoid non-determinism.
