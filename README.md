CSHELL PROJECT

by Anthony Liang, Shaeq Ahmed, Sam Xu

SHELL FEATURES (OVERVIEW):
      
      Intro screen (Cool ASCII text and colors)
      
      Basic shell functionalities and built in commands

      Implemented Cd and exit

      Simple Redirection 

      	     Stdin (<)

      	     Stdout (>)

      	     Pipe (|)
      
      Can link multiple pipes together

      More advanced redirection (&>,2>>,>>)

      Dynamically reallocation of user input 

      Parse multiple commands with ;  

      Ignores weird spacing (“ls           -l”)

      Prints bash prompt in linux format 

      	     <user>@<hostname>:<cwd>$

	     ~ if current working directory is home directory
	     
      Implemented autocomplete binded to TAB 

      Stores command history, can access with UP arrow key

      Cd prints out error statement if doesn’t exist


INSTRUCTIONS:
      
      Compile: make
      		
      Run: make run
      
      Clean: make clean
      
DOCUMENTATION:
      
      Check out documentation/CShell.pdf for more info!

LIMITATIONS AND BUGS:
     
      Did not implement redirection + pipe (ls | wc > foo.txt)