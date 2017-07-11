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
  for(unsigned i = 0; !contained || i < subnv.size(); i++)
    contained = subnv[i].second->containsVariable(_v);

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
  for(unsigned i = 0; !contained || i < subnv.size(); i++)
    contained = subnv[i].second->containsVariable(_v);
  
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

ostream & operator << (ostream & ostr, const CNF & cnf)
{
  ostr << "{ ";
  for(unsigned i = 0; i < cnf.size(); i++)
    {
      ostr << cnf[i];
      if(i != cnf.size() - 1)
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

bool tryResolveClauses(CNF & cnf, unsigned k, unsigned l)	
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
		    cout << "Resolution applied: " << k << ", " << l << endl;
		    cout << "Parent clauses: " << c1 << ", " << c2 << endl;
		    cout << "Substitution: " << sub << endl;
		    cout << "Resolvent: " << r << endl;
		    cout << cnf << endl;
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
		    cout << cnf << endl;
		    ret = true;
		  }
	      }
	  }
      }  
  return ret;
}

bool resolventFound(CNF & cnf, unsigned & i, unsigned & j)
{
  bool ret = false;
  while(j < cnf.size())
    {
      if(tryResolveClauses(cnf, i, j))
	{	  
	  ret = true;
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
		    cout << cnf << endl;
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
		    cout << cnf << endl;
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

const Term & Disequality::getLeftOperand() const
{
  return _ops[0];
}

const Term & Disequality::getRightOperand() const
{
  return _ops[1];
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

