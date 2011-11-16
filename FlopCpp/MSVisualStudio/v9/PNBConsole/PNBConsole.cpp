// PNBConsole.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "OsiCpxSolverInterface.hpp"
#include "SmiScnModel.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "ParallelNestedBenders.hpp"




int main(int argc,  char* argv[])
{
    std::string smps;
    std::string core;
    std::string time;
    std::string stoch;
    std::string parameter;

    // Declare the supported options.
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("smps", boost::program_options::value<std::string>(&smps), "SMPS base name")
        ("core", boost::program_options::value<std::string>(&core), "Core File")
        ("time", boost::program_options::value<std::string>(&time), "Time File")
        ("stoch", boost::program_options::value<std::string>(&stoch), "Stoch File")
        ("parameter", boost::program_options::value<std::string>(&parameter), "Parameter File")

        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    SmiScnModel smi_model;
    if (vm.count("smps")) {
        smi_model.readSmps(smps.c_str());
    } else {
        if ( vm.count("core") && vm.count("time") && vm.count("stoch") ){
            smi_model.readSmps(core.c_str(),time.c_str(),stoch.c_str());
        }
        else{
            std::cout << "You have to specify a problem by --smps or --time --core --stoch.";
            return -1;
        }
    }

    // Parameter file
    parallel_benders::Parameter param;
    param.init();

    if ( vm.count("parameter")){
        // reading file.xml
        read_xml(parameter.c_str(), param.parameter_);
    }


    // Create a new OsiStochasticPNB interface
    OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
    OsiStochasticParallelNestedBendersSolverInterface pnb(osi_solver,&smi_model);

    pnb.setParameters(param);
    pnb.initialSolve();

    delete osi_solver;
	return 0;
}

