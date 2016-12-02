compile: shell.c
	gcc shell.c -Wall -L/usr/local/lib -I/usr/local/include -lreadline
run:
	./a.out
clean:
	rm *~
	rm a.out
