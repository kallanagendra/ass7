1. Compile executables
  $ make
gcc -Wall -g -pedantic -c clock.c
gcc -Wall -ggdb -pedantic oss.c clock.o -o oss
gcc -Wall -g -pedantic user.c clock.o -o user

2. Execution
$ ./oss
[1; 704160000] User with PID=26397 started, will run for 375891 nsec
[1; 966000000] User with PID=26397 done
[3; 468960000] User with PID=26399 started, will run for 375620 nsec
[3; 696040000] User with PID=26399 done
[5; 164060000] User with PID=26400 started, will run for 35000 nsec

[5; 396660000] User with PID=26400 done
[19; 285080000] User with PID=26398 started, will run for 5000000 nsec
[19; 549760000] User with PID=26398 done
