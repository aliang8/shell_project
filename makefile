compile: shell.c
	gcc shell.c -Wall -lreadline
run:
	./a.out
clean:
	rm *~
	rm a.out
