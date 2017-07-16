#include "fol.hpp"

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost 
   adrese parsirane formule. */
extern Formula parsed_formula;
/*Atom u konstruktoru zahteva signaturu, dodata ova var u parser.ypp*/
extern Signature sig;

int main()
{
  /*Formula f1, f2;

  yyparse();
  if(parsed_formula.get() != 0)
    f1 = Formula(parsed_formula);

  yyparse();
  if(parsed_formula.get() != 0)
    f2 = Formula(parsed_formula);

  cout << f1 << endl;
  cout << f2 << endl;*/


/******************************************************/

    Signature s;
    Substitution sub;
    //f(c, a)
    //f(f(u,v),w)
   /* Term c = make_shared<VariableTerm>("c");
    Term a = make_shared<VariableTerm>("a");
    Term u = make_shared<VariableTerm>("u");
    Term v = make_shared<VariableTerm>("v");
    Term w = make_shared<VariableTerm>("w");
    s.addFunctionSymbol("F", 2);
    s.addFunctionSymbol("Q", 2);
    Term ff1 = make_shared<FunctionTerm>(s, "F", vector<Term> { c,a });
     cout << "bilo ssta";

    Term ff2 = make_shared<FunctionTerm>(s, "F", vector<Term> { { u,v } });
    Term ff3 = make_shared<FunctionTerm>(s, "Q", vector<Term> { {ff2, w}});
    cout << "bilo ta";
    TermPairs tp3 { { ff1, ff3 } };
    sub.clear();*/
    /*if(unify(tp3, sub))
      {
        cout << "Unifiable" << endl;
        cout << sub << endl;
      }
    else
      {
        cout << "Not unifiable" << endl;
      }*/
    s.addPredicateSymbol("=",2);
    /*Formula ada = make_shared<Equality>(s, c, ff3);
    cout << ada << endl;
    cout << ada->equalTo(ada) << endl;
    cout << ada->equalTo(make_shared<Equality>(s, a, ff3)) << endl;
    Substitution tmp;
    tmp.push_back(make_pair("w",ff2));
    cout << ada->substitute(tmp) << endl;*/

    CNF cnf(5);
    s.addPredicateSymbol("=", 2);
    s.addPredicateSymbol("~=", 2);
    s.addFunctionSymbol("f",2);
    s.addFunctionSymbol("g",1);
    s.addPredicateSymbol("n",1);

    Term u = make_shared<VariableTerm>("u");
    Term w = make_shared<VariableTerm>("w");
    Term z = make_shared<VariableTerm>("z");
    Term a = make_shared<VariableTerm>("a");
    Term b = make_shared<VariableTerm>("b");

   Term fuw = make_shared<FunctionTerm>(s, "f", vector<Term> {{u,w}});
   Term ffuwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{fuw,z}});
   Term fwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{w,z}});
   Term fufwz = make_shared<FunctionTerm>(s, "f", vector<Term> {{u,fwz}});
   Formula prva = make_shared<Equality>(s, ffuwz, fufwz);
   cnf[0].push_back(prva);

   Formula nu = make_shared<Atom>(s, "n", vector<Term> {{u}});
   Formula druga2 = make_shared<Equality>(s, fuw, w);
   cnf[0].push_back(make_shared<Not>(nu));
   cnf[0].push_back(druga2);

   cnf[1].push_back(make_shared<Not>(nu));
   Term fwu = make_shared<FunctionTerm>(s, "f", vector<Term> {{w,u}});
   Formula treca = make_shared<Equality>(s, fwu, w);
   cnf[1].push_back(treca);

  /* cnf[3].push_back(nu);
   Term gu = make_shared<FunctionTerm>(s, "g", vector<Term> {{u}});
   Term fugu = make_shared<FunctionTerm>(s, "f", vector<Term> {{u,gu}});
   Formula treca2 = make_shared<Disequality>(s, fugu, gu);
   cnf[3].push_back(treca2);
   Term fguu = make_shared<FunctionTerm>(s, "f", vector<Term> {{gu,u}});
   Formula treca3 = make_shared<Disequality>(s, fguu, gu);
   cnf[3].push_back(treca3);*/

   Formula na = make_shared<Atom>(s, "n", vector<Term> {{a}});
   cnf[2].push_back(na);

   Formula nb = make_shared<Atom>(s, "n", vector<Term> {{b}});
   cnf[3].push_back(nb);

   Formula arb = make_shared<Disequality>(s, a, b);
   cnf[4].push_back(arb);

   cout << cnf << endl;


   /*8) f(a, w) = w     (2, 5)
   9) f(w, a) = w     (3, 5)
   10) f(b, w) = w    (2, 6)
   11) f(w, b) = w    (3, 6)
   12) a = b          (Paramodulacija, 11, 8)*/

/*   Term faw = make_shared<FunctionTerm>(s, "f", vector<Term>{{a,w}});
   Formula fawEw = make_shared<Equality>(s, faw, w);
   Term fwb = make_shared<FunctionTerm>(s, "f", vector<Term>{{w,b}});
   Formula fwbEw = make_shared<Equality>(s, fwb, w);
   cnf[5].push_back(fawEw);
   cnf[6].push_back(fwbEw);

   cout << cnf << endl;

   cout << tryResolveClauses(cnf, 6, 5) << endl;*/


   if(resolution(cnf))
     {
       cout << "CNF satisfiable!" << endl;
     }
   else
     {
       cout << "CNF unsatisfiable!" << endl;
     }

   /*Formula ajb = make_shared<Equality>(s, w, b);
   Clause tmpcl;
   tmpcl.push_back(ajb);tmpcl.push_back(arb);
   cout << clauseTautology(tmpcl) << endl;*/

/*
   Formula test1 = make_shared<Equality>(s, gu, w);

   Formula test2 = make_shared<Equality>(s, make_shared<FunctionTerm>(s, "g",vector<Term> {{a}}), b);

   cout << unify(test1, test2, sub) << sub << endl;


/*   CNF cnf2(2);
   cnf2[0].push_back(test1);cnf2[0].push_back(na);
   cnf2[1].push_back(test2);cnf2[1].push_back(nb);
   cout << tryResolveClauses(cnf2, 0, 1) << endl;
*/

  return 0;
}
