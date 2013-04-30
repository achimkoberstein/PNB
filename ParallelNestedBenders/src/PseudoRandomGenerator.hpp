#ifndef PseudoRandomGenerator_hpp__
#define PseudoRandomGenerator_hpp__

#include <cstdint>

#include <boost/random.hpp>
#include <boost/math/distributions/students_t.hpp>


namespace parallel_benders {

typedef boost::mt19937 base_generator_type; //Set RNG
typedef boost::variate_generator<base_generator_type&, boost::uniform_01<> > uniform_distr_gen;


//// Stores Uniform01Generator, hides implementation from .hpp
struct Uniform01Generator {
    std::shared_ptr<base_generator_type> rng;
    std::shared_ptr<uniform_distr_gen> uniform;
    unsigned int seed;

    Uniform01Generator();
};



int get_random_number_from_system();

double computeSampleVarianceEstimator(int number_of_samples, double* sampled_values, double average);

double computeSymmetricConfidenceInterval(int number_of_batches, double alpha, double variance);
}

#endif // PseudoRandomGenerator_hpp__