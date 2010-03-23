// ******************** FlopCpp **********************************************
// File: MP_model.cpp
//****************************************************************************

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
#include <algorithm>
#include <limits>

#include <CoinPackedMatrix.hpp>
#include <OsiSolverInterface.hpp>
#include "MP_model.hpp"
#include "MP_variable.hpp"
#include "MP_constraint.hpp"
#include <CoinTime.hpp>

using namespace flopc;

MP_model& MP_model::default_model = *new MP_model(0);
MP_model* MP_model::current_model = &MP_model::default_model;
MP_model &MP_model::getDefaultModel() { return default_model;}
MP_model *MP_model::getCurrentModel() { return current_model;}

void NormalMessenger::statistics(int bm, int m, int bn, int n, int nz) {
  cout<<"FlopCpp: Number of constraint blocks: " <<bm<<endl;
  cout<<"FlopCpp: Number of individual constraints: " <<m<<endl;
  cout<<"FlopCpp: Number of variable blocks: " <<bn<<endl;
  cout<<"FlopCpp: Number of individual variables: " <<n<<endl;
  cout<<"FlopCpp: Number of non-zeroes (including rhs): " <<nz<<endl;
}

void NormalMessenger::generationTime(double t) {
  cout<<"FlopCpp: Generation time: "<<t<<endl;
}

void VerboseMessenger::constraintDebug(string name, const vector<MP::Coef>& cfs) {
  cout<<"FlopCpp: Constraint "<<name<<endl;
  for (unsigned int j=0; j<cfs.size(); j++) {
    int col=cfs[j].col;
    int row=cfs[j].row;
    double elm=cfs[j].val;
    int stage=cfs[j].stage;
    cout<<row<<"   "<<col<<"  "<<elm<<"  "<<stage<<endl;
  }
}

void VerboseMessenger::objectiveDebug(const vector<MP::Coef>& cfs) {
  cout<<"Objective "<<endl;
  for (unsigned int j=0; j<cfs.size(); j++) {
    int col=cfs[j].col;
    int row=cfs[j].row;
    double elm=cfs[j].val;
    cout<<row<<"   "<<col<<"  "<<elm<<endl;
  }
}

MP_model::MP_model(OsiSolverInterface* s, Messenger* m) : 
  Solver(s),messenger(m), Objective(0), stage(0), 
  m(0), n(0), nz(0),Cst(0),Clg(0),Rnr(0),Elm(0), bl(0), bu(0), c(0),l(0),u(0),colStage(0),rowStage(0),
  mSolverState(((s==0)?(MP_model::DETACHED):(MP_model::SOLVER_ONLY))) {
  MP_model::current_model = this;
}

MP_model::~MP_model() {
  delete messenger;
  if (Solver!=0) {
    delete Solver;
  }
}


MP_model& MP_model::add(MP_constraint& constraint) {
  Constraints.insert(&constraint);
  return *this;
}

void MP_model::add(MP_constraint* constraint) {
  constraint->M = this;
  if (constraint->left.isDefined() && constraint->right.isDefined()) {
    constraint->offset = m;
    m += constraint->size();
  }
}

double MP_model::getInfinity() const {
  if (Solver==0) {
    return std::numeric_limits<double>::infinity();
  } else {
    return Solver->getInfinity();
  }
}

void MP_model::add(MP_variable* v) {
  assert(v != 0);
  v->M = this;
  v->offset = n;
  n += v->size();
}

void MP_model::addRow(const Constraint& constraint) {
  vector<MP::Coef> cfs;
  vector<Constant> v;
  MP::GenerateFunctor f(0,cfs);
  constraint->left->generate(MP_domain::getEmpty(),v,f,1.0);
  constraint->right->generate(MP_domain::getEmpty(),v,f,-1.0);
  CoinPackedVector newRow;
  double rhs = 0.0;
  for (unsigned int j=0; j<cfs.size(); j++) {
    int col=cfs[j].col;
    //int row=cfs[j].row;
    double elm=cfs[j].val;
    //cout<<row<<"   "<<col<<"  "<<elm<<endl;
    if (col>=0) {
      newRow.insert(col,elm);
    } else if (col==-1) {
      rhs = elm;
    }
  }
  // NB no "assembly" of added row yet. Will be done....
 
  double local_bl = -rhs;
  double local_bu = -rhs;

  double inf = Solver->getInfinity();
  switch (constraint->sense) {
      case LE:
        local_bl = - inf;
        break;
      case GE:
        local_bu = inf;
        break;
      case EQ:
        // Nothing to do
        break;          
  }

  Solver->addRow(newRow,local_bl,local_bu);
}

void MP_model::setObjective(const MP_expression& o) { 
  Objective = o; 
}

void MP_model::minimize_max(MP_set &s, const MP_expression  &obj) {
  MP_variable v;
  MP_constraint constraint(s);
  add(constraint);
  constraint(s) = v() >= obj;
  minimize(v());
} 


void MP_model::assemble(vector<MP::Coef>& v, vector<MP::Coef>& av) {
  std::sort(v.begin(),v.end(),MP::CoefLess());
  int c,r,s;
  double val;
  RandomVariable* ptr;
  std::vector<MP::Coef>::const_iterator i = v.begin();
  while (i!=v.end()) {
    c = i->col;
    r = i->row;
    val = i->val;
    s = i->stage;
	ptr = i->ptrVar;
    i++;
	//In case of RHS with a constant term consisting of more than one constant
	//These values were added one after another, otherwise this does not make sense.
    while (i!=v.end() && c==i->col && r==i->row) {
      val += i->val;
      if (i->stage>0) {//If Coefficient was added with a random variable..
        s = i->stage;
		//We do not add random variables, so we better do not have a valid pointer yet
		assert(ptr == 0);
		ptr = i->ptrVar;
		//TODO: Coefficient will get replaced by value of RV. Maybe addd constant term to RV?
      }
      i++;
    }
	//FIXME: Add constant term to RV (can be multiplied or added..)
    av.push_back(MP::Coef(c,r,val,s,ptr));
  }
}

void MP_model::maximize() {
  if (Solver!=0) {
    attach(Solver);
    solve(MP_model::MAXIMIZE);
  } else {
    cout<<"no solver specified"<<endl;
  }
}

void MP_model::maximize(const MP_expression &obj) {
  if (Solver!=0) {
    Objective = obj;
    attach(Solver);
    solve(MP_model::MAXIMIZE);
  } else {
    cout<<"no solver specified"<<endl;
  }
}

void MP_model::minimize() {
  if (Solver!=0) {
    attach(Solver);
    solve(MP_model::MINIMIZE);
  } else {
    cout<<"no solver specified"<<endl;
  }
}

void MP_model::minimize(const MP_expression &obj) {
  if (Solver!=0) {
    Objective = obj;
    attach(Solver);
    solve(MP_model::MINIMIZE);
  } else {
    cout<<"no solver specified"<<endl;
  }
}

void MP_model::attach(OsiSolverInterface *_solver) { //TODO:give pointer to sample-function?!
  if (_solver == 0) {
    if (Solver == 0) {
      mSolverState = MP_model::DETACHED;
      return;
    }  
  } else {  // use pre-attached solver.
    if(Solver && Solver!=_solver) {
      detach();
    }
    Solver=_solver;
  }
  double time = CoinCpuTime();
  m=0;
  n=0;
  vector<MP::Coef> coefs;
  vector<MP::Coef> cfs;

  typedef std::set<MP_variable* >::iterator varIt;
  typedef std::set<MP_constraint* >::iterator conIt;
    
  //Insert decision variables into list of variables
  Objective->insertVariables(Variables);
  //Count number of columns and rows in the add methods. 
  //Insert the remaining variables not in the objective to the list of variables
  for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
    add(*i);
    (*i)->insertVariables(Variables);
  }
  for (varIt j=Variables.begin(); j!=Variables.end(); j++) {
    add(*j);
  }// We have number of rows and number of columns at this point

  // Generate coefficient matrix and right hand side. All Variables and Constraints needs their offset for that to work
  //At first, generate coefficients for every constraint and store them in coefs vector
  for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
    (*i)->coefficients(cfs);
    messenger->constraintDebug((*i)->getName(),cfs);
	//Sort and Store Coefficients of current
    assemble(cfs,coefs);
    cfs.erase(cfs.begin(),cfs.end());
  }
  nz = coefs.size();
  //Create std::vector<RandomVariable> Array from Coefficient Array that stores all RV for a given stage
 vector< vector<RandomVariable*> > randomVars(this->getStage().size()+1);//we store nStages, but without first place, so nStages+1 places are needed
 //Add all random vars that occur in the Matrix and Right-Hand-Side and Variable bounds? 
 for ( int i = 0; i != coefs.size(); i++){
	 if (coefs[i].ptrVar != 0) {
		 //We have a random variable associated with this column/row index
		 randomVars[coefs[i].ptrVar->Stage()].push_back(coefs[i].ptrVar);
		 }
	 }

 messenger->statistics(Constraints.size(),m,Variables.size(),n,nz);

  if (nz>0) {    
    Elm = new double[nz]; 
    Rnr = new int[nz];    
  }
  Cst = new int[n+2];   
  Clg = new int[n+1];   
  if (n>0) {
    l =   new double[n];  
    u =   new double[n];  
    c =  new double[n]; 
	colStage = new int[n];
  }
  if (m>0) {
    bl  = new double[m];  
    bu  = new double[m];  
	rowStage = new int[m];
  }
  const double inf = Solver->getInfinity();
  // Set correct Row Stage for every row by iterating over all elements
  // Row Stage is equal to the highest stage of a belonging variable
  for (int i=0; i<m; i++) {
	  rowStage[i] = 0;
	  }
  for (int i=0; i<nz; i++) {
	  int s = coefs[i].stage; //Stage of Variable that belongs to current coefficient
	  colStage[coefs[i].col] = s; //Set correct colStage. Must be the same for every coefficient for the same variable
	  int row = coefs[i].row; 
	  if ( s > rowStage[row] )  {
		  rowStage[row] = s;
		  }
	  }


  // Set correct Column stage for every column by iterating over all elements
  // Column Stage is equal to the lowest stage of a belonging coefficient
  //for (int j=0; j<n; j++) {
	 // colStage[j] = maxStage;
	 // }
  //for (int i=0; i<nz; i++) {
	 // int col = coefs[i].col;
	 // int s = coefs[i].stage;
	 // if ( s > 0 && s-1 < colStage[col] )  { //TODO: Why is this done? I do not quite get it, see eMail Hultberg-King
		//  colStage[col] = s-1;
		//  }
	 // }

  //   cout<<"RowStages"<<endl;
  //   for (int i=0; i<m; i++) {
  //     cout<<i<<" "<<rowStage[i]<<endl;
  //   }

  //   cout<<"ColStages"<<endl;
  //   for (int j=0; j<n; j++) {
  //     cout<<j<<" "<<colStage[j]<<endl;
  //   }

// Compute Column Length Array to easily compute start array from there
  for (int j=0; j<=n; j++) {
    Clg[j] = 0;
  }
  // Treat right hand side as n'th column
  for (int j=0; j<=n; j++) {
    Clg[j] = 0;
  }
  for (int i=0; i<nz; i++) { //count occurence of a variable (in how many rows is a variable present?
    int col = coefs[i].col;
    if (col == -1)  { //If col is -1 it is the right hand side, and inserted to column
      col = n;
    }
    Clg[col]++;
  }
  //Compute Column Start Array
  Cst[0]=0; //count number of non-zero-elements up to column i: cst[n+1] = anzahl an nicht-null-elementen
  for (int j=0; j<=n; j++) {
    Cst[j+1]=Cst[j]+Clg[j]; 
  }
  for (int i=0; i<=n; i++) { //Reset column length array, to compute Elm and Rnr 
    Clg[i]=0;
  }
  //Compute Element and Index Array (to which row does this coefficient belong) with aid of the column-start and the column lengths
  for (int i=0; i<nz; i++) {
    int col = coefs[i].col;
    if (col==-1) { //Right Hand Side?
      col = n;
    }
    int row = coefs[i].row;
    double elm = coefs[i].val;
    Elm[Cst[col]+Clg[col]] = elm;
    Rnr[Cst[col]+Clg[col]] = row;
    Clg[col]++;
  }

  // Compute row bounds, that are explicitly stored in the elements array as the right-hand-side
  for (int i=0; i<m; i++) {
    bl[i] = 0;
    bu[i] = 0;
  }
  for (int j=Cst[n]; j<Cst[n+1]; j++) {
    bl[Rnr[j]] = -Elm[j];
    bu[Rnr[j]] = -Elm[j];
  }
  //Depending on the sense of the constraint, create less/greater/equal constraint via setting of the boundLower and boundUpper Arrays
  for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
    if ((*i)->left.isDefined() && (*i)->right.isDefined() ) {
      int begin = (*i)->offset;
      int end = (*i)->offset+(*i)->size();
      switch ((*i)->sense) {
          case LE:
            for (int k=begin; k<end; k++) {
              bl[k] = - inf;
            } 
            break;
          case GE:
            for (int k=begin; k<end; k++) {
              bu[k] = inf;
            }     
            break;
          case EQ:
            // Nothing to do
            break;                
      }
    }
  }

  // Compute column bounds. 
  //Initialize all Variables with standard bounds.
  for (int j=0; j<n; j++) {
	  l[j] = 0.0;
	  u[j] = inf;
	  }
  //Set Bounds obtained via MP_variable
  for (varIt i=Variables.begin(); i!=Variables.end(); i++) {
	  for (int k=0; k<(*i)->size(); k++) {
		  l[(*i)->offset+k] = (*i)->lowerLimit.v[k];
		  u[(*i)->offset+k] = (*i)->upperLimit.v[k];
		  }
	  }

  // Generate objective function coefficients
  vector<Constant> v;
  MP::GenerateFunctor f(0,cfs);
  coefs.erase(coefs.begin(),coefs.end());
  Objective->generate(MP_domain::getEmpty(), v, f, 1.0);
  
  messenger->objectiveDebug(cfs);
  assemble(cfs,coefs);
  //add random variables (if any)
  for ( int i = 0; i != coefs.size(); i++){
	  if (coefs[i].ptrVar != 0) {
		  //We have a random variable associated with this column/row index
		  randomVars[coefs[i].ptrVar->Stage()].push_back(coefs[i].ptrVar);
		  }
	  }
  
  for (int j=0; j<n; j++) {
    c[j] = 0.0;
  }
  for (size_t i=0; i<coefs.size(); i++) {
    int col = coefs[i].col;
    double elm = coefs[i].val;
    c[col] = elm;
  } 





  Solver->loadProblem(n, m, Cst, Rnr, Elm, l, u, c, bl, bu);

  for (varIt i=Variables.begin(); i!=Variables.end(); i++) {
	  int begin = (*i)->offset;
	  int end = (*i)->offset+(*i)->size();
	  if ((*i)->type == discrete) {
		  for (int k=begin; k<end; k++) {
			  Solver->setInteger(k);
			  }
		  }
	  }
    mSolverState = MP_model::ATTACHED;

  //Clean Up
  //TODO: What is whith colStage and rowStage?!
  if (nz>0) {    
    delete [] Elm; 
    delete [] Rnr;    
  }
  delete [] Cst;   
  delete [] Clg;   
  if (n>0) {
    delete [] l;  
    delete [] u;  
    delete [] c;
  }
  if (m>0) {
    delete [] bl;  
    delete [] bu;
  }  



  messenger->generationTime(CoinCpuTime()-time);
}

void MP_model::detach() {
  assert(Solver);
  mSolverState = MP_model::DETACHED;
  delete Solver;
  Solver = 0;
  if (n>0) {
	  delete [] colStage;   
	  }
  if (m>0) {
	  delete [] rowStage;  
	  }  
}

int * MP_model::getRowStage() {
	return rowStage;
	}

int * MP_model::getColStage() {
	return colStage;
	}

MP_model::MP_status MP_model::solve(const MP_model::MP_direction &dir) {
  assert(Solver);
  assert(mSolverState != MP_model::DETACHED && 
         mSolverState != MP_model::SOLVER_ONLY);
  Solver->setObjSense(dir);
  bool isMIP = false;
  for (varIt i=Variables.begin(); i!=Variables.end(); i++) {
    if ((*i)->type == discrete) {
      isMIP = true;
      break;
    }
  }
  if (isMIP == true) {
    try {
      Solver->branchAndBound();
    } catch  (CoinError e) {
      cout<<e.message()<<endl;
      cout<<"Solving the LP relaxation instead."<<endl;
      try {
        Solver->initialSolve();
      } catch (CoinError e) {
        cout<<e.message()<<endl;
      }
    }
  } else {
    try {
      Solver->initialSolve();
    }  catch (CoinError e) {
      cout<<e.message()<<endl;
    }
  }
     
  if (Solver->isProvenOptimal() == true) {
    cout<<"FlopCpp: Optimal obj. value = "<<Solver->getObjValue()<<endl;
    cout<<"FlopCpp: Solver(m, n, nz) = "<<Solver->getNumRows()<<"  "<<
      Solver->getNumCols()<<"  "<<Solver->getNumElements()<<endl;
  } else if (Solver->isProvenPrimalInfeasible() == true) {
	cout<<"FlopCpp: Problem is primal infeasible."<<endl;
    return mSolverState=MP_model::PRIMAL_INFEASIBLE;

  } else if (Solver->isProvenDualInfeasible() == true) {
	  cout<<"FlopCpp: Problem is dual infeasible."<<endl;
    return mSolverState=MP_model::DUAL_INFEASIBLE;
    
  } else {
	  cout<<"FlopCpp: Solution process abandoned."<<endl;
    return mSolverState=MP_model::ABANDONED;
    
  }
  return mSolverState=MP_model::OPTIMAL;
}

namespace flopc {
  std::ostream &operator<<(std::ostream &os, 
                           const MP_model::MP_status &condition) {
    switch(condition) {
        case MP_model::OPTIMAL:
          os<<"OPTIMAL";
          break;
        case MP_model::PRIMAL_INFEASIBLE:
          os<<"PRIMAL_INFEASIBLE";
          break;
        case MP_model::DUAL_INFEASIBLE:
          os<<"DUAL_INFEASIBLE";
          break;
        case MP_model::ABANDONED:
          os<<"ABANDONED";
          break;
        default:
          os<<"UNKNOWN";
    };
    return os;
  }

  std::ostream &operator<<(std::ostream &os, 
                           const MP_model::MP_direction &direction) {
    switch(direction) {
        case MP_model::MINIMIZE:
          os<<"MINIMIZE";
          break;
        case MP_model::MAXIMIZE:
          os<<"MAXIMIZE";
          break;
        default:
          os<<"UNKNOWN";
    };
    return os;
  }
}
