#include "fol.hpp"

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost 
   adrese parsirane formule. */
extern Formula parsed_formula;
/*Atom u konstruktoru zahteva signaturu, dodata ova var u parser.ypp*/
extern Signature sig;

int main()
{
  yyparse();
  
  if(parsed_formula.get() != 0)
    cout << parsed_formula;
  
  cout << endl;
  return 0;
}
