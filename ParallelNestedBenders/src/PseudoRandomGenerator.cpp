#include "PseudoRandomGenerator.hpp"

//Windows. Construct necessary as VisualC++ include stdlib.h before I can set _CRT_RAND_S
#ifdef _MSC_VER 
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif 
#include <stdlib.h>
#include <ctime>          // std::time
#else
#include <stdio.h>
#include <fcntl.h>
#endif
//Linux: Call /dev/urandom to generate a seed

parallel_benders::Uniform01Generator::Uniform01Generator() : rng(),uniform(),seed(get_random_number_from_system()) {
    // Create a random number generator instance
    rng = std::shared_ptr<parallel_benders::base_generator_type>(new parallel_benders::base_generator_type(seed));
    // Create a [0,1) distribution
    boost::uniform_01<> uniform_01_distribution;
    uniform = std::shared_ptr<parallel_benders::uniform_distr_gen>(new parallel_benders::uniform_distr_gen(*(rng.get()),uniform_01_distribution));
}

int parallel_benders::get_random_number_from_system()
{
    
#ifdef _MSC_VER
    //Windows
    errno_t err;
    unsigned int time = 0;
    err = rand_s(&time); //Write seed in seed variable
    if (err != 0){
        std::cerr << "Error occured during seed generation: " << err << std::endl;
        // We need to abort here TODO
    }
    return time;
#endif
    //So we are not compiling using windows compiler, so we have /dev/urandom
#ifndef _MSC_VER
    //Copied from Good Practice RNG.pdf (google for it)
    int fn;
    unsigned int seed;
    fn = open("/dev/urandom", O_RDONLY);
    if (fn == -1)
        exit(-1); /* Failed! */
    if (read(fn, &seed, 4) != 4)
        exit(-1); /* Failed! */
    close(fn);
    return seed;
    //Without random numbers this does not make much sense
#endif
}

double parallel_benders::computeSymmetricConfidenceInterval( int number_of_batches, double alpha, double variance )
{
    assert( number_of_batches > 1);
    assert(alpha > 0 && alpha < 1);
    boost::math::students_t_distribution<double> students_t(number_of_batches-1); // Degrees of distribution is number of variables - 1
    return quantile(students_t,1-alpha/2.0)*(std::sqrt(variance/(number_of_batches)) );
}

double parallel_benders::computeSampleVarianceEstimator( int number_of_samples, double* sampled_values, double average )
{
    assert( number_of_samples > 1);
    double sum = 0;
    for (int i = 0; i < number_of_samples; i++)
        sum += std::pow(sampled_values[i]-average,2);
    sum *= 1.0/(number_of_samples-1);
    return sum;
}