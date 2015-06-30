ubreak.out : ubreak.o
	gcc -o ubreak.out ubreak.o -licuuc -licui18n

ubreak.o : ubreak.c
	gcc -c ubreak.c

clean :
	rm ubreak.out ubreak.o