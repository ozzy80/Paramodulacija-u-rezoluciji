#include "fol.hpp"

// Definicije funkcija clanica -----------------------------------------

// Funkcije substitucije -----------------------------------------------

Term BaseTerm::substitute(const Variable & v, const Term & t)
{
  return substitute({{v, t}});
}

Formula  BaseFormula::substitute(const Variable & v, const Term & t)
{
  return substitute({{v, t}});
}

Term VariableTerm::substitute(const Substitution & sub) 
{
  for(unsigned i = 0; i < sub.size(); i++)
    if(_v == sub[i].first)
      return sub[i].second;
  
    return shared_from_this();
}

Term FunctionTerm::substitute(const Substitution & sub) 
{
  vector<Term> sub_ops;
  
  for(unsigned i = 0; i < _ops.size(); i++)
    sub_ops.push_back(_ops[i]->substitute(sub));
  
  return make_shared<FunctionTerm>(_sig, _f, sub_ops);
}

Formula LogicConstant::substitute(const Substitution & sub) 
{
  return shared_from_this();
}

Formula Atom::substitute(const Substitution & sub) 
{
  vector<Term> sub_ops;
  
  for(unsigned i = 0; i < _ops.size(); i++)
    sub_ops.push_back(_ops[i]->substitute(sub));
  
  return make_shared<Atom>(_sig, _p, sub_ops); 
}

Formula Not::substitute(const Substitution & sub) 
{
  return make_shared<Not>(_op->substitute(sub));
}

Formula And::substitute(const Substitution & sub) 
{
  return make_shared<And>(_op1->substitute(sub), _op2->substitute(sub));
}

Formula Or::substitute(const Substitution & sub) 
{
  return make_shared<Or>(_op1->substitute(sub), _op2->substitute(sub));
}

Formula Imp::substitute(const Substitution & sub) 
{
  return make_shared<Imp>(_op1->substitute(sub), _op2->substitute(sub));
}

Formula Iff::substitute(const Substitution & sub) 
{
  return make_shared<Iff>(_op1->substitute(sub), _op2->substitute(sub));
}


Formula Forall::substitute(const Substitution & sub)
{
  Substitution subnv;
  
  for(unsigned i = 0; i < sub.size(); i++)
    if(sub[i].first != _v)
      subnv.push_back(sub[i]);

  if(subnv.size() == 0)
    return shared_from_this();

  /* Proveravamo da li se varijabla _v nalazi u bar nekom od termova */
  bool contained = false;
  for(unsigned i = 0; i < subnv.size(); i++) {
    contained = subnv[i].second->containsVariable(_v);
	if(contained) break;
  }

  /* Ako neki od termova sadrzi kvantifikovanu varijablu, tada moramo najpre
     preimenovati kvantifikovanu varijablu (nekom varijablom koja
     nije sadzana ni u termovima ni u formuli sto nam daje funkcija
     getUniqueVariable) */
    if(contained)
      {
	vector<Term> ts;
	for(unsigned i = 0; i < subnv.size(); i++)
	  ts.push_back(subnv[i].second);
	
	Variable new_v = getUniqueVariable(shared_from_this(), ts);
	
	Formula sub_op = _op->substitute(_v, make_shared<VariableTerm>(new_v));
	return make_shared<Forall>(new_v, sub_op->substitute(subnv));
      }
    else
      return make_shared<Forall>(_v, _op->substitute(subnv)); 
}

Formula Exists::substitute(const Substitution & sub)
{
  Substitution subnv;
  
  for(unsigned i = 0; i < sub.size(); i++)
    if(sub[i].first != _v)
      subnv.push_back(sub[i]);
  
  if(subnv.size() == 0)
    return shared_from_this();
  
  /* Proveravamo da li se varijabla _v nalazi u bar nekom od termova */
  bool contained = false;
  for(unsigned i = 0; i < subnv.size(); i++) {
    contained = subnv[i].second->containsVariable(_v);
	if(contained) break;
  }
  
  /* Ako neki od termova sadrzi kvantifikovanu varijablu, tada moramo najpre
     preimenovati kvantifikovanu varijablu (nekom varijablom koja
     nije sadzana ni u termovima ni u formuli sto nam daje funkcija
     getUniqueVariable) */
  if(contained)
    {
      vector<Term> ts;
      for(unsigned i = 0; i < subnv.size(); i++)
	ts.push_back(subnv[i].second);
      
      Variable new_v = getUniqueVariable(shared_from_this(), ts);
      
      Formula sub_op = _op->substitute(_v, make_shared<VariableTerm>(new_v));
      return make_shared<Exists>(new_v, sub_op->substitute(subnv));
    }
  else
    return make_shared<Exists>(_v, _op->substitute(subnv)); 
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje sintaksne identicnosti termova i formula ---

bool VariableTerm::equalTo(const Term & t) const
{
  return t->getType() == TT_VARIABLE && 
    ((VariableTerm *) t.get())->getVariable() == _v;
}

bool FunctionTerm::equalTo(const Term & t) const
{
  if(t->getType() != TT_FUNCTION)
    return false;
  
  if(_f != ((FunctionTerm *) t.get())->getSymbol())
    return false;
  
  const vector<Term> & t_ops = ((FunctionTerm *) t.get())->getOperands();
  
  if(_ops.size() != t_ops.size())
    return false;
  
  for(unsigned i = 0; i < _ops.size(); i++)
    if(!_ops[i]->equalTo(t_ops[i]))
      return false;
  
  return true;
}

bool LogicConstant::equalTo( const Formula & f) const
{
  return f->getType() == this->getType();
}


bool Atom::equalTo(const Formula & f) const
{
  if(f->getType() != T_ATOM)
    return false;
  
  if(_p != ((Atom *) f.get())->getSymbol())
    return false;
  
  const vector<Term> & f_ops = ((Atom *) f.get())->getOperands();
  
  if(_ops.size() != f_ops.size())
    return false;
  
  for(unsigned i = 0; i < _ops.size(); i++)
    if(!_ops[i]->equalTo(f_ops[i]))
      return false;
  
    return true;
}

bool UnaryConjective::equalTo(const Formula & f) const
{
  return f->getType() == this->getType() && 
    _op->equalTo(((UnaryConjective *)f.get())->getOperand());
}

bool BinaryConjective::equalTo( const Formula & f) const
{
  return f->getType() == this->getType() && 
    _op1->equalTo(((BinaryConjective *)f.get())->getOperand1()) 
    &&  
    _op2->equalTo(((BinaryConjective *)f.get())->getOperand2());
}

bool Quantifier::equalTo(const Formula & f) const
{
  return f->getType() == getType() &&
    ((Quantifier *) f.get())->getVariable() == _v && 
    ((Quantifier *) f.get())->getOperand()->equalTo(_op);
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje skupa varijabli ----------------------------

void VariableTerm::getVars(VariableSet & vars) const
{
  vars.insert(_v);
}

void FunctionTerm::getVars(VariableSet & vars) const
{
  for(unsigned i = 0; i < _ops.size(); i++)
    _ops[i]->getVars(vars);
}

void LogicConstant::getVars(VariableSet & vars, bool free) const
{
  return;
}

void Atom::getVars(VariableSet & vars, bool free) const
{
  for(unsigned i = 0; i < _ops.size(); i++)
    {
      _ops[i]->getVars(vars);
    }
}

void UnaryConjective::getVars(VariableSet & vars, bool free) const
{
  _op->getVars(vars, free);
}

void BinaryConjective::getVars(VariableSet & vars, bool free) const
{
  _op1->getVars(vars, free);
  _op2->getVars(vars, free);
}

void Quantifier::getVars(VariableSet & vars, bool free) const
{
  bool present = false;

  if(free)
    {
      /* Pamtimo da li je kvantifikovana varijabla vec u skupu slobodnih
	 varijabli */
      if(vars.find(_v) != vars.end())
	present = true;
    }
  
  _op->getVars(vars, free);
  if(!free)
    vars.insert(_v);
  
  if(free)
    {
      /* Ako varijabla ranije nije bila prisutna u skupu slobodnih varijabli,
	 tada je brisemo, zato sto to znaci da se ona pojavljuje samo u 
	 podformuli kvantifikovane formule,a u njoj je vezana kvantifikatorom */
      if(!present && vars.find(_v) != vars.end())
	vars.erase(_v);
    }
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje slozenosti formule -------------------------

unsigned AtomicFormula::complexity() const
{
  return 0;
}  

unsigned UnaryConjective::complexity() const
{
  return _op->complexity() + 1;
}

unsigned BinaryConjective::complexity() const
{
  return _op1->complexity() + _op2->complexity() + 1;
}

unsigned Quantifier::complexity() const
{
  return _op->complexity() + 1;
}

// ---------------------------------------------------------------------

// Funkcije za stampanje -----------------------------------------------


void VariableTerm::printTerm(ostream & ostr) const
{
  ostr << _v;
}

void FunctionTerm::printTerm(ostream & ostr) const
{
  ostr << _f;

  for(unsigned i = 0; i < _ops.size(); i++)
    {
      if(i == 0)
	ostr << "(";
      ostr << _ops[i];
      if(i != _ops.size() - 1)
	ostr << ",";
      else
	ostr << ")";
    }
}

void True::printFormula(ostream & ostr) const
{
  ostr << "True";
}

void False::printFormula(ostream & ostr) const
{
  ostr << "False";
}

void Atom::printFormula(ostream & ostr) const
{
  ostr << _p;
  for(unsigned i = 0; i < _ops.size(); i++)
    {
      if(i == 0)
	ostr << "(";
      ostr << _ops[i];
      if(i != _ops.size() - 1)
	ostr << ",";
      else
	ostr << ")";
    }
}

 void Equality::printFormula(ostream & ostr) const
{
  _ops[0]->printTerm(ostr);
  ostr << " = ";
  _ops[1]->printTerm(ostr);
}

void Disequality::printFormula(ostream & ostr) const
{

  _ops[0]->printTerm(ostr);
  ostr << " ~= ";
  _ops[1]->printTerm(ostr);
}

void Not::printFormula(ostream & ostr) const
{ 
  ostr << "~";
  Type op_type = _op->getType();

  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << "(";

  _op->printFormula(ostr);

  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << ")";
}

void And::printFormula(ostream & ostr) const
{
  Type op1_type = _op1->getType();
  Type op2_type = _op2->getType();

  if(op1_type == T_OR || op1_type == T_IMP || 
     op1_type == T_IFF)
    ostr << "(";
  
  _op1->printFormula(ostr);
  
  if(op1_type == T_OR || op1_type == T_IMP || 
     op1_type == T_IFF)
    ostr << ")";

  ostr << " & ";

  if(op2_type == T_OR || op2_type == T_IMP || 
     op2_type == T_IFF || op2_type == T_AND)
    ostr << "(";
  
  _op2->printFormula(ostr);

  if(op2_type == T_OR || op2_type == T_IMP || 
     op2_type == T_IFF || op2_type == T_AND)
    ostr << ")";
}

void Or::printFormula(ostream & ostr) const
{

  Type op1_type = _op1->getType();
  Type op2_type = _op2->getType();

  if(op1_type == T_IMP || op1_type == T_IFF)
    ostr << "(";
  
  _op1->printFormula(ostr);
  
  if(op1_type == T_IMP || op1_type == T_IFF)
    ostr << ")";

  ostr << " | ";

  if(op2_type == T_IMP || 
     op2_type == T_IFF || op2_type == T_OR)
    ostr << "(";
  
  _op2->printFormula(ostr);

  if(op2_type == T_IMP || 
     op2_type == T_IFF || op2_type == T_OR)
    ostr << ")";
}

void Imp::printFormula(ostream & ostr) const
{

  Type op1_type = _op1->getType();
  Type op2_type = _op2->getType();

  if(op1_type == T_IFF)
    ostr << "(";
  
  _op1->printFormula(ostr);
  
  if(op1_type == T_IFF)
    ostr << ")";

  ostr << " => ";

  if(op2_type == T_IMP || op2_type == T_IFF)
    ostr << "(";
  
  _op2->printFormula(ostr);

  if(op2_type == T_IMP || op2_type == T_IFF)
    ostr << ")";
}

void Iff::printFormula(ostream & ostr) const
{

  Type op1_type = _op1->getType();
  Type op2_type = _op2->getType();
  
  _op1->printFormula(ostr);
  
  ostr << " => ";

  if(op2_type == T_IFF)
    ostr << "(";
  
  _op2->printFormula(ostr);

  if(op2_type == T_IFF)
    ostr << ")";

}

void Forall::printFormula(ostream & ostr) const
{
  cout << "![" << _v << "] : ";

  Type op_type = _op->getType();
  
  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << "(";

  _op->printFormula(ostr);

  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << ")";
}

void Exists::printFormula(ostream & ostr) const
{
  cout  << "?[" << _v << "] : ";
 
  Type op_type = _op->getType();
  
  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << "(";

  _op->printFormula(ostr);
  
  if(op_type == T_AND || op_type == T_OR || 
     op_type == T_IMP || op_type == T_IFF)
    ostr << ")";
}


ostream & operator << (ostream & ostr, const Term & t)
{
  t->printTerm(ostr);
  return ostr;
}

ostream & operator << (ostream & ostr, const Formula & f)
{
  f->printFormula(ostr);
  return ostr;
}

// ---------------------------------------------------------------------

ostream & operator << (ostream & ostr, const Substitution & sub)
{
  ostr << "[ ";
  for(unsigned i = 0; i < sub.size(); i++)
    {     
      ostr << sub[i].first << " ---> " << sub[i].second;
      if(i < sub.size() - 1)
	ostr << ", ";      
    }
  ostr << " ]";

  return ostr;
}

// Funkcije za simplifikaciju -------------------------------------------

/* Simplifikacija atomicke formule je trivijalna */
Formula AtomicFormula::simplify()
{
  return shared_from_this();
}

Formula Not::simplify()
{
  Formula simp_op = _op->simplify();
  
  if(simp_op->getType() == T_TRUE)
    return make_shared<False>();
  else if(simp_op->getType() == T_FALSE)
    return make_shared<True>();
  else
    return make_shared<Not>(simp_op);
}

Formula And::simplify()
{
  /* Simplifikacija konjukcije po pravilima A /\ True === A, 
     A /\ False === False i sl. */
  Formula simp_op1 = _op1->simplify();
  Formula simp_op2 = _op2->simplify();
  
  if(simp_op1->getType() == T_TRUE)
    {
      return simp_op2;
    }
  else if(simp_op2->getType() == T_TRUE)
    return simp_op1;
  else if(simp_op1->getType() == T_FALSE ||
	  simp_op2->getType() == T_FALSE)
    return make_shared<False>();
  else
    return make_shared<And>(simp_op1, simp_op2);
}

Formula Or::simplify()
{
  /* Simplifikacija disjunkcije po pravilima: A \/ True === True,
     A \/ False === A, i sl. */
  Formula simp_op1 = _op1->simplify();
  Formula simp_op2 = _op2->simplify();
  
  if(simp_op1->getType() == T_FALSE) 
    return simp_op2;
  else if(simp_op2->getType() == T_FALSE)
    return simp_op1;
  else if(simp_op1->getType() == T_TRUE ||
	  simp_op2->getType() == T_TRUE)
    return make_shared<True>();
  else
    return make_shared<Or>(simp_op1, simp_op2);
}

Formula Imp::simplify()
{
  /* Simplifikacija implikacije po pravilima: A ==> True === True,
     A ==> False === ~A, True ==> A === A, False ==> A === True */
  Formula simp_op1 = _op1->simplify();
  Formula simp_op2 = _op2->simplify();
  
  if(simp_op1->getType() == T_FALSE) 
    return make_shared<True>();
  else if(simp_op2->getType() == T_FALSE)
    return make_shared<Not>(simp_op1);
  else if(simp_op1->getType() == T_TRUE)
    return simp_op2;
  else if(simp_op2->getType() == T_TRUE)
    return make_shared<True>();
  else
    return make_shared<Imp>(simp_op1, simp_op2);
}

Formula Iff::simplify()
{
  /* Ekvivalencija se simplifikuje pomocu pravila:
     True <=> A === A, False <=> A === ~A i sl. */
  
  Formula simp_op1 = _op1->simplify();
  Formula simp_op2 = _op2->simplify();

  if(simp_op1->getType() == T_FALSE && simp_op2->getType() == T_FALSE)
    return make_shared<True>();
  else if(simp_op1->getType() == T_FALSE) 
    return make_shared<Not>(simp_op2);
  else if(simp_op2->getType() == T_FALSE)
    return make_shared<Not>(simp_op1);
  else if(simp_op1->getType() == T_TRUE)
    return simp_op2;
  else if(simp_op2->getType() == T_TRUE)
    return simp_op1;
  else
    return make_shared<Iff>(simp_op1, simp_op2);
}

Formula Forall::simplify()
{
  Formula simp_op = _op->simplify();
  
  /* Ako simplifikovana podformula sadrzi slobodnu varijablu v, tada
     zadrzavamo kvantifikator, u suprotnom ga brisemo */
  if(simp_op->containsVariable(_v, true))
    return  make_shared<Forall>(_v, simp_op);
  else
    return simp_op;
}

Formula Exists::simplify()
{
  Formula simp_op = _op->simplify();

  /* Ako simplifikovana podformula sadrzi slobodnu varijablu v, tada
     zadrzavamo kvantifikator, u suprotnom ga brisemo */
  if(simp_op->containsVariable(_v, true))
    return make_shared<Exists>(_v, simp_op);
  else
    return simp_op;
}



// ---------------------------------------------------------------------


// NNF funkcije --------------------------------------------------------

Formula AtomicFormula::nnf()
{
  return shared_from_this();
}

Formula Not::nnf()
{
  /* Eliminacija dvojne negacije */
  if(_op->getType() == T_NOT)
    {
      Not * not_op = (Not *) _op.get();
      return not_op->getOperand()->nnf();
    }
  /* De-Morganov zakon ~(A/\B) === ~A \/ ~B, pa zatim rekurzivna 
     primena nnf-a na ~A i ~B */
  else if(_op->getType() == T_AND)
    {
      And * and_op =  (And *) _op.get();
	
      return make_shared<Or>(make_shared<Not>(and_op->getOperand1())->nnf(),
			     make_shared<Not>(and_op->getOperand2())->nnf());
      
    }
  /* De-Morganov zakon ~(A\/B) === ~A /\ ~B, pa zatim rekurzivna 
     primena nnf-a na ~A i ~B */
  else if(_op->getType() == T_OR)
    {
      Or * or_op =  (Or *) _op.get();
      
      return make_shared<And>(make_shared<Not>(or_op->getOperand1())->nnf(),
			      make_shared<Not>(or_op->getOperand2())->nnf());
      
    }
  /* De-Morganov zakon ~(A==>B) === A /\ ~B, pa zatim rekurzivna
     primena nnf-a na A i ~B */
  else if(_op->getType() == T_IMP)
    {
      Imp * imp_op =  (Imp *) _op.get();
	
      return make_shared<And>(imp_op->getOperand1()->nnf(),
			      make_shared<Not>(imp_op->getOperand2())->nnf());
      
    }
  /* Primena pravila ~(A<=>B) === (A /\ ~B) \/ (B /\ ~A) */
  else if(_op->getType() == T_IFF)
    {
      Iff * iff_op =  (Iff *) _op.get();
      
      return make_shared<Or>(make_shared<And>(iff_op->getOperand1()->nnf(),
					      make_shared<Not>(iff_op->getOperand2())->nnf()),
			     make_shared<And>(iff_op->getOperand2()->nnf(),
					      make_shared<Not>(iff_op->getOperand1())->nnf()));
      
    }
  /* Primena pravila ~(forall x) A === (exists x) ~A */ 
  else if(_op->getType() == T_FORALL)
    {
      Forall * forall_op = (Forall *) _op.get();      

      return make_shared<Exists>(forall_op->getVariable(), 
				 make_shared<Not>(forall_op->getOperand())->nnf());
    }
  /* Primena pravila ~(exists x) A === (forall x) ~A */
  else if(_op->getType() == T_EXISTS)
    {
      Exists * exists_op = (Exists *) _op.get();      
      
      return make_shared<Forall>(exists_op->getVariable(), 
				 make_shared<Not>(exists_op->getOperand())->nnf());
    }
  else
    {
      return shared_from_this();
    } 
}

Formula And::nnf()
{
  return make_shared<And>(_op1->nnf(), _op2->nnf());
}

Formula Or::nnf()
{
  return make_shared<Or>(_op1->nnf(), _op2->nnf());
}

Formula Imp::nnf()
{
  /* Eliminacija implikacije, pa zatim rekurzivna primena nnf()-a */
  return make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf());
}

Formula Iff::nnf()
{
  /* Eliminacija ekvivalencije, pa zatim rekurzivna primena nnf()-a.
     Primetimo da se ovde velicina formule duplira */
  return make_shared<And>(make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf()),
			  make_shared<Or>(make_shared<Not>(_op2)->nnf(), _op1->nnf()));
}

Formula Forall::nnf()
{
  return make_shared<Forall>(_v, _op->nnf());
}

Formula Exists::nnf()
{
  return make_shared<Exists>(_v, _op->nnf());
}


// POMOCNE FUNKCIJE ZA BARATANJE LISTAMA --------------------------------

/* Funkcija nadovezuje dve liste */
template <typename T>
T concatLists(const T & c1, const T & c2)
{
  T c = c1;
  
  for(auto it = c2.begin(), it_end = c2.end(); it != it_end; ++it)
    c.push_back(*it);

  //Krace:
  //c.resize(c1.size() + c2.size());
  //copy(c2.begin(), c2.end(), c.begin() + c1.size());

  // Ili, jos krace:
  //copy(c2.begin(), c2.end(), back_inserter(c));
  
  return c;
}


/* Funkcija nadovezuje svaku listu literala iz c1 sa svakom listom literala
   iz c2 i sve takve liste literala smesta u rezultujucu listu listi c */
LiteralListList makePairs(const LiteralListList & c1, 
			  const LiteralListList & c2)
	      
{
  LiteralListList c;

  for(auto & l1 : c1)
    for(auto & l2 : c2)
      c.push_back(concatLists(l1, l2));    
  return c;
}

/* Funkcija prikazuje listu listi */
ostream & operator << (ostream & ostr, const LiteralListList & l)
{
  ostr << "[ ";
  for(auto & ll :  l)
    {
      ostr << "[ ";
      for(auto & f : ll)
	{
	  ostr << f << " ";
	}
      ostr << "] ";
    }
  ostr << " ]";

  return ostr;
}


// ----------------------------------------------------------------------


// FUNKCIJE ZA ODREDJIVANJE CNF U OBLIKU LISTE LISTA LITERALA ----------

LiteralListList True::listCNF()
{
  /* Formuli True odgovara prazna lista klauza. Naime, po konvenciji,
     prazna lista klauza je zadovoljena u svakoj valuaciji (zato sto
     u skupu ne postoji klauza koja nije zadovoljena), pa je otuda
     logicki ekvivalentna formuli True. */
  return { };
}

LiteralListList False::listCNF()
{
  /* Formuli False odgovara lista klauza koja sadrzi samo jednu praznu
     klauzu. Po konvenciji, prazna klauza je netacna u svakoj valuaciji,
     (zato sto ne postoji literal koji je zadovoljen), pa je otuda 
     logicki ekvivalentna sa False */
  return {{}};
}

LiteralListList Atom::listCNF()
{
  /* Pozitivan literal (atom) se predstavlja listom klauza koja sadrzi
     samo jednu klauzu koja se sastoji iz tog jednog literala */
  return { { shared_from_this() } };
}

LiteralListList Not::listCNF()
{
  /* Negativan literal se predstavlja listom klauza koja sadrzi
     samo jednu klauzu koja se sastoji iz tog jednog literala */
  return { { shared_from_this() } };
}

LiteralListList And::listCNF()
{
  /* CNF lista se kod konjukcije dobija nadovezivanjem CNF listi
     podformula */
  LiteralListList cl1 = _op1->listCNF();
  LiteralListList cl2 = _op2->listCNF();

  
  return concatLists(cl1, cl2);
}

LiteralListList Or::listCNF()
{
  /* CNF lista disjunkcije se dobija tako sto se CNF liste podformula
     distributivno "pomnoze", tj. liste literala se nadovezu svaka sa
     svakom */
  
  LiteralListList cl1 = _op1->listCNF();
  LiteralListList cl2 = _op2->listCNF();
  
  return makePairs(cl1, cl2);
}

LiteralListList Imp::listCNF()
{
  throw "CNF not aplicable";
}

LiteralListList Iff::listCNF()
{
  throw "CNF not aplicable";
}

LiteralListList Quantifier::listCNF()
{
  throw "CNF not aplicable in Quantifier";
}

// -------------------------------------------------------------------

// Funkcije za izvlacenje kvantifikatora  ------------------------------

Formula AtomicFormula::pullquants()
{
  /* U slucaju atomicke formule, ne radimo nista */
  return shared_from_this();
}

Formula Not::pullquants()
{
  /* U slucaju negacije, ne radimo nista, s obzirom da je formula
     vec u NNF-u, pa su negacije spustene do nivoa atoma. */
  return shared_from_this();
}

Formula And::pullquants()
{
  /* Ako je formula oblika (forall x) A /\ (forall y) B */
  if(_op1->getType() == T_FORALL && _op2->getType() == T_FORALL)
    {
      Forall * fop1 = (Forall *) _op1.get();
      Forall * fop2 = (Forall *) _op2.get();
      
      /* Specijalno, ako je formula oblika (forall x) A /\ (forall x) B
	 tada ne moramo da preimenujemo vezanu promenljivu, vec samo
	 izvlacimo kvantifikator (forall x) (A /\ B), a zatim iz podformule
	 rekurzivno izvucemo kvantifikatore */
      if(fop1->getVariable() == fop2->getVariable())
	{
	  return make_shared<Forall>(fop1->getVariable(), 
				     make_shared<And>(fop1->getOperand(),
						      fop2->getOperand())->pullquants());
	}
      /* U suprotnom, uvodimo novu promenljivu koja se ne pojavljuje
	 ni u jednoj od  formula A i B */
      else {
	Variable var = 
	  getUniqueVariable(fop1->getOperand(), fop2->getOperand());
	
	return make_shared<Forall>(var, 
				   make_shared<And>(fop1->
						    getOperand()->
						    substitute(fop1->getVariable(),
							       make_shared<VariableTerm>(var)),
						    fop2->
						    getOperand()->
						    substitute(fop2->getVariable(),
							       make_shared<VariableTerm>(var)))->pullquants());
      } 
    }
  /* Slucaj ((exists x) A) /\ B */
  else if(_op1->getType() == T_EXISTS)
    {
      Exists * eop1 = (Exists *) _op1.get();
      
      /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
	 samo izvuci kvantifikator (exists x) (A /\ B) */
      if(!_op2->containsVariable(eop1->getVariable(), true))
	{
	  return make_shared<Exists>(eop1->getVariable(), 
				     make_shared<And>(eop1->getOperand(), _op2)->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(eop1->getOperand(), _op2);
	  return make_shared<Exists>(var, 
				     make_shared<And>(eop1->getOperand()->
						      substitute(eop1->getVariable(), 
								 make_shared<VariableTerm>(var)),
						      _op2)->pullquants());
	}
    }
  /* Slucaj A /\ (exists x) B */
  else if(_op2->getType() == T_EXISTS)
    {
      Exists * eop2 = (Exists *) _op2.get();
      
      /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
	 samo izvuci kvantifikator (exists x) (A /\ B) */
      if(!_op1->containsVariable(eop2->getVariable(), true))
	{
	  return make_shared<Exists>(eop2->getVariable(), 
				     make_shared<And>(_op1, eop2->getOperand())->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(eop2->getOperand(), _op1);
	  return make_shared<Exists>(var, 
				     make_shared<And>(_op1, 
						      eop2->getOperand()->
						      substitute(eop2->getVariable(), 
								 make_shared<VariableTerm>(var)))->pullquants());
	}
    }
  /* Slucaj ((forall x) A) /\ B */
  else if(_op1->getType() == T_FORALL)
    {
      Forall * fop1 = (Forall *) _op1.get();
      
      /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
	 samo izvuci kvantifikator (forall x) (A /\ B) */
      if(!_op2->containsVariable(fop1->getVariable(), true))
	{
	  return make_shared<Forall>(fop1->getVariable(), 
				     make_shared<And>(fop1->getOperand(), _op2)->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(fop1->getOperand(), _op2);
	  return make_shared<Forall>(var, make_shared<And>(fop1->getOperand()->
							   substitute(fop1->getVariable(), 
								      make_shared<VariableTerm>(var)),
							   _op2)->pullquants());
	}
    }
  /* Slucaj A /\ (forall x) B */
  else if(_op2->getType() == T_FORALL)
    {
      Forall * fop2 = (Forall *) _op2.get();
      
      /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
	 samo izvuci kvantifikator (forall x) (A /\ B) */
      if(!_op1->containsVariable(fop2->getVariable(), true))
	{
	  return make_shared<Forall>(fop2->getVariable(), 
				     make_shared<And>(_op1, fop2->getOperand())->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(fop2->getOperand(), _op1);
	  return make_shared<Forall>(var, 
				     make_shared<And>(_op1, 
						      fop2->getOperand()->
						      substitute(fop2->getVariable(), 
								 make_shared<VariableTerm>(var)))->pullquants());
	}
    }
  /* Formula je oblika A /\ B, gde ni A ni B nemaju kvantifikator kao vodeci
     veznik. U tom slucaju, ne radimo nista, jer su svi kvantifikatori izvuceni. */
  else
    {
      return shared_from_this();
    }
}



Formula Or::pullquants()
{
  /* Ako je formula oblika (exists x) A \/ (exists y) B */
  if(_op1->getType() == T_EXISTS && _op2->getType() == T_EXISTS)
    {
      Exists * fop1 = (Exists *) _op1.get();
      Exists * fop2 = (Exists *) _op2.get();
      
      /* Specijalno, ako je formula oblika (exists x) A \/ (exists x) B
	 tada ne moramo da preimenujemo vezanu promenljivu, vec samo
	 izvlacimo kvantifikator (exists x) (A \/ B), a zatim iz podformule
	 rekurzivno izvucemo kvantifikatore */
      if(fop1->getVariable() == fop2->getVariable())
	{
	  return make_shared<Exists>(fop1->getVariable(), 
				     make_shared<Or>(fop1->getOperand(),
						      fop2->getOperand())->pullquants());
	}
      /* U suprotnom, uvodimo novu promenljivu koja se ne pojavljuje
	 ni u jednoj od  formula A i B */
      else {
	Variable var = 
	  getUniqueVariable(fop1->getOperand(), fop2->getOperand());
	
	return make_shared<Exists>(var, 
				   make_shared<Or>(fop1->
						   getOperand()->
						   substitute(fop1->getVariable(),
							      make_shared<VariableTerm>(var)),
						   fop2->
						   getOperand()->
						   substitute(fop2->getVariable(),
							      make_shared<VariableTerm>(var)))->pullquants());
      } 
    }
  /* Slucaj ((exists x) A) \/ B */
  else if(_op1->getType() == T_EXISTS)
    {
      Exists * eop1 = (Exists *) _op1.get();
      
      /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
	 samo izvuci kvantifikator (exists x) (A /\ B) */
      if(!_op2->containsVariable(eop1->getVariable(), true))
	{
	  return make_shared<Exists>(eop1->getVariable(), 
				     make_shared<Or>(eop1->getOperand(), _op2)->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(eop1->getOperand(), _op2);
	  return make_shared<Exists>(var, 
				     make_shared<Or>(eop1->getOperand()->
						     substitute(eop1->getVariable(), 
								make_shared<VariableTerm>(var)),
						     _op2)->pullquants());
	}
    }
  /* Slucaj A \/ (exists x) B */
  else if(_op2->getType() == T_EXISTS)
    {
      Exists * eop2 = (Exists *) _op2.get();
      
      /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
	 samo izvuci kvantifikator (exists x) (A \/ B) */
      if(!_op1->containsVariable(eop2->getVariable(), true))
	{
	  return make_shared<Exists>(eop2->getVariable(), 
				     make_shared<Or>(_op1, eop2->getOperand())->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(eop2->getOperand(), _op1);
	  return make_shared<Exists>(var, 
				     make_shared<Or>(_op1, 
						     eop2->getOperand()->
						     substitute(eop2->getVariable(), 
								make_shared<VariableTerm>(var)))->pullquants());
	}
    }
  /* Slucaj ((forall x) A) \/ B */
  else if(_op1->getType() == T_FORALL)
    {
      Forall * fop1 = (Forall *) _op1.get();
      
      /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
	 samo izvuci kvantifikator (forall x) (A \/ B) */
      if(!_op2->containsVariable(fop1->getVariable(), true))
	{
	  return make_shared<Forall>(fop1->getVariable(), 
				     make_shared<Or>(fop1->getOperand(), _op2)->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(fop1->getOperand(), _op2);
	  return make_shared<Forall>(var, make_shared<Or>(fop1->getOperand()->
							  substitute(fop1->getVariable(), 
								     make_shared<VariableTerm>(var)),
							  _op2)->pullquants());
	}
    }
  /* Slucaj A \/ (forall x) B */
  else if(_op2->getType() == T_FORALL)
    {
      Forall * fop2 = (Forall *) _op2.get();
      
      /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
	 samo izvuci kvantifikator (forall x) (A \/ B) */
      if(!_op1->containsVariable(fop2->getVariable(), true))
	{
	  return make_shared<Forall>(fop2->getVariable(), 
				     make_shared<Or>(_op1, fop2->getOperand())->pullquants());
	}
      /* u suprotnom, moramo da preimenujemo vezanu varijablu */
      else
	{
	  Variable var = getUniqueVariable(fop2->getOperand(), _op1);
	  return make_shared<Forall>(var, 
				     make_shared<Or>(_op1, 
						     fop2->getOperand()->
						     substitute(fop2->getVariable(), 
								make_shared<VariableTerm>(var)))->pullquants());
	}
    }
  /* Formula je oblika A \/ B, gde ni A ni B nemaju kvantifikator kao vodeci
     veznik. U tom slucaju, ne radimo nista, jer su svi kvantifikatori izvuceni. */
  else
    {
      return shared_from_this();
    }

}

Formula Imp::pullquants()
{
  /* Implikacija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
  throw "pullquants not applicable";
}

Formula Iff::pullquants()
{
  /* Ekvivalencija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
  throw "pullquants not applicable";
}

Formula Forall::pullquants()
{
  /* U slucaju Forall formule, ne radimo nista */
  return shared_from_this();
}

Formula Exists::pullquants()
{
  /* U slucaju Exists formule, ne radimo nista */
  return shared_from_this();
}


// ---------------------------------------------------------------------


// ---------------------------------------------------------------------

// Funkcije za PRENEX normalnu formu

Formula AtomicFormula::prenex()
{
  /* U slucaju atomicke formule ne radimo nista */
  return shared_from_this();
}

Formula Not::prenex()
{
  /* U slucaju negacije ne radimo nista, zato sto podrazumevamo
     da je formula vec u NNF-u, pa su negacije spustene do atoma */
  return shared_from_this();
}

Formula And::prenex()
{
  /* U slucaju formule oblika A /\ B, najpre transforimisemo A i B
     u PRENEX, a zatim iz dobijene konjukcije izvucemo kvantifikatore */

  Formula pr_op1 = _op1->prenex();
  Formula pr_op2 = _op2->prenex();

  return make_shared<And>(pr_op1, pr_op2)->pullquants();
}

Formula Or::prenex()
{
  /* U slucaju formule oblika A \/ B, najpre transforimisemo A i B
     u PRENEX, a zatim iz dobijene disjunkcije izvucemo kvantifikatore */

  Formula pr_op1 = _op1->prenex();
  Formula pr_op2 = _op2->prenex();

  return make_shared<Or>(pr_op1, pr_op2)->pullquants();
}

Formula Imp::prenex()
{
  /* Implikacija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
  throw "Prenex not applicable";
}

Formula Iff::prenex()
{
  /* Ekvivalencija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
  throw "Prenex not applicable";
}

Formula Forall::prenex()
{
  /* U slucaju univerzalnog kvantifikatora, potrebno je samo podformulu
     svesti na prenex formu */
  return make_shared<Forall>(_v, _op->prenex());
}

Formula Exists::prenex()
{
  /* U slucaju egzistencijalnog kvantifikatora, potrebno je samo podformulu
     svesti na prenex formu */
  return make_shared<Exists>(_v, _op->prenex());
}

// -----------------------------------------------------------------------

// Funkcije za skolemizaciju ---------------------------------------------

/* Kod funkcija za skolemizaciju, podrazumeva se da je formula u PRENEX
   normalnoj formi, tj. oblika (Q1 x1) (Q2 x2) ... (Qn xn) A, gde su 
   Qi \in {forall, exists}, dok je A formula bez kvantifikatora koja ne
   sadrzi druge varijable osim x1,...,xn */

/*!*/
FunctionSymbol getUniqueFunctionSymbol(const Signature & s)
{
  static unsigned i = 0;
  unsigned arity;

  FunctionSymbol f;
  
  do {
    f = string("uf") + to_string(++i);
  } while(s.checkFunctionSymbol(f, arity));
  
  return f;
}

Formula BaseFormula::skolem(Signature & s, vector<Variable> && vars)
{
  /* Podrazumevano, za formulu bez kvantifikatora, ne radimo nista */
  return shared_from_this();
}

Formula Forall::skolem(Signature & s, vector<Variable> && vars)
{
  /* Ako je formula oblika (forall x) A, tada samo dodajemo varijablu
     x u sekvencu univerzalno kvantifikovanih varijabli, i zatim pozivamo
     rekurzivni poziv za podformulu */
  vars.push_back(_v);
  return make_shared<Forall>(_v, _op->skolem(s, std::move(vars)));
}

Formula Exists::skolem(Signature & s, vector<Variable> && vars)
{
  /* Kada naidjemo na egzistencijalni kvantifikator (exists y), 
     tada je potrebno eliminisati ga, uvodjenjem novog funkcijskog simbola 
     u signaturu, pa zato najpre uvodimo novi simbol koji nije prisutan u 
     signaturu s */

  FunctionSymbol f = getUniqueFunctionSymbol(s);
  
  /* Dodajemo f u signaturu sa arnoscu k = vars.size(), tj. arnost je jednaka
     broju univerzalno kvantifikovanih varijabli koje prethode egzistencijalnom
     kvantifikatoru. Primetimo da ako je k = 0, tj. ako nije bilo univerzalnih
     kvantifikatora pre datog egzistencijalnog, tada ce se uvesti u signaturu
     novi funkcijski simbol arnosti 0, tj. simbol konstante 
     (Skolemova konstanta). */
  s.addFunctionSymbol(f, vars.size());

  /* Kreiramo vektor termova x1,x2,...,xk */
  vector<Term> varTerms;

  for(unsigned i = 0; i < vars.size(); i++)
    varTerms.push_back(make_shared<VariableTerm>(vars[i]));

  /* Kreiramo term f(x1,...,xk) */
  Term t = make_shared<FunctionTerm>(s, f, varTerms);

  /* Zamenjujemo u podformuli y -> f(x1,...,xk), a zatim nastavljamo
     rekurzivno skolemizaciju u podformuli. */
  Formula tmp = _op->substitute(_v, t);

  return tmp->skolem(s, std::move(vars));
}

// Funkcije za unifikaciju -----------------------------------------------

ostream & operator << (ostream & ostr, const TermPairs & pairs)
{
  for(const auto & p : pairs)
    {
      ostr << "(" << p.first << ", " << p.second << ") ";      
    }
  return ostr;
}



bool unify(const AtomPairs & pairs, Substitution & sub)
{
  TermPairs term_pairs;

  for(auto i = pairs.begin(); i != pairs.end(); i++)
    {
      if(((Atom*)(*i).first.get())->getSymbol() == ((Atom*)(*i).second.get())->getSymbol())
	{
	  const vector<Term> & ops_f = ((Atom*)(*i).first.get())->getOperands();
	  const vector<Term> & ops_s = ((Atom*)(*i).second.get())->getOperands();

	  for(unsigned k = 0; k < ops_f.size(); k++)
	    term_pairs.push_back(make_pair(ops_f[k], ops_s[k]));
	}
      else
	return false;
    }
  return unify(term_pairs, sub);
}

bool unify(const TermPairs & pairs, Substitution & sub)
{
  TermPairs res_pairs = pairs;

  if(!do_unify(res_pairs))
    return false;

  for(auto i = res_pairs.cbegin(); i != res_pairs.cend(); i++)
    {
      VariableTerm * vt = (VariableTerm *) (*i).first.get();
      sub.push_back(make_pair(vt->getVariable(), (*i).second));
    }
  
  return true;
}

bool unify(const Term & t1, const Term & t2, Substitution & sub)
{
  return unify({{t1, t2}}, sub);
}

bool unify(const Formula & a1, const Formula & a2, Substitution & sub)
{
  return unify({{a1, a2}}, sub);
}


bool do_unify(TermPairs & pairs)
{
  bool repeat =  false;
  bool collision = false;
  bool cycle = false;

  do {
    
    applyFactoring(pairs);    
    applyTautology(pairs);

    repeat = 
      applyOrientation(pairs) ||
      applyDecomposition(pairs, collision) ||
      applyApplication(pairs, cycle);
    
    if(collision || cycle)
      return false;
    
  } while(repeat);
  
  return true;
}


void applyFactoring(TermPairs & pairs)
{
  for(auto i = pairs.begin(); i != pairs.end(); i++)
    {
      auto j = i;
      j++;
      while(j != pairs.end())
	{
	  if((*j).first->equalTo((*i).first) &&
	     (*j).second->equalTo((*i).second))
	    {
	      cout << "Factoring applied: (" << (*j).first << "," << (*j).second << ")" << endl;
	      auto erase_it = j;
	      j++;
	      pairs.erase(erase_it);
	      cout << pairs << endl;
	    }
	  else
	    j++;
	}
    }
}

void applyTautology(TermPairs & pairs)
{
  auto i = pairs.begin();
  
  while(i != pairs.end())
    {
      if((*i).first->equalTo((*i).second))
	{
	  cout << "Tautology applied: (" << (*i).first << "," << (*i).second << ")" << endl;
	  auto erase_it = i;
	  i++;
	  pairs.erase(erase_it);
	  cout << pairs << endl;
	}
      else
	i++;
    }
}

bool applyOrientation(TermPairs & pairs)
{
  bool ret = false;

  for(auto i = pairs.begin(); i != pairs.end(); i++)
    {
      if((*i).first->getType() != BaseTerm::TT_VARIABLE &&
	 (*i).second->getType() == BaseTerm::TT_VARIABLE)
	{
	  cout << "Orientation applied: (" << (*i).first << "," << (*i).second << ")" << endl;
	  swap((*i).first, (*i).second);
	  ret = true;
	  cout << pairs << endl;
	}
    }

  return ret;
}

bool applyDecomposition(TermPairs & pairs, bool & collision)
{
  bool ret = false;
  
  auto i = pairs.begin();
  while(i != pairs.end())
    {
      if((*i).first->getType() == BaseTerm::TT_FUNCTION &&
	 (*i).second->getType() == BaseTerm::TT_FUNCTION)
	{
	  FunctionTerm * ff = (FunctionTerm *) (*i).first.get();
	  FunctionTerm * fs = (FunctionTerm *) (*i).second.get();

	  if(ff->getSymbol() == fs->getSymbol())
	    {
	      const vector<Term> & ff_ops = ff->getOperands();
	      const vector<Term> & fs_ops = fs->getOperands();

	      for(unsigned k = 0; k < ff_ops.size(); k++)
		{
		  pairs.push_back(make_pair(ff_ops[k], fs_ops[k]));
		}

	      cout << "Decomposition applied: (" << (*i).first << "," << (*i).second << ")" << endl;
	      auto erase_it = i;
	      i++;
	      pairs.erase(erase_it);
	      cout << pairs << endl;
	      ret = true;
	    }
	  else
	    {
	      cout << "Collision detected: " << ff->getSymbol() << " != " << fs->getSymbol() << endl;
	      collision = true;
	      return true;
	    }
	  
	}
      else
	i++;
    }

  collision = false;
  return ret;

}

bool applyApplication(TermPairs & pairs, bool & cycle)
{
  bool ret = false;

  for(auto i = pairs.begin(); i != pairs.end(); i++)
    {
      if((*i).first->getType() == BaseTerm::TT_VARIABLE)
	{
	  VariableTerm * vt = (VariableTerm *) (*i).first.get();
	  if((*i).second->containsVariable(vt->getVariable()))
	    {
	      cycle = true;
	      cout << "Cycle detected: " << (*i).second <<  " contains " << vt->getVariable() << endl;
	      return true;
	    }
	  else
	    {
	      bool changed = false;
	      for(auto j = pairs.begin(); j != pairs.end(); j++)
		{
		  if(j != i)
		    {
		      if((*j).first->containsVariable(vt->getVariable()))
			{
			  (*j).first = (*j).first->
			    substitute(vt->getVariable(),
				       (*i).second);
			  ret = true;
			  changed = true;
			}
		      if((*j).second->containsVariable(vt->getVariable()))
			{
			  (*j).second = (*j).second->
			    substitute(vt->getVariable(),
				       (*i).second);
			  ret = true;
			  changed = true;
			}
		    }
		}
	      if(changed)
		{
		  cout << "Application applied: (" << (*i).first << "," << (*i).second << ")" << endl; 
		  cout << pairs << endl;
		}
	    }
	}
    }
  cycle = false;
  return ret;
}

// -----------------------------------------------------------------------

// Rezolucija ------------------------------------------------------------

ostream & operator << (ostream & ostr, const Clause & c)
{
  ostr << "{ ";
  for(unsigned i = 0; i < c.size(); i++)
    {
      ostr << c[i];
      if(i != c.size() - 1)
        ostr << ", ";
    }
  ostr << " }";
  return ostr;
}

void getClauseVars(const Clause & c, VariableSet & vars)
{
  for(unsigned i = 0; i < c.size(); i++)
    c[i]->getVars(vars);
}

bool clauseContainsVariable(const Clause & c, const Variable & v)
{
  VariableSet vars;
  getClauseVars(c, vars);
  return vars.find(v) != vars.end();
}

Variable getUniqueVariable(const Clause & c1, const Clause & c2)
{
  static unsigned i;

  Variable v;

  do {
    v = string("cv") + to_string(++i);
  } while(clauseContainsVariable(c1, v) || clauseContainsVariable(c2, v));

  return v;
}

bool clauseContainsLiteral(const Clause & c, const Formula & f)
{
  for(unsigned i = 0; i < c.size(); i++)
    if(f->equalTo(c[i]))
      return true;

  return false;
}



bool clauseSubsumed(const Clause & c1, const Clause & c2)
{
  for(unsigned i = 0; i < c1.size(); i++)
    {
      if(!clauseContainsLiteral(c2, c1[i]))
        return false;
    }
  return true;
}


Formula oppositeLiteral(const Formula & l)
{
  if(l->getType() == BaseFormula::T_NOT)
    return ((Not *)l.get())->getOperand();
  if(l->getType() == BaseFormula::T_EQUAL)
    return make_shared<Disequality>(((Equality *)l.get())->getSignature(), ((Equality *)l.get())->getLeftOperand(), ((Equality *)l.get())->getRightOperand());
  if(l->getType() == BaseFormula::T_DISEQUAL)
    return make_shared<Equality>(((Disequality *)l.get())->getSignature(), ((Disequality *)l.get())->getLeftOperand(), ((Disequality *)l.get())->getRightOperand());
  else
    return make_shared<Not>(l);
}

bool clauseTautology(const Clause & c)
{
  for(unsigned i = 0; i < c.size(); i++)
    if(clauseContainsLiteral(c, oppositeLiteral(c[i])))
      return true;

  return false;
}


bool clauseExists(const CNF & cnf, const Clause & c)
{
  for(unsigned i = 0; i < cnf.size(); i++)
    {
      if(clauseSubsumed(cnf[i], c))
        return true;
    }
  return false;
}

void substituteClause(Clause & c, const Substitution & sub)
{
  for(unsigned i = 0; i < c.size(); i++)
    c[i] = c[i]->substitute(sub);
}

void substituteClause(Clause & c, const Variable & v, const Term & t)
{
  Substitution sub;
  sub.push_back(make_pair(v, t));
  substituteClause(c, sub);
}

void removeLiteralFromClause(Clause & c, unsigned k)
{
  c[k] = c.back();
  c.pop_back();
}

void concatClauses(const Clause & c1, const Clause & c2, Clause & cr)
{
  cr = c1;
  for(unsigned i = 0; i < c2.size(); i++)
    cr.push_back(c2[i]);
}

void resolveClauses(const Clause & c1, const Clause & c2,
                    unsigned i, unsigned j,
                    const Substitution & sub, Clause & res)
{
  Clause c1p = c1;
  Clause c2p = c2;
  removeLiteralFromClause(c1p, i);
  removeLiteralFromClause(c2p, j);
  concatClauses(c1p, c2p, res);
  substituteClause(res, sub);
}

void resolveClauses(const Clause & c1, const Clause & c2,
                    unsigned i, const Substitution & sub, Clause & res)
{
  Clause c1p = c1;
  Clause c2p = c2;
  removeLiteralFromClause(c1p, i);
  concatClauses(c1p, c2p, res);
  substituteClause(res, sub);
}

bool tryResolveClauses(CNF & cnf, unsigned k, unsigned l, bool &prematureB)
{
  VariableSet vars;
  bool ret = false;
  Clause c1 = cnf[k], c2 = cnf[l];

  getClauseVars(c2, vars);

  for(auto it = vars.cbegin(); it != vars.cend(); it++)
    {
      if(clauseContainsVariable(c1, *it))
        {
          Variable new_v = getUniqueVariable(c1, c2);
          substituteClause(c2, *it, make_shared<VariableTerm>(new_v));
        }
    }

  for(unsigned i = 0; i < c1.size(); i++)
    for(unsigned j = 0; j < c2.size(); j++)
      {
        if(c1[i]->getType() == BaseFormula::T_ATOM &&
           c2[j]->getType() == BaseFormula::T_NOT)
          {
            Formula c1a =  c1[i];
            Formula c2n = c2[j];
            Formula c2a = ((Not*)c2n.get())->getOperand();

            Substitution sub;
            if(unify(c1a, c2a, sub))
              {
                Clause r;
                resolveClauses(c1, c2, i, j, sub, r);
                if(!clauseExists(cnf, r) && !clauseTautology(r))
                  {
                    cnf.push_back(r);
                    cout << r << endl;
                    cout << "Resolution applied: " << k << ", " << l << endl;
                    cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                    cout << "Substitution: " << sub << endl;
                    cout << "Resolvent: " << r << endl;
                   // cout << cnf << endl;
                    ret = true;
                  }
              }
          }
        else if(c1[i]->getType() == BaseFormula::T_NOT &&
                c2[j]->getType() == BaseFormula::T_ATOM)
          {
            Formula c2a =  c2[j];
            Formula c1n =  c1[i];
            Formula c1a =  ((Not*)c1n.get())->getOperand();

            Substitution sub;
            if(unify(c1a, c2a, sub))
              {
                Clause r;
                resolveClauses(c1, c2, i, j, sub, r);
                if(!clauseExists(cnf, r) && !clauseTautology(r))
                  {
                    cnf.push_back(r);
                    cout << "Resolution applied: " << k << ", " << l << endl;
                    cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                    cout << "Substitution: " << sub << endl;
                    cout << "Resolvent: " << r << endl;
                    //cout << cnf << endl;
                    ret = true;
                  }
              }
          }
        else if((c1[i]->getType() == BaseFormula::T_EQUAL &&
                  c2[j]->getType() == BaseFormula::T_DISEQUAL)
                        ||
                                (c1[i]->getType() == BaseFormula::T_DISEQUAL &&
                                  c2[j]->getType() == BaseFormula::T_EQUAL))
               {
                Substitution sub;
                Equality* tmp = (Equality*)c1[i].get();
                Formula reverse;

                if(c1[i]->getType() == BaseFormula::T_EQUAL)
                    reverse = make_shared<Equality>(tmp->getSignature(), tmp->getRightOperand(), tmp->getLeftOperand());
                else
                    reverse = make_shared<Disequality>(tmp->getSignature(), tmp->getRightOperand(), tmp->getLeftOperand());

                if(unify(c1[i], oppositeLiteral(c2[j]), sub) || unify(reverse, oppositeLiteral(c2[j]), sub)){
                         Clause r;
                         resolveClauses(c1, c2, i, j, sub, r);
                         if(!clauseExists(cnf, r) && !clauseTautology(r))
                           {
                                 cnf.push_back(r);
                                 cout << r << endl;
                                 cout << "Resolution applied: " << k << ", " << l << endl;
                                 cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                                 cout << "Substitution: " << sub << endl;
                                 cout << "Resolvent: " << r << endl;
                                // cout << cnf << endl;
                                 if(r.empty())
                                 {
                                     prematureB = true;
                                     return true;
                                 }
                                 ret = true;
                           }
                }
          }
        else if(c1[i]->getType() == BaseFormula::T_EQUAL)
          {
            Substitution sub;
            Formula c1e =  c1[i];

            Term t =  ((Equality*)c1e.get())->getLeftOperand();
            Term s =  ((Equality*)c1e.get())->getRightOperand();

            if(c2[j]->getType() == BaseFormula::T_NOT)
            {
                Formula tmp =  ((Not*)c2[j].get())->getOperand();
                vector<Term> c2n = ((Atom*)tmp.get())->getOperands();
                for(unsigned ii=0; ii < c2n.size(); ++ii)
                {
                    if(unify(t, c2n[ii], sub))
                    {
                        c2n[ii] = s;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)tmp.get())->getSignature(), ((Atom*)tmp.get())->getSymbol(), c2n);
                        Clause tmpClause = c2;
                        tmpClause[j] = make_shared<Not>(tmpAtom);

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                            //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c2n[ii], sub))
                    {
                        c2n[ii] = t;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)tmp.get())->getSignature(), ((Atom*)tmp.get())->getSymbol(), c2n);
                        Clause tmpClause = c2;
                        tmpClause[j] = make_shared<Not>(tmpAtom);

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                            //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c2[j]->getType() == BaseFormula::T_ATOM)
            {
                vector<Term> c2n = ((Atom*)c2[j].get())->getOperands();
                for(unsigned ii=0; ii < c2n.size(); ++ii)
                {
                    if(unify(t, c2n[ii], sub))
                    {
                        c2n[ii] = s;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)c2[j].get())->getSignature(), ((Atom*)c2[j].get())->getSymbol(), c2n);
                        Clause tmpClause = c2;
                        tmpClause[j] = tmpAtom;

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c2n[ii], sub))
                    {
                        c2n[ii] = t;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)c2[j].get())->getSignature(), ((Atom*)c2[j].get())->getSymbol(), c2n);
                        Clause tmpClause = c2;
                        tmpClause[j] = tmpAtom;

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                            //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c2[j]->getType() == BaseFormula::T_EQUAL)
            {
                vector<Term> c2n = ((Atom*)c2[j].get())->getOperands();
                for(unsigned ii=0; ii < c2n.size(); ++ii)
                {
                    if(unify(t, c2n[ii], sub))
                    {
                        c2n[ii] = s;
                        Formula tmpAtom = make_shared<Equality>(((Atom*)c2[j].get())->getSignature(), c2n[0], c2n[1]);
                        Clause tmpClause = c2;
                        tmpClause[j] = tmpAtom;

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c2n[ii], sub))
                    {
                        c2n[ii] = t;
                        Formula tmpAtom = make_shared<Equality>(((Atom*)c2[j].get())->getSignature(), c2n[0], c2n[1]);
                        Clause tmpClause = c2;
                        tmpClause[j] = tmpAtom;

                        Clause r;
                        resolveClauses(c1, tmpClause, i, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c2[j]->getType() == BaseFormula::T_DISEQUAL)
             {
                 vector<Term> c2n = ((Atom*)c2[j].get())->getOperands();
                 for(unsigned ii=0; ii < c2n.size(); ++ii)
                 {
                     if(unify(t, c2n[ii], sub))
                     {
                         c2n[ii] = s;
                         Formula tmpAtom = make_shared<Disequality>(((Atom*)c2[j].get())->getSignature(), c2n[0], c2n[1]);
                         Clause tmpClause = c2;
                         tmpClause[j] = tmpAtom;

                         Clause r;
                         resolveClauses(c1, tmpClause, i, sub, r);
                         if(!clauseExists(cnf, r) && !clauseTautology(r))
                           {
                             cnf.push_back(r);
                             cout << "Resolution applied: " << k << ", " << l << endl;
                             cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                             cout << "Substitution: " << sub << endl;
                             cout << "Resolvent: " << r << endl;
                           //cout << cnf << endl;
                             ret = true;
                             break;
                           }
                     }
                     else if(unify(s, c2n[ii], sub))
                     {
                         c2n[ii] = t;
                         Formula tmpAtom = make_shared<Disequality>(((Atom*)c2[j].get())->getSignature(), c2n[0], c2n[1]);
                         Clause tmpClause = c2;
                         tmpClause[j] = tmpAtom;

                         Clause r;
                         resolveClauses(c1, tmpClause, i, sub, r);
                         if(!clauseExists(cnf, r) && !clauseTautology(r))
                           {
                             cnf.push_back(r);
                             cout << "Resolution applied: " << k << ", " << l << endl;
                             cout << "Parent clauses: " << c1 << ", " << c2 << endl;
                             cout << "Substitution: " << sub << endl;
                             cout << "Resolvent: " << r << endl;
                           //cout << cnf << endl;
                             ret = true;
                             break;
                           }
                     }
                 }
             }

          }
        else if(c2[j]->getType() == BaseFormula::T_EQUAL)
          {
            Substitution sub;
            Formula c2e =  c2[j];

            Term t =  ((Equality*)c2e.get())->getLeftOperand();
            Term s =  ((Equality*)c2e.get())->getRightOperand();

            if(c1[i]->getType() == BaseFormula::T_NOT)
            {
                Formula tmp =  ((Not*)c1[i].get())->getOperand();
                vector<Term> c1n = ((Atom*)tmp.get())->getOperands();
                for(unsigned ii=0; ii < c1n.size(); ++ii)
                {
                    if(unify(t, c1n[ii], sub))
                    {
                        c1n[ii] = s;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)tmp.get())->getSignature(), ((Atom*)tmp.get())->getSymbol(), c1n);
                        Clause tmpClause = c1;
                        tmpClause[i] = make_shared<Not>(tmpAtom);

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c1n[ii], sub))
                    {
                        c1n[ii] = t;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)tmp.get())->getSignature(), ((Atom*)tmp.get())->getSymbol(), c1n);
                        Clause tmpClause = c1;
                        tmpClause[i] = make_shared<Not>(tmpAtom);

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c1[i]->getType() == BaseFormula::T_ATOM)
            {
                vector<Term> c1n = ((Atom*)c1[i].get())->getOperands();
                for(unsigned ii=0; ii < c1n.size(); ++ii)
                {
                    // da li treba da unifikujemo samo sa prvim na koji naidjemo?
                    if(unify(t, c1n[ii], sub))
                    {
                        c1n[ii] = s;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)c1[i].get())->getSignature(), ((Atom*)c1[i].get())->getSymbol(), c1n);
                        Clause tmpClause = c1;
                        tmpClause[i] = tmpAtom;

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c1n[ii], sub))
                    {
                        c1n[ii] = t;
                        Formula tmpAtom = make_shared<Atom>(((Atom*)c1[i].get())->getSignature(), ((Atom*)c1[i].get())->getSymbol(), c1n);
                        Clause tmpClause = c1;
                        tmpClause[i] = tmpAtom;

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c1[i]->getType() == BaseFormula::T_EQUAL)
            {
                vector<Term> c1n = ((Atom*)c1[i].get())->getOperands();
                for(unsigned ii=0; ii < c1n.size(); ++ii)
                {
                    if(unify(t, c1n[ii], sub))
                    {
                        c1n[ii] = s;
                        Formula tmpAtom = make_shared<Equality>(((Atom*)c1[i].get())->getSignature(), c1n[0], c1n[1]);
                        Clause tmpClause = c1;
                        tmpClause[i] = tmpAtom;

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                    else if(unify(s, c1n[ii], sub))
                    {
                        c1n[ii] = t;
                        Formula tmpAtom = make_shared<Equality>(((Atom*)c1[i].get())->getSignature(), c1n[0], c1n[1]);
                        Clause tmpClause = c1;
                        tmpClause[i] = tmpAtom;

                        Clause r;
                        resolveClauses(c2, tmpClause, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                            cnf.push_back(r);
                            cout << "Resolution applied: " << k << ", " << l << endl;
                            cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                            cout << "Substitution: " << sub << endl;
                            cout << "Resolvent: " << r << endl;
                          //cout << cnf << endl;
                            ret = true;
                            break;
                          }
                    }
                }
            }
            else if(c1[i]->getType() == BaseFormula::T_DISEQUAL)
             {
                 vector<Term> c1n = ((Atom*)c1[i].get())->getOperands();
                 for(unsigned ii=0; ii < c1n.size(); ++ii)
                 {
                     if(unify(t, c1n[ii], sub))
                     {
                         c1n[ii] = s;
                         Formula tmpAtom = make_shared<Disequality>(((Atom*)c1[i].get())->getSignature(), c1n[0], c1n[1]);
                         Clause tmpClause = c1;
                         tmpClause[i] = tmpAtom;

                         Clause r;
                         resolveClauses(c2, tmpClause, j, sub, r);
                         if(!clauseExists(cnf, r) && !clauseTautology(r))
                           {
                             cnf.push_back(r);
                             cout << "Resolution applied: " << k << ", " << l << endl;
                             cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                             cout << "Substitution: " << sub << endl;
                             cout << "Resolvent: " << r << endl;
                           //cout << cnf << endl;
                             ret = true;
                             break;
                           }
                     }
                     else if(unify(s, c1n[ii], sub))
                     {
                         c1n[ii] = t;
                         Formula tmpAtom = make_shared<Disequality>(((Atom*)c1[i].get())->getSignature(), c1n[0], c1n[1]);
                         Clause tmpClause = c1;
                         tmpClause[i] = tmpAtom;

                         Clause r;
                         resolveClauses(c2, tmpClause, j, sub, r);
                         if(!clauseExists(cnf, r) && !clauseTautology(r))
                           {
                             cnf.push_back(r);
                             cout << "Resolution applied: " << k << ", " << l << endl;
                             cout << "Parent clauses: " << c2 << ", " << c1 << endl;
                             cout << "Substitution: " << sub << endl;
                             cout << "Resolvent: " << r << endl;
                           //cout << cnf << endl;
                             ret = true;
                             break;
                           }
                     }
                 }
             }

          }
      }
  return ret;
}

bool resolventFound(CNF & cnf, unsigned & i, unsigned & j)
{
  bool ret = false;
  bool prematureB = false;
  while(j < cnf.size())
    {
      if(tryResolveClauses(cnf, i, j, prematureB))
        {
          ret = true;
          if(prematureB)
              return ret;
        }

      if(i < j - 1)
        i++;
      else
        {
          i = 0; j++;
        }
    }
  return ret;
}

void groupLiterals(const Clause & c, unsigned j, const Substitution & sub, Clause & res)
{
  res = c;
  removeLiteralFromClause(res, j);
  substituteClause(res, sub);
}

bool tryGroupLiterals(CNF & cnf, unsigned k)
{
  const Clause c = cnf[k];
  bool ret = false;
  for(unsigned i = 0; i < c.size(); i++)
    for(unsigned j = i + 1; j < c.size(); j++)
      {
        Substitution sub;
        if(c[i]->getType() == BaseFormula::T_ATOM &&
           c[j]->getType() == BaseFormula::T_ATOM)
          {
            if(unify(c[i], c[j], sub))
              {
                Clause r;
                groupLiterals(c, j, sub, r);
                if(!clauseExists(cnf, r) && !clauseTautology(r))
                  {
                    cnf.push_back(r);
                    cout << "Grouping applied: " << k << endl;
                    cout << "Parent clause: " << c << endl;
                    cout << "Substitution: " << sub << endl;
                    cout << "Grouped clause: " << r << endl;
                   // cout << cnf << endl;
                    ret = true;
                  }
              }
          }
        else if(c[i]->getType() == BaseFormula::T_NOT &&
                c[j]->getType() == BaseFormula::T_NOT)
          {
            Formula cia =  ((Not*)c[i].get())->getOperand();
            Formula cja =  ((Not*)c[j].get())->getOperand();
            if(unify(cia, cja, sub))
              {
                        Clause r;
                        groupLiterals(c, j, sub, r);
                        if(!clauseExists(cnf, r) && !clauseTautology(r))
                          {
                                cnf.push_back(r);
                                cout << "Grouping applied: " << k << endl;
                                cout << "Parent clause: " << c << endl;
                                cout << "Substitution: " << sub << endl;
                                cout << "Grouped clause: " << r << endl;
                               // cout << cnf << endl;
                                ret = true;
                          }
              }
       }
        else if(c[i]->getType() == BaseFormula::T_EQUAL &&
                c[j]->getType() == BaseFormula::T_EQUAL)
          {

            if(unify(c[i], c[j], sub))
              {
                Clause r;
                groupLiterals(c, j, sub, r);
                if(!clauseExists(cnf, r) && !clauseTautology(r))
                  {
                    cnf.push_back(r);
                    cout << "Grouping applied: " << k << endl;
                    cout << "Parent clause: " << c << endl;
                    cout << "Substitution: " << sub << endl;
                    cout << "Grouped clause: " << r << endl;
                  //  cout << cnf << endl;
                    ret = true;
                  }
              }
          }
        else if(c[i]->getType() == BaseFormula::T_DISEQUAL &&
                     c[j]->getType() == BaseFormula::T_DISEQUAL)
         {
            if(unify(c[i], c[j], sub))
              {
                Clause r;
                groupLiterals(c, j, sub, r);
                if(!clauseExists(cnf, r) && !clauseTautology(r))
                  {
                    cnf.push_back(r);
                    cout << "Grouping applied: " << k << endl;
                    cout << "Parent clause: " << c << endl;
                    cout << "Substitution: " << sub << endl;
                    cout << "Grouped clause: " << r << endl;
                  //  cout << cnf << endl;
                    ret = true;
                  }
              }
         }
      }
  return ret;
}

bool groupingFound(CNF & cnf, unsigned & k)
{
  bool ret = false;
  while(k < cnf.size())
    {
      if(tryGroupLiterals(cnf, k))
        {
          ret = true;
        }
      k++;
    }
  return ret;
}


bool resolution(const CNF & cnf)
{
  CNF vcnf = cnf;
  unsigned i = 0, j = 1, k = 0;

  unsigned s = 0;
  while(groupingFound(vcnf, k) || resolventFound(vcnf, i, j))
    {
      for(; s < vcnf.size(); s++)
        if(vcnf[s].size() == 0)
          return false;
    }

  return true;
}

// Ostale funkcije clanice -----------------------------------------------

// Klasa Signature -------------------------------------------------------

void Signature::addFunctionSymbol(const FunctionSymbol & f, unsigned arity)
{
  _functions.insert(make_pair(f, arity));
}

void Signature::addPredicateSymbol(const PredicateSymbol & p, unsigned arity)
{
  _predicates.insert(make_pair(p, arity));
}

bool Signature::checkFunctionSymbol(const FunctionSymbol & f, 
				    unsigned & arity) const
{
  auto it = _functions.find(f);
  
  if(it != _functions.end())
    {
      arity = it->second;
      return true;
    }
  else
    return false;
}

bool Signature::checkPredicateSymbol(const PredicateSymbol & f, 
				     unsigned & arity) const
{
  auto it = _predicates.find(f);
  
  if(it != _predicates.end())
    {
      arity = it->second;
      return true;
    }
  else
    return false;
}

// -----------------------------------------------------------------------

// Klasa BaseTerm ------------------------------------------------------------

bool BaseTerm::containsVariable(const Variable & v) const
{
  VariableSet vars;
  getVars(vars);
  return vars.find(v) != vars.end();
}
// -----------------------------------------------------------------------

// Klasa VariableTerm ----------------------------------------------------

VariableTerm::VariableTerm(const Variable & v)
  :_v(v)
{}

BaseTerm::Type VariableTerm::getType() const
{
  return TT_VARIABLE;
}

const Variable & VariableTerm::getVariable() const
{
  return _v;
}

// -----------------------------------------------------------------------

// Klasa FunctionTerm ----------------------------------------------------

FunctionTerm::FunctionTerm(const Signature & s, const FunctionSymbol & f, 
			   const vector<Term> & ops)
  :_sig(s),
   _f(f),
   _ops(ops)
{
  unsigned arity;
  if(!_sig.checkFunctionSymbol(_f, arity) || arity != _ops.size())
    throw "Syntax error!";
}

FunctionTerm::FunctionTerm(const Signature & s, const FunctionSymbol & f, 
			   vector<Term> && ops)
  :_sig(s),
   _f(f),
   _ops(std::move(ops))
{
  unsigned arity;
  if(!_sig.checkFunctionSymbol(_f, arity) || arity != _ops.size())
    throw "Syntax error!";
}

BaseTerm::Type FunctionTerm::getType() const
{
  return TT_FUNCTION;
}


const Signature & FunctionTerm::getSignature() const
{
  return _sig;
}

const FunctionSymbol & FunctionTerm::getSymbol() const
{
  return _f;
}

const vector<Term> & FunctionTerm::getOperands() const
{
  return _ops;
}

// ----------------------------------------------------------------------

// Klasa BaseFormula --------------------------------------------------------

bool BaseFormula::containsVariable(const Variable & v, bool free) const
{
  VariableSet vars;
  getVars(vars, free);
  return vars.find(v) != vars.end();
}

// ----------------------------------------------------------------------

Variable getUniqueVariable(const Formula & f,  const vector<Term> & ts)
{
  static unsigned i = 0;
  
  Variable v;
  
  while(true) {
    v = string("uv") + to_string(++i);
    
    if(f->containsVariable(v))
      continue;
    
    unsigned j;
    for(j = 0; j < ts.size(); j++)
      {
	if(ts[j]->containsVariable(v))
	  break;
      }

    if(j == ts.size())
      break;
  }
  
  return v;
}

// Klasa AtomicFormula --------------------------------------------------

// Klasa LogicConstant --------------------------------------------------

BaseFormula::Type True::getType() const
{
  return T_TRUE;
}
// ----------------------------------------------------------------------

// Klasa False ----------------------------------------------------------

BaseFormula::Type False::getType() const
{
  return T_FALSE;
}

// ----------------------------------------------------------------------

// Klasa Atom -----------------------------------------------------------

Atom::Atom(const Signature & s, 
	   const PredicateSymbol & p, 
	   const vector<Term> & ops)
  :_sig(s),
   _p(p),
   _ops(ops)
{
  unsigned arity;
  if(!_sig.checkPredicateSymbol(_p, arity) || arity != _ops.size())
    throw "Syntax error!";
}

Atom::Atom(const Signature & s, 
	   const PredicateSymbol & p, 
	   vector<Term> && ops)
  :_sig(s),
   _p(p),
   _ops(std::move(ops))
{
  unsigned arity;
  if(!_sig.checkPredicateSymbol(_p, arity) || arity != _ops.size())
    throw "Syntax error!";
}

const PredicateSymbol & Atom::getSymbol() const
{
  return _p;
}

const Signature & Atom::getSignature() const
{
  return _sig;
}

const vector<Term> & Atom::getOperands() const
{
  return _ops;
}


BaseFormula::Type Atom::getType() const
{
  return T_ATOM;
}

// -----------------------------------------------------------------------

// Klase Equality i Disequality ------------------------------------------
// -----------------------------------------------------------------------

const Term & Equality::getLeftOperand() const
{
  return _ops[0];
}

const Term & Equality::getRightOperand() const
{
  return _ops[1];
}

BaseFormula::Type Equality::getType() const
{
  return T_EQUAL;
}

bool Equality::equalTo(const Formula & f) const
{
  if(f->getType() != T_EQUAL)
    return false;

  const vector<Term> & f_ops = ((Atom *) f.get())->getOperands();

  for(unsigned i = 0; i < _ops.size(); i++)
    if(!_ops[i]->equalTo(f_ops[i]))
      return false;

    return true;
}

Formula Equality::substitute(const Substitution & sub)
{
  vector<Term> sub_ops;

  for(unsigned i = 0; i < _ops.size(); i++)
    sub_ops.push_back(_ops[i]->substitute(sub));

  return make_shared<Equality>(_sig, sub_ops[0], sub_ops[1]);
}

const Term & Disequality::getLeftOperand() const
{
  return _ops[0];
}

const Term & Disequality::getRightOperand() const
{
  return _ops[1];
}

BaseFormula::Type Disequality::getType() const
{
  return T_DISEQUAL;
}

bool Disequality::equalTo(const Formula & f) const
{
  if(f->getType() != T_DISEQUAL)
    return false;

  const vector<Term> & f_ops = ((Atom *) f.get())->getOperands();

  for(unsigned i = 0; i < _ops.size(); i++)
    if(!_ops[i]->equalTo(f_ops[i]))
      return false;

    return true;
}

Formula Disequality::substitute(const Substitution & sub)
{
  vector<Term> sub_ops;

  for(unsigned i = 0; i < _ops.size(); i++)
    sub_ops.push_back(_ops[i]->substitute(sub));

  return make_shared<Disequality>(_sig, sub_ops[0], sub_ops[1]);
}

// Klasa UnaryConjective -------------------------------------------------

UnaryConjective::UnaryConjective(const Formula & op)
  :_op(op)
{}

const Formula & UnaryConjective::getOperand() const
{
  return _op;
}

// -----------------------------------------------------------------------

// Klasa Not -------------------------------------------------------------


BaseFormula::Type Not::getType() const
{
  return T_NOT;
}

// -----------------------------------------------------------------------

// Klasa BinaryConjective ------------------------------------------------

BinaryConjective::BinaryConjective( const Formula & op1,  const Formula & op2)
  :_op1(op1),
   _op2(op2)
{}

const Formula & BinaryConjective::getOperand1() const
{
  return _op1;
  }

const Formula & BinaryConjective::getOperand2() const
{
  return _op2;
}
  

// Klasa And ---------------------------------------------------------------

BaseFormula::Type And::getType() const
{
  return T_AND;
}

// -------------------------------------------------------------------------

// Klasa Or ----------------------------------------------------------------

BaseFormula::Type Or::getType() const
{
  return T_OR;
}

// -------------------------------------------------------------------------

// Klasa Imp ---------------------------------------------------------------

BaseFormula::Type Imp::getType() const
{
  return T_IMP;
}

// -------------------------------------------------------------------------

// Klasa Iff ---------------------------------------------------------------

BaseFormula::Type Iff::getType() const
{
  return T_IFF;
}

// -------------------------------------------------------------------------
  
// Klasa Quantifier --------------------------------------------------------

Quantifier::Quantifier(const Variable & v, const Formula & op)
  :_v(v),
   _op(op)
{}

const Variable & Quantifier::getVariable() const
{
  return _v;
}

const Formula & Quantifier::getOperand() const
{
  return _op;
}

// ------------------------------------------------------------------------

// Klasa Forall -----------------------------------------------------------

BaseFormula::Type Forall::getType() const
{
  return T_FORALL;
}

// ------------------------------------------------------------------------

// Klasa Exists -----------------------------------------------------------

BaseFormula::Type Exists::getType() const
{
  return T_EXISTS;
}

