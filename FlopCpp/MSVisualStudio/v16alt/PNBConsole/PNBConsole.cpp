// PNBConsole.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <string>
#include <exception>

#include <boost/exception/all.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/xml_parser.hpp>
#define GOOGLE_GLOG_DLL_DECL // Has to be done to correct linking issues
#include <glog/logging.h>

#include "CoinTime.hpp"
#include "OsiCpxSolverInterface.hpp"
//#include "OsiGrbSolverInterface.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "ParallelNestedBenders.hpp"
#include "SmiScnModel.hpp"




int main(int argc,  char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();


    std::string smps;
    std::string core;
    std::string time;
    std::string stoch;
    std::string parameter;
	std::string specs_to_file;

    google::SetLogDestination(google::WARNING, "google_log_"); // output file
    //google::LogToStderr(); // only to console
    google::SetStderrLogging(google::INFO); // logging to std::err enabled for all logging levels >= 0 (Different logging under release builds?)

    // Declare the supported options.
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("smps", boost::program_options::value<std::string>(&smps), "SMPS base name")
        ("core", boost::program_options::value<std::string>(&core), "Core File")
        ("time", boost::program_options::value<std::string>(&time), "Time File")
        ("stoch", boost::program_options::value<std::string>(&stoch), "Stoch File")
        ("parameter", boost::program_options::value<std::string>(&parameter), "Parameter File")
		("specs_to_file", boost::program_options::value<std::string>(&specs_to_file), "Instance Specs Output File")
        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // Parameter file
    parallel_benders::Parameter param;
    param.init();
    write_xml("default_param.xml",param.parameter_);

    if ( vm.count("parameter")){
        // reading file.xml
        try {
            read_xml(parameter.c_str(), param.parameter_);
        }
        catch (std::exception& e ){
            LOG(ERROR) << "Could not read in parameter file, using standard parameters.";
        }
    }

    // Subject to property
    google::SetStderrLogging(0); 
    
    double time_d = CoinCpuTime(); 

    int smps_read_in = 0;
    SmiScnModel smi_model;
	std::string instance_path;
    if (vm.count("smps")) {
		instance_path = smps.c_str();
        smps_read_in = smi_model.readSmps(smps.c_str());
    } else {
        if ( vm.count("core") && vm.count("time") && vm.count("stoch") ){
			instance_path = stoch.c_str();
            smps_read_in = smi_model.readSmps(core.c_str(),time.c_str(),stoch.c_str());
        }
        else{
            LOG(ERROR) << "You have to specify a problem by --smps or --time --core --stoch.";
            return -1;
        }
    }
    if ( smps_read_in != 0){
        LOG(ERROR) << "Reading in your SMPS file failed.";
        return -1;
    }

    // Create a new OsiStochasticPNB interface
    OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
    OsiStochasticParallelNestedBendersSolverInterface pnb(osi_solver,&smi_model);
	pnb.setTestsetProblemInstanceNames(instance_path);
	if (vm.count("specs_to_file")) {
		pnb.setInstanceSpecsFile(specs_to_file.c_str());
	}	

    pnb.setParameters(param);
    pnb.initialSolve();
	time_d = CoinCpuTime()-time_d;
	LOG(INFO) << "Overall solution time : "<< time_d << " s.";
	LOG(INFO) << "Writing solution to file primal_solution.txt";
    if ( pnb.getSolutionCode() == 0 )
        pnb.printSolutionToFile("primal_solution.txt");

    delete osi_solver;
    

	return 0;
}

