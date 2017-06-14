#include <stdio.h>
#include <stdlib.h>

int yyparse();
extern FILE* yyin;
extern int yynerrs;
extern struct symbol_table *symtab;

int main(int argc, char **argv)
{
  if(argc == 1)
  {
    printf("no input file specified \n");
    return 1;
  }

  yyin = fopen(argv[1], "r");

  if(!yyin)
  {
    printf("unable to open input file %s\n", argv[1]);
    return 1;
  }

  symtab = create_symbol_table();
  yyparse();

  if(yynerrs == 0)
    printf("parsing completed successfully\n");
  else 
    printf("%d errors occurred during parsing\n", yynerrs);

  destroy_symbol_table(symtab);
  return 0;
}

