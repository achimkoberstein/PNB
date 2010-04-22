// ******************** FlopCpp **********************************************
// File: MP_model.cpp
//****************************************************************************

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
#include <algorithm>
#include <limits>
#include <map>
#include <queue>

#include "boost/shared_ptr.hpp"

#include <CoinTime.hpp>
#include <CoinPackedMatrix.hpp>
#include <CoinPackedVector.hpp>
#include <OsiSolverInterface.hpp>

#include "SmiScnData.hpp"
#include "SmiScenarioTree.hpp"
#include "SmiScnModel.hpp"

#include "MP_model.hpp"
#include "MP_variable.hpp"
#include "MP_constraint.hpp"


using namespace flopc;

const std::string MP_model::stageString("StageSet");
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
        int stage=cfs[j].varStage;
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
Solver(s),messenger(m),stage(0),scenSet(0),Objective(0), 
m(0), n(0), nz(0),Cst(0),Clg(0),Rnr(0),Elm(0), bl(0), bu(0), c(0),l(0),u(0),colStage(0),rowStage(0),colIndirection(0),rowIndirection(0),
mSolverState(((s==0)?(MP_model::DETACHED):(MP_model::SOLVER_ONLY))),smiModel(0) {
    MP_model::current_model = this;
}

MP_model::~MP_model() {
    delete messenger;
    if (Solver!=0) { //If solver was not deleted, call detach and therefore delete the Solver
        this->detach();
    }
    //Delete objective first? I do not know what happens here..
    //if (stage != 0)
    //    delete stage;
    //if (scenSet != 0)
    //    delete scenSet;
    //Reset currentModel Pointer to default model IF it points to this model to prevent dangling pointer
    if (MP_model::current_model == this)
        MP_model::current_model = &MP_model::default_model;

}


MP_model& MP_model::add(MP_constraint& constraint) {
    Constraints.insert(&constraint);
    return *this;
}

void MP_model::add(MP_constraint* constraint) { //Counts total number of rows in this model and sets correct offsets
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

void MP_model::add(MP_variable* v) { //Counts total number of variables in this model
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


void MP_model::assemble(vector<MP::Coef>& v, vector<boost::shared_ptr<MP::Coef> >& av) {
    std::sort(v.begin(),v.end(),&MP::CoefLess);
    int c,r,s,rs;
    double val;
    std::vector<double> scenVector;
    std::vector<MP::Coef>::const_iterator i = v.begin();
    while (i!=v.end()) {
        c = i->col;
        r = i->row;
        val = i->val;
        s = i->varStage;
        rs = i->randomStage;
        scenVector = i->scenVector;
        i++;
        //In case of RHS with a constant term consisting of more than one constant
        //These values were added one after another, otherwise this does not make sense.
        while (i!=v.end() && c==i->col && r==i->row) {
            val += i->val;
            if (i->varStage>0) {//If Coefficient was added with a random variable..
                //This is for right-hand-side addition
                s = i->varStage;
                //We do not add random variables, so we better do not have a valid pointer yet
                //Add values of other scenarios
                assert(scenVector.size() == i->scenVector.size());
                for (int j = 0; j < scenVector.size(); j++){
                    scenVector[j] = scenVector[j] + (i->scenVector)[j];
                }
            }
            i++;
        }
        boost::shared_ptr<MP::Coef> temp(new MP::Coef(c,r,val,s,rs,scenVector));
        av.push_back(temp);
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

void MP_model::attach(OsiSolverInterface *_solver) { //TODO: give pointer to sample-function?!
    if (_solver == 0) {//No solver was given
        if (Solver == 0) {// No solver was set: Attachment is not possible, so we return and set state to detached.
            mSolverState = MP_model::DETACHED;
            return;
        }  
    } else {  // use pre-attached solver.
        if(Solver && Solver!=_solver) {
            detach(); //If old solver is not new solver, go delete the old solver and continue to work with the new one
        }
        Solver=_solver;
    }
    double time = CoinCpuTime();
    m=0;
    n=0;
    vector<boost::shared_ptr<MP::Coef> > coefs;
    vector<MP::Coef> cfs;

    typedef std::set<MP_variable* >::iterator varIt;
    typedef std::set<MP_constraint* >::iterator conIt;

    if (RandomVariables.empty())
        RandomVariables = std::vector<std::set<RandomVariable*> >(stage.size());

    //Insert decision variables into list of variables
    Objective->insertVariables(Variables);
    if (stage.size()){ //if stochastic insert Random Variables
        Objective->insertRandomVariables(RandomVariables);
    }
    //Count number of columns and rows in the add methods. 
    //Insert the remaining variables not in the objective to the list of variables stored in container Variables.
    for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
        add(*i);
        if (stage.size()) // if stochastic insert Random Variables
            (*i)->insertRandomVariables(RandomVariables);
        (*i)->insertVariables(Variables);
    }
    //Count the number of real variables in the model with the add Method. Variables defined but not used in the model are also counted. 
    //TODO: How to deal with variables that are only defined but have no coefficient, special bounds, objective function coefficient??
    for (varIt j=Variables.begin(); j!=Variables.end(); j++) {
        add(*j); 
    }// We have number of rows and number of columns at this point

    // We need to generate all the values we need at this point, if they are not already present.
    // For now: Simple Check: All randomVariables are either ScenarioRandomVars OR (exclusive) Independently distributed RandomVars.
    //TODO: Write that check


    // TODO: Sample Values for all variables (independent ones..)
    // TODO: We can use more intelligent ways of storing the values (sparse arrays..)?
    // TODO: We need to calculate the number of scenarios per stage(for one node). With this information we can calculate the rest...
    // TODO: Think about combining all-against-all (with equal prob or not)
    //sampleRandomVariates(RandomVariables); //Give standard sample size!!

    // Generate coefficient matrix and right hand side. All Variables and Constraints needs their offset for that to work
    //At first, generate coefficients for every constraint and store them in coefs vector: Assigns column and row number
    for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
        (*i)->coefficients(cfs);
        messenger->constraintDebug((*i)->getName(),cfs);
        //Sort and Store Coefficients of current constraint and add them to constraint set coefs.
        assemble(cfs,coefs);
        cfs.erase(cfs.begin(),cfs.end());
    }
    nz = coefs.size();


#pragma region colIndirection;
    if (stage.size()){
        //At this point we have all coefficients for the matrix. We need to sort them by stage, if we are in the stochastic setting and keep the old ordering to be able to set correct values for FlopC++ calls
        std::sort(coefs.begin(),coefs.end(),&MP::CoefLessWithStageShared);
        //Initialize all values with outOfBound to prevent misuse
        colIndirection = new int[n+1]; //store all variables (n) starting from index 0 and rhs as value n => n+1 size needed to store all these values
        for (int i = 0; i < n+1; i++){
            colIndirection[i] = outOfBound;
        }
        //Iterate over set to set correct indices in the coefficients and store the old indices in indirection array. This way we get correct column indices. Rows are not sorted by then
        int j = 0; //index counter
        for (int i = 0; i < nz; i++){
            int oldCol = coefs[i]->col;
            if (oldCol == -1)//Handle RHS
                oldCol = n;
            if ( colIndirection[oldCol] == outOfBound ){ //this Coefficient is not already assigned to a new index, store new index
                if (oldCol == n){//Handle RHS
                    colIndirection[oldCol] = n;
                }
                else {
                    colIndirection[oldCol] = j++;
                }
            }// Assign correct values 
            coefs[i]->col = colIndirection[oldCol];
            //If coef is a random coefficient for a variable bound, we want it removed from the coefs array..
        }
        //Check for empty variables, i.e. variables not assigned in the problem. Maybe we can report that as an error??
        // Empty Variables have no coefficients assigned, but they increase the variable counter
        //TODO: How to handle this?
        if (j != n){
            assert(j <n);
            std::cout << "We have empty variables in this formulation" << std::endl;
            //We need to assign all empty variables to an index which is not used. This is fairly easy: Index j and above are not used..
            for (int count = 0; count < n; count++){
                if (colIndirection[count] == outOfBound) //We have found an empty variable index
                    colIndirection[count] = j++;
            }
            assert(j == n);
        }

        //Now we can access the new column ordering via colIndirection from FlopC++.
        //Now we changed coefficients and we should sort the coefs again, so, that they obey the new ordering
        std::sort(coefs.begin(),coefs.end(),&MP::CoefLessShared);
        //We need to extract all the random Coefficients that belong to the variable bounds and store them in the random coefficients array.
  
    }
#pragma endregion colIndirection;

    messenger->statistics(Constraints.size(),m,Variables.size(),n,nz);

#pragma region coreModelGeneration
    ////
    // Generate Core Model
    ////

    //nz should only count number of elements in the matrix and not the RHS also..
    int realnz = nz;
    Cst = new int[n+2];   
    Clg = new int[n+1];   
    if (n>0) {
        l =   new double[n];  
        u =   new double[n];  
        c =  new double[n]; 
        colStage = new int[n+1]; //Also store colStage of RHS (in column n)
    }
    if (m>0) {
        bl  = new double[m];  
        bu  = new double[m];  
        rowStage = new int[m];
    }
    for (int i=0; i<m; i++) {
        rowStage[i] = 0;
    }
    for (int i=0; i<n+1; i++) {
        colStage[i] = 0;
    }

    const double inf = Solver->getInfinity();
    if (stage.size()){
        // Set correct Row Stage for every row by iterating over all elements
        // Row Stage is equal to the highest stage of a belonging variable
        for (int i=0; i<nz; i++) { //Set row and column stage in corresponding arrays
            int s = coefs[i]->varStage; //Stage of Variable that belongs to current coefficient
            colStage[coefs[i]->col] = s; //Set correct colStage. Must be the same for every coefficient for the same variable
            int row = coefs[i]->row;
            if ( row >= 0 && s > rowStage[row] )  {
                rowStage[row] = s;
            }
        }
        //Now we need to sort the rows by stage
        std::vector<int> rowCounter(stage.size(),0); //Store overall number of rows of a stage
        std::vector<int> rowOffset(m); //store offset of current row in a stage
        std::vector<int> tempRowStage(m);
        rowIndirection = new int[m];
        for (int i = 0; i < m; i++){
            rowOffset[i] = rowCounter[rowStage[i]]++; //In the end, rowCounter gives the number of rows in a stage.
            tempRowStage[i] = rowStage[i];
            rowIndirection[i] = outOfBound;
        }
        for (int i = 0; i < m; i++)//Reset rowStage. Empty rows are now rows with stage 0
            rowStage[i] = 0;
        //Now reset the rowIndex in the coefficient set (and RandomVars..)
        //Iterate over every coefficient. Set old coefficient to new coefficient
        //Something goes wrong here..
        for (int i = 0; i < nz; i++){
            int oldRow = coefs[i]->row;
            if (oldRow < 0) //In the case of Random coefficient for variable bound, continue with next coeff
                continue;
            int curRowStage = tempRowStage[oldRow];
            //Count number of rows before this row
            int rowCount = 0;
            for (int j = 0; j < curRowStage;j++){
                rowCount += rowCounter[j];
            }
            coefs[i]->row = rowCount + rowOffset[oldRow]; //compute correct offset
            rowStage[coefs[i]->row] = curRowStage; //Set rowStage of new indexed row to the stage of the row
            rowIndirection[oldRow] = coefs[i]->row;
            //if (coefs[i]->ptrVar != 0)
            //    coefs[i]->ptrVar->RowIndex(coefs[i]->row);
        }
    }
    //Are there any empty rows? That means rows with no coefficient assigned to them? Is this possible?
    //TODO: Check that
    // Yes, this is indeed possible and is an error
    //for (int i = 0; i < m; i++) {
    //    if( rowIndirection[i] == outOfBound) { // We have an empty row. Can we delete that row without messing everything up?!
    //        //Assign row index of an empty row. Not so easy to do..
    //    }
    //}




    //Now we can access the new column ordering via colIndirection from FlopC++.

    //Create std::vector<RandomVariable> Array from Coefficient Array that stores all RV for a given stage
    vector< vector<boost::shared_ptr<MP::Coef> > > randomCoefs(stage.size()+1);//we store nStages, but without first place, so nStages+1 places are needed
    //Add all random vars that occur in the Matrix and Right-Hand-Side and Variable bounds? 
    //TODO: Add Coefficients instead of random vars..
    if (stage.size()){
        //In case of random variables in the model we might have random bounds..
        //Generate random bounds for all variables..
        // As these bounds were generated in the variables section, the indirection layer is incorrect, this means we have to update that
        for (varIt j=Variables.begin(); j!=Variables.end(); j++) {
            (*j)->bounds(randomCoefs);
        }
        for (int i = 0; i < randomCoefs.size();i++) {
            for (int j = 0; j < randomCoefs[i].size();j++) {
                randomCoefs[i][j]->col = colIndirection[randomCoefs[i][j]->col];
            }
        }

        for ( int i = 0; i != coefs.size(); i++){
            if ( !coefs[i]->scenVector.empty() ) { //We have scenario values
                //We have a random parameter with values
                randomCoefs[coefs[i]->randomStage].push_back(coefs[i]);
            }
        }

    }

    // Compute Column Length Array to easily compute start array from there
    for (int j=0; j<=n; j++) {
        Clg[j] = 0;
    }
    // Treat right hand side as n'th column
    for (int j=0; j<=n; j++) {
        Clg[j] = 0;
    }
    for (int i=0; i<nz; i++) { //count occurence of a variable (in how many rows is a variable present?
        int col = coefs[i]->col;
        if (col == n)  { 
            realnz--;
        }
        if (col == -1){ //If col is -1 it is the right hand side. We set col to n in the following and reduce number of realnz in matrix by 1
            realnz--;
            col = n;
            coefs[i]->col = n; //Also set coefficient to n to do it permanently
        }
        if (coefs[i]->row >= 0) //We have a correct variable here that occurs in a row 
            Clg[col]++;
        //In the else case we do not want to count the coefficient here, but we want to reduce number of realnz: We have a random variable bound
        else 
            realnz--;
    }
    //Compute Column Start Array
    Cst[0]=0; //count number of non-zero-elements up to column i: cst[n+1] = anzahl an nicht-null-elementen
    for (int j=0; j<=n; j++) {
        Cst[j+1]=Cst[j]+Clg[j]; 
    }
    for (int i=0; i<=n; i++) { //Reset column length array, to compute Elm and Rnr 
        Clg[i]=0;
    }
    if (realnz>0) {    
        Elm = new double[realnz]; 
        Rnr = new int[realnz];    
    }
        // Compute row bounds, that are explicitly stored in the elements array as the right-hand-side
    for (int i=0; i<m; i++) {
        bl[i] = 0;
        bu[i] = 0;
    }
    //Compute Element and Index Array (to which row does this coefficient belong) with aid of the column-start and the column lengths
    for (int i=0; i<nz; i++) {
        int col = coefs[i]->col;
        int row = coefs[i]->row;
        double elm = coefs[i]->val;
        if (col == n){ //RHS:
            bl[row] = -elm;
            bu[row] = -elm;
        }
        else {
            Elm[Cst[col]+Clg[col]] = elm;
            Rnr[Cst[col]+Clg[col]] = row;
        }
        Clg[col]++;
    }


    //for (int j=Cst[n]; j<Cst[n+1]; j++) {
    //    bl[Rnr[j]] = -Elm[j];
    //    bu[Rnr[j]] = -Elm[j];
    //}
    //Depending on the sense of the constraint, create less/greater/equal constraint via setting of the boundLower and boundUpper Arrays
    //TODO: Empty row handling done here..
    for (conIt i=Constraints.begin(); i!=Constraints.end(); i++) {
        if ((*i)->left.isDefined() && (*i)->right.isDefined() ) {
            int begin = (*i)->offset;
            int end = (*i)->offset+(*i)->size();
            switch ((*i)->sense) {
                  case LE:
                      for (int k=begin; k<end; k++) {
                          if (stage.size()){
                              if (rowIndirection[k] != outOfBound) //Else empty row
                                  bl[rowIndirection[k]] = - inf;
                          }
                          else
                              bl[k] = -inf;
                      } 
                      break;
                  case GE:
                      for (int k=begin; k<end; k++) {
                          if (stage.size()){
                              if (rowIndirection[k] != outOfBound) //Else empty row
                                  bu[rowIndirection[k]] = inf;
                          }
                          else
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
    //Set Bounds obtained via MP_variable via direct access
    // Furthermore: Variables obtain solver infinity bounds
    const double numericInfinity = std::numeric_limits<double>::infinity();
    for (varIt i=Variables.begin(); i!=Variables.end(); i++) {
        for (int k=0; k<(*i)->size(); k++) {
            const DataRef* dPtr;
            if (stage.size()){ //In case of stochastic, we have to obey colIndirection
                dPtr = dynamic_cast<const DataRef*> ((*i)->lowerLimit.operator->());// v[k];
                if (dPtr == 0)
                    l[colIndirection[(*i)->offset+k]] = (*i)->lowerLimit->evaluate();
                else
                    l[colIndirection[(*i)->offset+k]] = dPtr->D->v[k];
                dPtr = dynamic_cast<const DataRef*> ((*i)->upperLimit.operator->());
                if (dPtr == 0)
                    u[colIndirection[(*i)->offset+k]] = (*i)->upperLimit->evaluate();
                else
                    u[colIndirection[(*i)->offset+k]] = dPtr->D->v[k];

                if (l[colIndirection[(*i)->offset+k]] == -numericInfinity)
                    l[colIndirection[(*i)->offset+k]] = -inf;
                if (u[colIndirection[(*i)->offset+k]] == numericInfinity)
                    u[colIndirection[(*i)->offset+k]] = inf;

             }
            else {
               dPtr = dynamic_cast<const DataRef*> ((*i)->lowerLimit.operator->());// v[k];
                if (dPtr == 0)
                    l[(*i)->offset+k] = (*i)->lowerLimit->evaluate();
                else
                    l[(*i)->offset+k] = dPtr->D->v[k];
                dPtr = dynamic_cast<const DataRef*> ((*i)->upperLimit.operator->());
                if (dPtr == 0)
                    u[(*i)->offset+k] = (*i)->upperLimit->evaluate();
                else
                    u[(*i)->offset+k] = dPtr->D->v[k];

                if (l[(*i)->offset+k] == -numericInfinity)
                    l[(*i)->offset+k] = -inf;
                if (u[(*i)->offset+k] == numericInfinity)
                    u[(*i)->offset+k] = inf;
            }
        }
    }

    // Generate objective function coefficients
    vector<Constant> v;
    MP::GenerateFunctor f(0,cfs);
    coefs.erase(coefs.begin(),coefs.end());
    Objective->generate(MP_domain::getEmpty(), v, f, 1.0);
    messenger->objectiveDebug(cfs);
    assemble(cfs,coefs);
    //Sort coefficients by stage..
    std::sort(coefs.begin(),coefs.end(),&MP::CoefLessWithStageShared);
    //add random coefficients (if any) and we are in the stochastic setting
    //TODO: Coefficients that are in the objective function but not in the constraints can mess things up..?
    if (stage.size()){
        for ( int i = 0; i != coefs.size(); i++){
            //Set correct rowIndex and colIndex via colIndirection
            coefs[i]->col = colIndirection[coefs[i]->col];
            if (colStage[coefs[i]->col] != coefs[i]->varStage) { // Variable is not present in any expression..
                assert(colStage[coefs[i]->col] == 0);
                colStage[coefs[i]->col] = coefs[i]->varStage; //
            }
            if (coefs[i]->row == -1) //Objective function 
                coefs[i]->row = m;
        }
    }
    else {
        for (int i = 0; i != coefs.size(); i++)
            coefs[i]->row = m;
    }

    for (int j=0; j<n; j++) {
        c[j] = 0.0;
    }
    //Set objective coefficients
    for (size_t i=0; i<coefs.size(); i++) {
        int col = coefs[i]->col;
        if ( coefs[i]->row == m){
            double elm = coefs[i]->val;
            c[col] = elm;
        }
    } 
    //We need to set column information by stage for all columns where it is not yet set

    // Set integer information..
    std::vector<int> integerIndices;
    for (varIt i=Variables.begin(); i!=Variables.end(); i++) {
        int begin = (*i)->offset;
        int end = (*i)->offset+(*i)->size();
        if ((*i)->type == discrete) {
            for (int k=begin; k<end; k++) {
                if (stage.size()){
                    integerIndices.push_back(colIndirection[k]);
                }
                else{
                    integerIndices.push_back(k);
                }
            }
        }
    }
    std::sort(integerIndices.begin(),integerIndices.end()); //Sort indices in ascending order

    ////
    // Core Model generated..
    // All rows and cols are sorted by stage.
    // RHS = n, Objective = m (indices)
    // Old indices are stored in colIndirection and rowIndirection
    ////
#pragma endregion coreModelGeneration


    //Check for stochasticity: This is the case if we have a stage set with size greater than 0.
    if (stage.size() > 0){
        ////
        // Feed Core Model to SmiScnModel
        ////
        // Generate Core ModelMatrix: Need Only the matrix, no rhs, so we need realnz (Elm,Rnr and Cst are too big..)	
        CoinPackedMatrix matrix(true,m,n,realnz,Elm,Rnr,Cst,NULL); //Matrix without right-hand-side..
        matrix.reverseOrdering(); //Smi needs matrix in row ordered form
        printMatrix(&matrix,n,m,l,u,c,bl,bu,colStage,rowStage);
        //matrix.removeGaps(); //What is this for?
        Solver->loadProblem(matrix, l, u, c, bl, bu); //Generate Core Data, in Solver
        SmiCoreData *osiCore = 0;
        if (!integerIndices.empty()){
            Solver->setInteger(&integerIndices[0],integerIndices.size());
            osiCore = new SmiCoreData(Solver,stage.size(),colStage,rowStage,&integerIndices[0]); //Create CoreData Node. Needs to be deleted in case of stochastic solver.
        }
        else {
            osiCore = new SmiCoreData(Solver,stage.size(),colStage,rowStage); //Create CoreData Node. Needs to be deleted in case of stochastic solver.
        }

        // Get SmiScnModel that holds a generated ScenarioTree.
        //TODO: Think about that. Maybe the attach function should just set the SmiCoreData object and return with that
        smiModel = generateScenarioTree(randomCoefs,osiCore);
        if (smiModel == 0)
            //We were not sucessful in creating a scenario tree. Try normal operation. TODO: Think about reversed matrix..
            Solver->loadProblem(n,m,Cst,Rnr,Elm,l,u,c,bl,bu);
        else
            smiModel->Core(osiCore); //Let smiModel handle disposal of SmiCoreData
    }
    else {
        Solver->loadProblem(n, m, Cst, Rnr, Elm, l, u, c, bl, bu);
        if (!integerIndices.empty())
            Solver->setInteger(&integerIndices[0],integerIndices.size());
        printMatrix( const_cast<CoinPackedMatrix*>(Solver->getMatrixByRow()),n,m,l,u,c,bl,bu,colStage,rowStage);
    }

    mSolverState = MP_model::ATTACHED;

#pragma region cleanUp;
    //Clean Up
    //ColIndirection and rowIndirection arrays may be needed later on
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
#pragma endregion cleanUp;



    messenger->generationTime(CoinCpuTime()-time);
}

void MP_model::sampleRandomVariates(std::vector<std::set<RandomVariable*> > rv, int sampleSize){
    typedef std::set<RandomVariable*>::iterator rvIterator;

    std::vector<int> randVarsPerStage(stage.size(),0);
    //For every stage: We have stage independence
    for( int i = 1; i < stage.size(); i++){
        // Sample values for every variable independently, all sampled values are equally probable
        for (rvIterator it = rv[i].begin(); it != rv[i].end();++it){
            (*it)->sample(sampleSize);
            randVarsPerStage[i]++;
        }
        // Combine all-against-all 
        int overallSamples = randVarsPerStage[i]*sampleSize;
        double prob = 1/overallSamples; //Prob of a sample
        for (int j = 0; j < sampleSize; j++) {
            
            for (rvIterator it = rv[i].begin(); it!= rv[i].end();++it){
                double curVal = (*it)->getSampledValue(j);
                for (rvIterator it2 = rv[i].begin(); it2 != rv[i].end();++it){ //Start with next variable
                    if (it2 == it)
                        continue; //go to end of loop if we are at the same variable



                }
            }
        }//end for j=0 to sampleSize
    }//end for i=1 to stage

}

void MP_model::detach() {
    assert(Solver);
    mSolverState = MP_model::DETACHED;
    delete Solver;
    Solver = 0;
    //Delete stochastic information. We have this, if we have a time structure AND we have attached a solver.
    if (stage.size() > 0 && smiModel != 0){ 
        smiModel->releaseSolver(); //TODO: This can be deleted, if stochasticSolverInterface is ready.
        delete smiModel;
        if (n > 0){
            delete [] colStage;
            delete [] colIndirection;
        }
        if (m > 0){
            delete [] rowStage;
            delete [] rowIndirection;
        }
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
    if (stage.size() > 0 && smiModel != 0) { // We have a time structure AND a smiModel ready. If only timeStructure, there were no RandomVariables specified.
        //OsiClpSolverInterface osiClp;
        //smiModel->setOsiSolverHandle(osiClp);
        smiModel->setOsiSolverHandle(Solver);
        //OsiSolverInterface* osiStoch = smiModel->loadOsiSolverData();
        Solver = smiModel->loadOsiSolverData();
        Solver->setObjSense(dir); //Important: After loading solver data, objSense is not specified..
        try {
            Solver->initialSolve();
        }  catch (CoinError e) {
            cout<<e.message()<<endl;
        }
    }
    else {
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

    //Make this a member function, so we can look at the arrays we need (row sense for example)
    SmiScnModel* MP_model::generateScenarioTree(const std::vector< std::vector<boost::shared_ptr<MP::Coef> > >& randomVariableVector, SmiCoreData* smiCore){
        typedef std::vector<boost::shared_ptr<MP::Coef>>::const_iterator randVarIterator;
        //Get basic important values
        int nStages = smiCore->getNumStages();
        int nScen = this->ScenSet().size(); //If nScen is not set, curStage.e. equal to 0, we need to count number of Scenarios 
        std::vector<int> randVarsPerStage(nStages);
        bool count = false;
        if (nScen==0) // We have no specified scenario set, so we rely on the default sample size and count the scenario size afterwards.
            count = true;
        for (int curStage = 1; curStage < nStages; curStage++){ //Count number of random coefficients for each stage..
            for (randVarIterator it = randomVariableVector[curStage].begin(); it != randomVariableVector[curStage].end();++it){
                randVarsPerStage[curStage]++;
            }
        }
        if (nScen == 0 || randVarsPerStage[1] == 0)//If we have nothing to do, we have nothing to. 
            return 0;
        //At this point we have a fan of values, represented by values inside each RV
        //Now we want to make a tree out of it
        int m = smiCore->getNumRows();
        int n = smiCore->getNumCols(); 
        // Initialize data structures needed later on
        std::vector<CoinPackedMatrix*> matrixVec(nScen); // Column Ordered Matrices
        std::vector<CoinPackedMatrix*> objVec(nScen); //Empty Column ordered matrices for objective function and row bounds.. Necessary as CoinPackedVector does not allow adding of elements
        std::vector<CoinPackedMatrix*> rloVec(nScen);
        std::vector<CoinPackedMatrix*> rupVec(nScen);
        std::vector<CoinPackedMatrix*> bloVec(nScen);
        std::vector<CoinPackedMatrix*> bupVec(nScen);
        for (int curScen = 0; curScen < nScen;curScen++){ //Think about that in terms of lazy evaluation..?!
            //Initialize all matrices
            matrixVec[curScen] = new CoinPackedMatrix();
            matrixVec[curScen]->reverseOrdering();
            matrixVec[curScen]->setDimensions(m,n);
            //Initialize all vectors
            objVec[curScen] = new CoinPackedMatrix();
            objVec[curScen]->setDimensions(n,1);
            rloVec[curScen] = new CoinPackedMatrix();
            rloVec[curScen]->setDimensions(m,1);
            rupVec[curScen] = new CoinPackedMatrix();
            rupVec[curScen]->setDimensions(m,1);
            bloVec[curScen] = new CoinPackedMatrix();
            bloVec[curScen]->setDimensions(n,1);
            bupVec[curScen] = new CoinPackedMatrix();
            bupVec[curScen]->setDimensions(n,1);

        }

        //CoinPackedVector clo;
        //CoinPackedVector cup;
        std::vector<int> tempAncScenVec(nScen,outOfBound); //Initalized Vector with outOfBound (not valid)
        std::vector<int> branchStageVec(nScen,outOfBound); //Initialized Vector with 0
        //set correct values for first scenario:
        tempAncScenVec[0] = 0;
        branchStageVec[0] = 1;

        std::vector<int> scenarioIndirection(nScen,outOfBound);
        std::vector<int> realOrderingFromVirtualScenario(nScen,outOfBound);
        std::vector<double> probabilities(nScen,1.0/nScen);

        typedef std::set<int>::iterator scenIt;
        std::set<int> scenarioHeap;

        for (int i = 0; i < nScen; i++)
            scenarioHeap.insert(i);

        std::vector<std::deque<int> > subScenarioQueueVector(nStages,std::deque<int> ()); //For each stage an own queue
        subScenarioQueueVector[0].push_front(0);

        //We do not like recursion, therefore we need to store all the elements in a list of vectors and stuff
        std::vector< std::vector< std::set<int> > > subScenariosPerStageAndScenario(nStages,std::vector<std::set<int> > (nScen));
        for (int j = 0; j < nScen; j++){ // Assign scenarioHeap to all the first stage sets. TODO: Improve memory consumption..
            subScenariosPerStageAndScenario[0][j] = scenarioHeap;
        }


        int curStage = 0; //We start at stage 0
        int curScen = 0; //We start with scenario 0
        int scenCounter = 0; //We only need this counter in the very last round.

        //We do the following: Add all subnodes to this node to the subqueue list with their list of scenarios. Then go to the subqueue list and do the same.
        while( !subScenarioQueueVector[curStage].empty()){
            //Look at top node and remove it from the queue
            curScen = subScenarioQueueVector[curStage].front();
            subScenarioQueueVector[curStage].pop_front();
            // Loop over all subset scenarios of the current scenario to form new nodes
            while (!subScenariosPerStageAndScenario[curStage][curScen].empty()){
                int parentScen = *subScenariosPerStageAndScenario[curStage][curScen].begin(); //the parentScen will be the "parent" or head of the new node.
                //If the node is already a parent of a node in a lower stage, we keep the old values
                if (branchStageVec[parentScen] == outOfBound)
                    branchStageVec[parentScen] = curStage+1;
                if (tempAncScenVec[parentScen] == outOfBound)
                    tempAncScenVec[parentScen] = curScen;

                if (curStage < nStages-1){//Add to queue for the next stage, if we are not in the last stage
                    subScenarioQueueVector[curStage+1].push_back(parentScen);
                }
                else{ //In the last stage: increase scenario counter and set scenarioIndirection and delete the parent scenario from subScenarios
                    scenarioIndirection[parentScen] = scenCounter++;
                    realOrderingFromVirtualScenario[scenCounter -1] = parentScen;
                    subScenariosPerStageAndScenario[curStage][curScen].erase(subScenariosPerStageAndScenario[curStage][curScen].begin()); //Remove from subsetSet to only contain strict subset in the last stage: This ensures correct probability calculation
                }

#pragma region setCoefficients
                //Do the real stuff: Add values for the current scenario.
                //Add all random coefficients to corresponding places
                for (int j = 0; j < randVarsPerStage[curStage];j++){  //Values differ 
                    boost::shared_ptr<MP::Coef> it = randomVariableVector[curStage][j];
                    double coefficient = it->scenVector[parentScen];
                    int colIndex = it->col;
                    int rowIndex = it->row;
                    if (colIndex < n) {//Matrix-Element or variable bounds
                        assert(rowIndex <= m); //matrix element or objective function or variable bound
                        if (rowIndex < 0){ //variable bound
                            assert(colIndex >= 0); //col index needs to be inside bounds
                            if (rowIndex == lowerBound)
                                bloVec[parentScen]->modifyCoefficient(colIndex,0,coefficient,true);
                            else if (rowIndex == upperBound)
                                bupVec[parentScen]->modifyCoefficient(colIndex,0,coefficient,true);
                            else
                                throw invalid_argument_exception();
                        }
                        else if (rowIndex < m){ //matrix element
                            assert(colIndex >= 0); //col index needs to be inside bounds
                            matrixVec[parentScen]->modifyCoefficient(rowIndex,colIndex,coefficient,true); // Keeps added zeroes. If set to false, zeroes get changed to ones
                        }
                        else{ //Objective Function
                            assert(rowIndex == m);
                            objVec[parentScen]->modifyCoefficient(colIndex,0,coefficient);
                        }
                    }
                    else{ //RHS : This means coefficient is multiplied with -1. TODO: Change this behaviour in FlopC++ so that also right hand side has original values.
                        assert(colIndex == n);
                        //We need the row sense to insert in the right vector..
                        switch (this->Solver->getRowSense()[rowIndex])
                        { 
                        case 'E' : rloVec[parentScen]->modifyCoefficient(rowIndex,0,-coefficient,true); rupVec[parentScen]->modifyCoefficient(rowIndex,0,-coefficient,true); break;
                        case 'G' : rloVec[parentScen]->modifyCoefficient(rowIndex,0,-coefficient,true); break;
                        case 'L' : rupVec[parentScen]->modifyCoefficient(rowIndex,0,-coefficient,true); break; 
                        case 'N' : break;
                        default : throw not_implemented_error(); 
                        }//end switch
                    }//end else
                }//end for: randVars
#pragma endregion setCoefficients

                // Remove elements from subsetHeap that are equal to this one and add them to subset for next stage.
                for (scenIt innerIt = subScenariosPerStageAndScenario[curStage][curScen].begin(); innerIt != subScenariosPerStageAndScenario[curStage][curScen].end();){
                    if (curStage == nStages-1){
                        //We are in the last stage. All scenarios that still have the same values end up in the same scenario, but that increases the probability
                        //Remove the scenario from current stage and set realOdering to outOfBound
                        tempAncScenVec[*innerIt] = tempAncScenVec[parentScen];
                        scenarioIndirection[*innerIt] = outOfBound;
                        realOrderingFromVirtualScenario[scenCounter] = outOfBound;
                        probabilities[parentScen] += probabilities[*innerIt]; //add one scenario prob for each added scenario.
                        probabilities[*innerIt] = 0;
                        subScenariosPerStageAndScenario[curStage][curScen].erase(innerIt++);
                    }
                    // Define the new scenario subset node for the next stage
                    else if (!differFromPrevious(randomVariableVector,curStage+1,parentScen,*innerIt)){// We have a scenario subset node        
                        //Add to scenario subset heap of next stage node and scenario
                        subScenariosPerStageAndScenario[curStage+1][parentScen].insert(*innerIt); //debug this statement
                        //Erase from subsetHeap, so that this subset scenario does not form a new node in a subsequent run of the while loop
                        subScenariosPerStageAndScenario[curStage][curScen].erase(innerIt++);
                    }
                    else{ // Scenario is in another node, skip it for now. Will be looked at during the inner while loop
                        ++innerIt;
                    }
                }//end for
            }//end while: subScenariosPerStageAndScenario

            // All nodes for the current stages are done, turn to the next stage. If we are in the last stage, do nothing do leave outer while loop.
            if (subScenarioQueueVector[curStage].empty() && curStage < nStages-1){
                curStage++;
            }
        }//end while: deque

        //Generate scenarios in SMI
        // SMI expects a "correct" scenario tree ordering, so we reorder the scenarios first.
        //We need to use scenarioIndirection to get correct index for scenario i (natural ordering as expected from smi).
        std::vector<int> ancScenVec(nScen);
        for (int i = 0; i < scenCounter; i++){
            ancScenVec[i] = realOrderingFromVirtualScenario[tempAncScenVec[scenarioIndirection[i]]]; //Get program index for natural scenario i. Need to transfer program index to natural scenario index. 
        }

        SmiScnModel *smiModel = new SmiScnModel(); //Create new SmiScnModel
        int numOfIrrelevantScen = std::count(branchStageVec.begin(),branchStageVec.end(),outOfBound);
        for (int i = 0; i < scenCounter; i++){
            int curScen = scenarioIndirection[i];
            //If ancScen of an arbitrary scenario is not set the data is such that the number of Scenarios reduces by this scenario, as it does not differ from another one.
            if (ancScenVec[curScen]!=outOfBound){ //If ancScen is set, we can generate a scenario, otherwise, we won't.
                //We need to convert the CoinPackedMatrix to CoinPackedVectors..
                CoinPackedVector curBloVec(bloVec[curScen]->getVector(0));
                CoinPackedVector curBupVec(bupVec[curScen]->getVector(0));
                CoinPackedVector curObjVec(objVec[curScen]->getVector(0));
                CoinPackedVector curRloVec(rloVec[curScen]->getVector(0));
                CoinPackedVector curRupVec(rupVec[curScen]->getVector(0));

                smiModel->generateScenario(smiCore,matrixVec[curScen],&curBloVec,&curBupVec,&curObjVec,&curRloVec,&curRupVec,branchStageVec[curScen],ancScenVec[i],probabilities[curScen]);
            }
        }
        //TODO: Think about debugging routines or debugging logger..
        std::cout << "Generated " << scenCounter << " scenarios." << std::endl;

        //Delete temporary variables
        for (int curScen = 0; curScen < nScen; curScen++){
            delete matrixVec[curScen];
            delete objVec[curScen];
            delete rloVec[curScen];
            delete rupVec[curScen];
        }
        return smiModel;
    }




    bool MP_model::differFromPrevious(const std::vector<std::vector<boost::shared_ptr<MP::Coef> > >& randomVariableVector,int curStage,int curScen,int nextScen){
        if (curScen == nextScen)
            return false;
        bool identical = true;
        for (int i = 0; i < randomVariableVector[curStage].size();i++){
            if (!compareDouble(randomVariableVector[curStage][i]->scenVector[curScen],randomVariableVector[curStage][i]->scenVector[nextScen]) ){
                identical = false;
                break; //break loop and return
            }
        }
        return !identical;
    }

    //SmiScnModel* generateScenario(MP_model* flopModel, const std::vector< std::vector<flopc::RandomVariable*> >& randomVariableVector, SmiCoreData* smiCore, int scen, int branchStage){
    //    // Sample values for all variables at this stage
    //    return 0;
    //}

    void printMatrix(CoinPackedMatrix* matrix_,int n, int m, double* clo, double* cup, double* obj, double* rlo, double* rup, int* colStage = 0, int* rowStage = 0){
        printf("Print Problem\n");
        printf("Objective: ");
        for (int i = 0; i < n;i++){
            cout << " " << obj[i] << "*x" << i; 
        }
        cout << endl;
        for (int mi = 0; mi < matrix_->getMajorDim(); mi++){
            printf("%g<= ",rlo[mi]);
            for (int mj = 0; mj < matrix_->getMinorDim(); mj++){
                printf("%g ",matrix_->getCoefficient(mi,mj));
            }
            printf("<= %g",rup[mi]);
            if (rowStage != 0)
                cout << " # stage " << rowStage[mi];
            cout << endl;
        }
        cout << "\n";
        //Print variables with bounds
        for (int i = 0; i < n; i++){
            cout << clo[i] << " <= x" << i << " <= " << cup[i];
            if (colStage != 0)
                cout << " in stage " << colStage[i];
            cout << endl;
        }
        printf("\nMatrix printed \n");
    }







    void Messenger::constraintDebug( string name, const vector<MP::Coef>& cfs )
    {

    }

    void Messenger::objectiveDebug( const vector<MP::Coef>& cfs )
    {

    }

    void Messenger::statistics( int bm, int m, int bn, int n, int nz )
    {

    }

    void Messenger::generationTime( double t )
    {

    }

    void Messenger::logMessage( int level, const char * const msg )
    {

    }
}
