#include "fol.hpp"
#include <cctype>
#include <cstdlib>
#include <unistd.h>

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost
   adrese parsirane formule. */
extern Formula parsed_formula;
// Definisana u parser.ypp
extern Signature sig;

bool debugFlag = false;

int main(int argc, char **argv) {
  int c;
  while ((c = getopt(argc, argv, "d")) != -1)
    switch (c) {
    case 'd':
      debugFlag = true;
      break;
    default:
      cerr << "Usage: " << argv[0] << " [-d]" << endl;
      return 1;
    }

  yyparse();

  if (parsed_formula.get() != 0)
    cout << "Formula:\t" << parsed_formula << endl;
  else
    return 1;

  cout << "NNF:\t\t";
  cout << parsed_formula->simplify()->nnf() << endl;

  cout << "Prenex:\t\t";
  cout << parsed_formula->simplify()->nnf()->prenex() << endl;
  
  Formula skl = parsed_formula->simplify()->nnf()->prenex()->skolem(sig);
  cout << "Skolem:\t\t";
  cout << skl << endl;

  while (skl->getType() == BaseFormula::T_FORALL)
    skl = ((Forall *)skl.get())->getOperand();

  cout << "Bez univ kvant:\t";
  cout << skl << endl;

  cout << "CNF:\t\t";
  LiteralListList cnfLista = skl->listCNF();
  cout << cnfLista << endl << endl;
  
  cout << (resolution(cnfLista) ? "SAT" : "UNSAT") << endl;
  
  return 0;
}
