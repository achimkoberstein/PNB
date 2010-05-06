#include "MP_shared_implementation_details.hpp"

//Windows. Construct necessary as VisualC++ include stdlib.h before I can set _CRT_RAND_S
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif 
#include <stdlib.h>
//Linux: Call /dev/urandom to generate a seed

using namespace flopc;

Uniform01Generator::Uniform01Generator() : rng(),uniform(),seed(0) {
            //Windows
            errno_t err;
            err = rand_s(&seed);
            if (err != 0)
                std::cerr << "Error occured during seed generation: " << err << std::endl;
            std::cout << "Seed is: " << seed << std::endl;
        };