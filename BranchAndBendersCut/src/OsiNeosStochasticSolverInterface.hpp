#ifndef OsiNeosStochasticSolverInterface_h__
#define OsiNeosStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

namespace parallel_benders {
    class ParallelNestedBenders;
    struct Parameter;
}


class OsiStochasticNeosSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticNeosSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);
    //OsiStochasticNeosSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model, BendersParameter& params);
    ~OsiStochasticNeosSolverInterface();

    void initialSolve();

    //double getObjValue();
    //void setParameters(const parallel_benders::Parameter& parameters);
    //int getNumIterations();
    //const parallel_benders::Parameter& getParameters() const;
    //const parallel_benders::Timer& getTimer() const;


private:
    // We need variables here
    std::string xml_string_;

};

OsiStochasticNeosSolverInterface::OsiStochasticNeosSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model )
: OsiStochasticSolverInterface(osi_solver,smi_model),xml_string_("")
{

}

OsiStochasticNeosSolverInterface::~OsiStochasticNeosSolverInterface()
{

}
#endif // OsiNeosStochasticSolverInterface_h__