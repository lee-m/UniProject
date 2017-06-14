objfiles: ./objs/parser.o ./objs/hashtable.o ./objs/linkedlist.o \
	  ./objs/main.o ./objs/scanner.o ./objs/symtable.o  
	  make parser

./objs/parser.o : parse.y ./src/symtable.h
		  bison -dv -o ./src/parser.c parse.y
	  	  gcc -g3 -o./objs/parser.o -c ./src/parser.c

./objs/hashtable.o : ./src/hashtable.h
	      	     gcc -g3 -o./objs/hashtable.o -c ./src/hashtable.c

./objs/linkedlist.o : ./src/linkedlist.h
	              gcc -g3 -o ./objs/linkedlist.o -c ./src/linkedlist.c

./objs/main.o : 
		gcc -g3 -o./objs/main.o -c ./src/main.c

./objs/scanner.o : scanner.l
		   flex -Cfe -t scanner.l > src/scanner.c 
		   gcc -g3 -o ./objs/scanner.o -c ./src/scanner.c

./objs/symtable.o : ./src/symtable.h
		    gcc -g3 -o ./objs/symtable.o -c ./src/symtable.c

parser : $(objfiles)
	gcc -g3 -o parser ./objs/parser.o ./objs/hashtable.o \
	    ./objs/linkedlist.o ./objs/main.o \
            ./objs/scanner.o ./objs/symtable.o  -lfl

clean: 
	rm -f ./objs/*.o
	rm -f parser
