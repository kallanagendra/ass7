default: oss user

oss: oss.c clock.o
	gcc -Wall -g oss.c clock.o -o oss

user: user.c clock.o
	gcc -Wall -g user.c clock.o -o user

clock.o: clock.c clock.h
	gcc -Wall -g -c clock.c

clean:
	rm -f oss user clock.o
