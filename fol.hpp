#ifndef FOL_H
#define FOL_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/* Funkcijski i predikatski simboli */
typedef string FunctionSymbol;
typedef string PredicateSymbol;

/* Signatura se sastoji iz funkcijskih i predikatskih simbola kojima
   su pridruzene arnosti (nenegativni celi brojevi) */
class Signature {
private:
  map<FunctionSymbol, unsigned> _functions;
  map<PredicateSymbol, unsigned> _predicates;

public:
  /* Dodavanje funkcijskog simbola date arnosti */
  void addFunctionSymbol(const FunctionSymbol &f, unsigned arity);

  /* Dodavanje predikatskog simbola date arnosti */
  void addPredicateSymbol(const PredicateSymbol &p, unsigned arity);

  /* Provera da li postoji dati funkcijski simbol, i koja mu je arnost */
  bool checkFunctionSymbol(const FunctionSymbol &f, unsigned &arity) const;

  /* Provera da li postoji dati predikatski simbol, i koja mu je arnost */
  bool checkPredicateSymbol(const PredicateSymbol &f, unsigned &arity) const;
};

/* Tip podatka za predstavljanje varijable */
typedef string Variable;

/* Skup varijabli */
typedef set<Variable> VariableSet;

class BaseTerm;
typedef std::shared_ptr<BaseTerm> Term;

/* Uopstena supstitucija */
typedef vector<pair<Variable, Term>> Substitution;

/* Funkcija prikazuje uopstenu supstituciju u preglednom obliku */
ostream &operator<<(ostream &ostr, const Substitution &sub);

/* Apstraktna klasa BaseTerm koja predstavlja termove */
class BaseTerm : public enable_shared_from_this<BaseTerm> {

public:
  /* Termovi mogu biti ili varijable ili funkcijski simboli primenjeni
     na (0 ili vise) termova */
  enum Type { TT_VARIABLE, TT_FUNCTION };

  /* Vraca tip terma */
  virtual Type getType() const = 0;

  /* Prikazuje term */
  virtual void printTerm(ostream &ostr) const = 0;

  /* Ispituje sintaksnu jednakost termova */
  virtual bool equalTo(const Term &t) const = 0;

  /* Vraca skup svih varijabli koje se pojavljuju u termu */
  virtual void getVars(VariableSet &vars) const = 0;

  /* Odredjuje da li se data varijabla nalazi u termu */
  bool containsVariable(const Variable &v) const;

  /* Uopstena zamena */
  virtual Term substitute(const Substitution &sub) = 0;

  /* Zamena varijable v termom t */
  Term substitute(const Variable &v, const Term &t);

  virtual ~BaseTerm() {}
};

ostream &operator<<(ostream &ostr, const Term &t);

/* Term koji predstavlja jednu varijablu */
class VariableTerm : public BaseTerm {
private:
  Variable _v;

public:
  VariableTerm(const Variable &v);
  virtual Type getType() const;
  const Variable &getVariable() const;
  virtual void printTerm(ostream &ostr) const;
  virtual bool equalTo(const Term &t) const;
  virtual void getVars(VariableSet &vars) const;
  virtual Term substitute(const Substitution &sub);
};

/* Term koji predstavlja funkcijski simbol primenjen na odgovarajuci
   broj podtermova */
class FunctionTerm : public BaseTerm {
private:
  const Signature &_sig;
  FunctionSymbol _f;
  vector<Term> _ops;

public:
  FunctionTerm(const Signature &s, const FunctionSymbol &f,
               const vector<Term> &ops);
  FunctionTerm(const Signature &s, const FunctionSymbol &f,
               vector<Term> &&ops = vector<Term>());

  virtual Type getType() const;
  const Signature &getSignature() const;
  const FunctionSymbol &getSymbol() const;
  const vector<Term> &getOperands() const;
  virtual void printTerm(ostream &ostr) const;
  virtual bool equalTo(const Term &t) const;
  virtual void getVars(VariableSet &vars) const;
  virtual Term substitute(const Substitution &sub);
};

class BaseFormula;
typedef std::shared_ptr<BaseFormula> Formula;

typedef vector<Formula> LiteralList;
typedef vector<LiteralList> LiteralListList;

/* Apstraktna klasa kojom se predstavljaju formule */
class BaseFormula : public enable_shared_from_this<BaseFormula> {

public:
  /* Tipovi formula (dodatak u odnosu na iskaznu logiku su formule
     kod kojih je vodeci simbol univerzalni ili egzistencijalni
     kvantifikator */
  enum Type {
    T_TRUE,
    T_FALSE,
    T_ATOM,
    T_NOT,
    T_AND,
    T_OR,
    T_IMP,
    T_IFF,
    T_FORALL,
    T_EXISTS,
    T_EQUAL,
    T_DISEQUAL
  };

  /* Prikaz formule */
  virtual void printFormula(ostream &ostr) const = 0;

  /* Tip formule */
  virtual Type getType() const = 0;

  /* Slozenost formule */
  virtual unsigned complexity() const = 0;

  /* Sintaksna jednakost dve formule */
  virtual bool equalTo(const Formula &f) const = 0;

  /* Ocitava sve varijable koje se pojavljuju u formuli. Ako
     je zadat drugi parametar sa vrednoscu true, tada se izdvajaju samo
     slobodne varijable u formuli */
  virtual void getVars(VariableSet &vars, bool free = false) const = 0;

  /* Ispituje da li se varijabla pojavljuje u formuli (kao slobodna ili
     vezana) */
  bool containsVariable(const Variable &v, bool free = false) const;

  /* Uopstena zamena */
  virtual Formula substitute(const Substitution &sub) = 0;

  Formula substitute(const Variable &v, const Term &t);

  virtual Formula simplify() = 0;
  virtual Formula nnf() = 0;
  virtual Formula pullquants() = 0;
  virtual Formula prenex() = 0;
  virtual Formula skolem(Signature &s,
                         vector<Variable> &&vars = vector<Variable>());
  virtual LiteralListList listCNF() = 0;

  virtual ~BaseFormula() {}
};

ostream &operator<<(ostream &ostr, const Formula &f);

/* Klasa predstavlja sve atomicke formule (True, False i Atom) */
class AtomicFormula : public BaseFormula {

public:
  virtual unsigned complexity() const;
  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
};

/* Klasa predstavlja logicke konstante (True i False) */
class LogicConstant : public AtomicFormula {

public:
  virtual bool equalTo(const Formula &f) const;
  virtual void getVars(VariableSet &vars, bool free) const;
  virtual Formula substitute(const Substitution &sub);
};

/* Klasa predstavlja True logicku konstantu */
class True : public LogicConstant {

public:
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja logicku konstantu False */
class False : public LogicConstant {

public:
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja atom, koji za razliku od iskazne logike ovde ima
   znatno slozeniju strukturu. Svaki atom je predikatski simbol primenjen
   na odgovarajuci broj podtermova */
class Atom : public AtomicFormula {
protected:
  const Signature &_sig;
  PredicateSymbol _p;
  vector<Term> _ops;

public:
  Atom(const Signature &s, const PredicateSymbol &p, const vector<Term> &ops);
  Atom(const Signature &s, const PredicateSymbol &p,
       vector<Term> &&ops = vector<Term>());

  const PredicateSymbol &getSymbol() const;
  const Signature &getSignature() const;
  const vector<Term> &getOperands() const;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual bool equalTo(const Formula &f) const;

  virtual void getVars(VariableSet &vars, bool free) const;
  virtual Formula substitute(const Substitution &sub);
  virtual LiteralListList listCNF();
};

class Equality : public Atom {
public:
  Equality(const Signature &s, const Term &lop, const Term &rop)
      : Atom(s, "=", {lop, rop}) {}

  const Term &getLeftOperand() const;

  const Term &getRightOperand() const;

  virtual Type getType() const;
  virtual bool equalTo(const Formula &f) const;
  virtual Formula substitute(const Substitution &sub);

  virtual void printFormula(ostream &ostr) const;
};

class Disequality : public Atom {
public:
  Disequality(const Signature &s, const Term &lop, const Term &rop)
      : Atom(s, "~=", {lop, rop}) {}

  const Term &getLeftOperand() const;

  const Term &getRightOperand() const;

  virtual Type getType() const;
  virtual bool equalTo(const Formula &f) const;
  virtual Formula substitute(const Substitution &sub);

  virtual void printFormula(ostream &ostr) const;
};

/* Klasa unarni veznik (obuhvata negaciju) */
class UnaryConjective : public BaseFormula {
protected:
  Formula _op;

public:
  UnaryConjective(const Formula &op);
  const Formula &getOperand() const;
  virtual unsigned complexity() const;
  virtual bool equalTo(const Formula &f) const;
  virtual void getVars(VariableSet &vars, bool free) const;
};

/* Klasa koja predstavlja negaciju */
class Not : public UnaryConjective {
public:
  using UnaryConjective::UnaryConjective;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja sve binarne veznike */
class BinaryConjective : public BaseFormula {
protected:
  Formula _op1, _op2;

public:
  BinaryConjective(const Formula &op1, const Formula &op2);
  const Formula &getOperand1() const;
  const Formula &getOperand2() const;
  virtual unsigned complexity() const;
  virtual bool equalTo(const Formula &f) const;
  virtual void getVars(VariableSet &vars, bool free) const;
};

/* Klasa predstavlja konjunkciju */
class And : public BinaryConjective {
public:
  using BinaryConjective::BinaryConjective;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja disjunkciju */
class Or : public BinaryConjective {
public:
  using BinaryConjective::BinaryConjective;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja implikaciju */
class Imp : public BinaryConjective {
public:
  using BinaryConjective::BinaryConjective;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja ekvivalenciju */
class Iff : public BinaryConjective {

public:
  using BinaryConjective::BinaryConjective;
  virtual void printFormula(ostream &ostr) const;
  virtual Type getType() const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja kvantifikovane formule */
class Quantifier : public BaseFormula {
protected:
  Variable _v;
  Formula _op;

public:
  Quantifier(const Variable &v, const Formula &op);
  const Variable &getVariable() const;
  const Formula &getOperand() const;
  virtual unsigned complexity() const;
  virtual bool equalTo(const Formula &f) const;
  virtual void getVars(VariableSet &vars, bool free) const;
  virtual LiteralListList listCNF();
};

/* Klasa predstavlja univerzalno kvantifikovanu formulu */
class Forall : public Quantifier {
public:
  using Quantifier::Quantifier;
  virtual Type getType() const;
  virtual void printFormula(ostream &ostr) const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual Formula skolem(Signature &s, vector<Variable> &&vars);
};

/* Klasa predstavlja egzistencijalnog kvantifikatora */
class Exists : public Quantifier {
public:
  using Quantifier::Quantifier;
  virtual Type getType() const;
  virtual void printFormula(ostream &ostr) const;
  virtual Formula substitute(const Substitution &sub);

  virtual Formula simplify();
  virtual Formula nnf();
  virtual Formula pullquants();
  virtual Formula prenex();
  virtual Formula skolem(Signature &s, vector<Variable> &&vars);
};

/*!*/
template <typename T1, typename T2>
Variable getUniqueVariable(const T1 &e1, const T2 &e2) {
  static unsigned i = 0;

  Variable v;

  do {
    v = string("uv") + to_string(++i);
  } while (e1->containsVariable(v) || e2->containsVariable(v));

  return v;
}

/* Funkcija vraca varijablu koja se ne pojavljuje ni u f ni u jednom
   od termova iz ts */
Variable getUniqueVariable(const Formula &f, const vector<Term> &ts);

/* Funkcije za unifikaciju */

typedef list<pair<Term, Term>> TermPairs;
typedef list<pair<Formula, Formula>> AtomPairs;

/* Prikazuje na izlazu skup parova termova koje zelimo
   da unifikujemo */
ostream &operator<<(ostream &ostr, const TermPairs &pairs);

/* Ispituje da li je skup parova termova unifikabilan, i vraca
   najopstiji unifikator ako jeste */
bool unify(const TermPairs &pairs, Substitution &sub);

/* Ispituje da li je skup parova atoma unifikabilan, i vraca
   najopstiji unifikator ako jeste */
bool unify(const AtomPairs &pairs, Substitution &sub);

/* Specijalni slucaj -- dva terma */
bool unify(const Term &t1, const Term &t2, Substitution &sub);

/* Specijalni slucaj -- dva atoma */
bool unify(const Formula &a1, const Formula &a2, Substitution &sub);

/* Pomocna funkcije za unifikaciju -- transformise skup parova
   termova primenjujuci pravila iz algoritma. */
bool do_unify(TermPairs &pairs);

/* Primenjuje pravilo factoring */
void applyFactoring(TermPairs &pairs);

/* Primenjuje pravilo tautology */
void applyTautology(TermPairs &pairs);

/* Primenjuje pravilo orientation i vraca true ako je tim pravilom nesto
   promenjeno */
bool applyOrientation(TermPairs &pairs);

/* Primenjuje pravilo decomposition/collision i vraca true ako je tim
   pravilom nesto promenjeno */
bool applyDecomposition(TermPairs &pairs, bool &collision);

/* Primenjuje pravilo application/cycle i vraca true ako je tim pravilom
   nesto promenjeno. */
bool applyApplication(TermPairs &pairs, bool &cycle);

/* Funkcije za rezoluciju */

/* Klazu je predstavljena kao niz literala */
typedef vector<Formula> Clause;

/* CNF formula je niz klauza */
typedef vector<Clause> CNF;

/* Prikazuje klauzu */
ostream &operator<<(ostream &ostr, const Clause &c);

/* Prikazuje CNF formulu */
ostream &operator<<(ostream &ostr, const CNF &cnf);

/* Vraca skup varijabli koje se pojavljuju u klauzi */
void getClauseVars(const Clause &c, VariableSet &vars);

/* Ispituje da li klauza sadrzi varijablu */
bool clauseContainsVariable(const Clause &c, const Variable &v);

/* Funkcija vraca novu jedinstvenu varijablu koja ne postoji ni u c1 ni u c2 */
Variable getUniqueVariable(const Clause &c1, const Clause &c2);

/* Ispituje da li klauza sadrzi literal */
bool clauseContainsLiteral(const Clause &c, const Formula &f);

/* Ispituje da li je klauza c1 sadrzana u klauzi c2 */
bool clauseSubsumed(const Clause &c1, const Clause &c2);

/* Vraca suprotni literal datog literala */
Formula oppositeLiteral(const Formula &l);

/* Ispituje da li je klauza c tautologija (tj. da li sadzi neki atom
   A i njegovu negaciju ~A) */
bool clauseTautology(const Clause &c);

/* Proverava da li u skupu postoji klauza koja je podskup date
   klauza c */
bool clauseExists(const CNF &cnf, const Clause &c);

/* Vrsi uopstenu zamenu u svim literalima klauze */
void substituteClause(Clause &c, const Substitution &sub);

/* Vrsi zamenu varijable v termom t u svim literalima klauze */
void substituteClause(Clause &c, const Variable &v, const Term &t);

/* Brise k-ti literal iz klauze */
void removeLiteralFromClause(Clause &c, unsigned k);

/* Nadovezuje dve klauze */
void concatClauses(const Clause &c1, const Clause &c2, Clause &cr);

/* Primenjuje pravilo rezolucije nad klauzama c1 i c2 po literalima
   na pozicijama i, ondosno j, primenjujuci substituciju sub */
void resolveClauses(const Clause &c1, const Clause &c2, unsigned i, unsigned j,
                    const Substitution &sub, Clause &res);
/**/
void resolveClauses(const Clause &c1, const Clause &c2, unsigned i,
                    const Substitution &sub, Clause &res);

/* Ispituje da li je moguce primeniti pravilo rezolucije nad klauzama k i l
   i ako je moguce, primenjuje ga na sve moguce nacine i dodaje dobijene klauze
   u CNF */
bool tryResolveClauses(CNF &cnf, unsigned k, unsigned l, bool &prematureB);

/* Ispituje da li je moguce naci dve klauze nad kojima se moze primeniti
   pravilo rezolucije, i primenjuje ga ako je moguce, vracajuci rezolventnu
   klauzu res. Parametri i i j su tu zbog optimizacije, da bismo znali dokle
   smo stigli u pretrazi, tj. da je ne bismo pokretali svaki put iz pocetka. */
bool resolventFound(CNF &cnf, unsigned &i, unsigned &j);

/* Primenjuje pravilo grupisanja: literal na poziciji j se uklanja,
   a na ostale se primenjuje zamena sub */
void groupLiterals(const Clause &c, unsigned j, const Substitution &sub,
                   Clause &res);

/* Pokusava da primeni pravilo grupisanja na k-tu klauzu u formuli. Ukoliko uspe
   da na
   taj nacin dobije novu klauzu, vraca true i u CNF dodaje rezultujucu
   klauzu */
bool tryGroupLiterals(CNF &cnf, unsigned k);

/* Za sve klauze redom ispituje da li moze primeniti pravilo grupisanja
   na neki nacin. Parametar k se prenosi po referenci zbog efikasnosti
   (da ne bismo svaki put iz pocetka pokretali postupak) */
bool groupingFound(CNF &cnf, unsigned &k);

/* Metod rezolucije */
bool resolution(const CNF &cnf);

#endif
