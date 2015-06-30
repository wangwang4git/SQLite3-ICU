reg.out : reg.o
	gcc -o reg.out reg.o -licuuc -licui18n

reg.o : reg.c
	gcc -c reg.c

clean :
	rm reg.out reg.o