rm -f src/parser.c src/parser.h src/scanner.c
flex -Cfe -t scanner.l > src/scanner.c
bison -dv -o src/parser.c parse.y
cd src
gcc -O3 -o parser main.c parser.c scanner.c linkedlist.c -lfl
