#include "SmiDiscreteDistribution.hpp"

SmiDiscreteDistribution::~SmiDiscreteDistribution() {
    for (size_t i=0; i<smiDiscrete_.size(); ++i)
        delete smiDiscrete_[i];
}

SmiDiscreteEvent* const SmiDiscreteRV::inverseTransformation( double value )
{
    assert(prob_ <= 1.0 + 0.0001); // Allow a small divergence
    assert(prob_ >= 1.0 - 0.0001 );
    int num_events = events_.size();
    // Do a simple test
    double summed_prob = 0;
    for ( int i = 0; i < num_events; i++){
        summed_prob += getEventProb(i);
        if ( value <= summed_prob )
            return events_[i];
    }
    assert(summed_prob < 1.0);
    return events_[num_events-1]; // Value was larger than the sum, we had
}

void SmiDiscreteRV::addEvent( OsiSolverInterface &osi, double prob )
{
    SmiLinearData d(osi);
    SmiDiscreteEvent *e = new SmiDiscreteEvent(d,prob);
    events_.push_back(e); 
    prob_+=prob;
}

void SmiDiscreteRV::addEvent( CoinPackedMatrix &matrix, CoinPackedVector &dclo, CoinPackedVector &dcup, CoinPackedVector &dobj, CoinPackedVector &drlo, CoinPackedVector &drup, double prob )
{
    SmiLinearData d(matrix,dclo,dcup,dobj,drlo,drup);
    SmiDiscreteEvent *e = new SmiDiscreteEvent(d,prob);
    events_.push_back(e); 
    prob_+=prob;
}

SmiDiscreteRV::~SmiDiscreteRV()
{
    for (size_t i=0; i<events_.size(); ++i)
        delete events_[i];
}
