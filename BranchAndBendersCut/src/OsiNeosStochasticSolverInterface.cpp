#include "OsiNeosStochasticSolverInterface.hpp"

//
//<document>
//<category>slp</category>
//<solver>bnbs</solver>
//<inputMethod>SMPS</inputMethod>
//
//<cor><![CDATA[
//    ...Insert Value Here...
//]]></cor>
//
//<tim><![CDATA[
//    ...Insert Value Here...
//]]></tim>
//
//<sto><![CDATA[
//    ...Insert Value Here...
//]]></sto>
//
//<par><![CDATA[
//    ...Insert Value Here...
//]]></par>
//
//<ALGTYPE><![CDATA[
//    ...Insert Value Here...
//]]></ALGTYPE>
//
//<comment><![CDATA[
//    ...Insert Value Here...
//]]></comment>
//
//</document>

void OsiStochasticNeosSolverInterface::initialSolve()
    {
        // We need to prepare the XML String
        std::string xml_string;
        std::string category("slp");
        std::string solver("bnbs");
        std::string inputMethod("SMPS");

        std::stringstream xml_string_stream;
        xml_string_stream << "<document>" << std::endl;
        xml_string_stream << "<category>" << category << "</category>" << std::endl;
        xml_string_stream << "<solver>" << solver << "</solver>" << std::endl;
        xml_string_stream << "<inputMethod>" << inputMethod << "</inputMethod>" << std::endl;
        xml_string_stream << std::endl;

        // Read in cor file

        // Read in time file

        // Read in stoch file

        // Read in parameter file
        
        // Set algtype

        // Set comment

        xml_string_stream << "</document>" << std::endl;

        // Send xmls_string_stream as string to NEOS.
        
        // Specific stuff: ALGTYPE == CPLEX, if we use bnbs 
        std::string algtype("CPLEX");

    }
