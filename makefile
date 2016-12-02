compile: shell.c
	gcc shell.c -Wall
run:
	gcc shell.c -Wall ./a.out
clean:
	rm *~
	rm a.out
