#include "fol.hpp"

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost 
   adrese parsirane formule. */
extern Formula parsed_formula;
/*Atom u konstruktoru zahteva signaturu, dodata ova var u parser.ypp*/
extern Signature sig;

int main()
{
         Signature s;
         Substitution sub;
         s.addPredicateSymbol("=", 2);
         s.addPredicateSymbol("~=", 2);
         s.addFunctionSymbol("f",2);
         s.addFunctionSymbol("g",1);
         s.addFunctionSymbol("a",0);
         s.addFunctionSymbol("b",0);
         s.addPredicateSymbol("n",1);

         CNF cnf(5);

         Term u = make_shared<VariableTerm>("u");
         Term w = make_shared<VariableTerm>("w");
         Term z = make_shared<VariableTerm>("z");
         Term a = make_shared<FunctionTerm>(s, "a");
         Term b = make_shared<FunctionTerm>(s, "b");


         Term fuw = make_shared<FunctionTerm>(s, "f", vector<Term> {{u,w}});
         Term ffuwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{fuw,z}});
         Term fwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{w,z}});
         Term fufwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{u,fwz}});
         Formula prva = make_shared<Equality>(s, ffuwz, fufwz);
//	 cnf[0].push_back(prva);

         Formula nu = make_shared<Atom>(s, "n", vector<Term> {{u}});
         Formula druga2 = make_shared<Equality>(s, fuw, w);
         cnf[0].push_back(make_shared<Not>(nu));
         cnf[0].push_back(druga2);

         cnf[1].push_back(make_shared<Not>(nu));
         Term fwu = make_shared<FunctionTerm>(s, "f", vector<Term> {{w,u}});
         Formula treca = make_shared<Equality>(s, fwu, w);
         cnf[1].push_back(treca);


         Formula na = make_shared<Atom>(s, "n", vector<Term> {{a}});
         cnf[2].push_back(na);

         Formula nb = make_shared<Atom>(s, "n", vector<Term> {{b}});
         cnf[3].push_back(nb);

         Formula arb = make_shared<Disequality>(s, a, b);
         cnf[4].push_back(arb);

           /*8) f(a, w) = w     (2, 5)
           9) f(w, a) = w     (3, 5)
           10) f(b, w) = w    (2, 6)
           11) f(w, b) = w    (3, 6)
           12) a = b          (Paramodulacija, 11, 8)*/

         /*  Term faw = make_shared<FunctionTerm>(s, "f", vector<Term>{{a,w}});
           Formula fawEw = make_shared<Equality>(s, faw, w);
           Term fwb = make_shared<FunctionTerm>(s, "f", vector<Term>{{w,b}});
           Formula fwbEw = make_shared<Equality>(s, fwb, w);
           cnf[5].push_back(fawEw);
           cnf[6].push_back(fwbEw);*/

           if(resolution(cnf))
             {
               cout << "CNF satisfiable!" << endl;
             }
           else
             {
               cout << "CNF unsatisfiable!" << endl;
             }
  return 0;
}
