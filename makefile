compile: shell.c
	gcc -lreadline shell.c
run:
	./a.out
clean:
	rm *~
	rm a.out
