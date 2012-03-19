// ResultProcessing.cpp : Defines the entry point for the console application.
//
#include <glog/logging.h>
#include <stdio.h>
#include <iostream>


#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <string>
#include <sstream>

int main(){
    using namespace std;

    std::ifstream original;
    std::ofstream output;
    std::stringstream temp_stream;
    std::stringstream not_yet_processed;


    original.open("Results.csv");
    output.open("Results-changed.csv");

    if ( !original.is_open() || !output.is_open()  ){
        //LOG(ERROR) << "Could not open file.";
        std::cout << "blubb";
    }
    // Copy original to not_yet processed
    original.seekg(0,ios::beg);
    not_yet_processed << original.rdbuf();
    original.seekg(0,ios::beg);
    not_yet_processed.seekg(0,ios::beg);

    std::string title;
    std::vector<std::string> fields;

    std::getline(original,title);
    boost::escaped_list_separator<char> esl("",";","");
    boost::tokenizer<boost::escaped_list_separator<char> > tokenizer(title,esl);
    for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg=tokenizer.begin(); beg!=tokenizer.end();++beg){
        fields.push_back(*beg);
    }
    std::getline(not_yet_processed,title);// Skip the first line too!

    // Read the first line of input and store position afterwards
    std::string line;
    // Prepare boost expressions!
    boost::smatch matches;
    boost::regex replace_plus("[+]"); // Find all + literals
    std::string replacement_plus("[+]");// Replace all + literals with [+], so we can use that later as a regex

    while(std::getline(not_yet_processed,line)){
        boost::tokenizer<boost::escaped_list_separator<char> > tok(line,esl);
        int current_index = 0;
        int count_occurences = 1;
        std::string regex;
        std::vector<double> results(4,0.0); // Store numerical values we want to extract!
        for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg){
            // Build a regex search string via this tokens!
            if ( fields[current_index] ==  " Overall Wall Clock Time (s)" || fields[current_index] == " Iterations" || fields[current_index] == " Solver Wall Clock Time (s)" ){
                regex += "(.*?);"; // Non-Greedy Matching!
            }
            else if ( fields[current_index] == " Time Setup"){
                regex += ".*"; // Non-Greedy Matching!
                break;
            }
            else {

                regex += *beg + ";";
            }
            current_index++;
        }
        regex = boost::regex_replace(regex,replace_plus,replacement_plus); // Replace + with [+] to prevent special characters!
        boost::regex regular_expression(regex);

        // Start with this line!
        bool matched = boost::regex_search(line,matches,regular_expression);
        for (int i = 1; i < matches.size(); i++){
            // Not possible to use lexical_cast (throws errors in connection with iostream?)
            results[i] += strtod(std::string(matches[i]).c_str(),NULL);
        }


        // Now we iterate over the rest of the file!
        std::string search_string;

        while ( std::getline(not_yet_processed,search_string) ){
            matched = boost::regex_search(search_string,matches,regular_expression);
            if ( matched ) {
                count_occurences++;
                for (int i = 1; i < matches.size(); i++)
                    results[i] += strtod(std::string(matches[i]).c_str(),NULL);
                    
            }
            else { // Write output to temporary file or std::stringstream!
                temp_stream << search_string << std::endl;
            }
        }
        // Now create the string again
        std::stringstream output_string;
        current_index = 0;
        for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg){
            // Build a regex search string via this tokens!
            if ( fields[current_index] ==  " Overall Wall Clock Time (s)" )
                output_string << results[1]/count_occurences;
            else if (fields[current_index] == " Iterations" ) 
                output_string << results[2]/count_occurences;
            else if (fields[current_index] == " Solver Wall Clock Time (s)" ){
                output_string << results[3]/count_occurences;                
            }
            else if ( fields[current_index] == " Time Setup"){
                break;
            }
            else {
                output_string << *beg;
            }
            output_string << "; ";
            current_index++;
        }
        output << output_string.str() << std::endl;
        std::cout << output_string.str() << std::endl;
        //std::string replace_string("\\")
        //// Now we want to replace the line string with the values we received
        //std::string output_string = boost::regex_replace(line,regex,replace_string);
        not_yet_processed.clear();
        not_yet_processed.str("");
        not_yet_processed << temp_stream.rdbuf();
        //std::cout << not_yet_processed.str();
        not_yet_processed.seekg(0,ios::beg);
        temp_stream.clear(); // 
        temp_stream.str("");

    }


    


    // First Line of File identifies fields

    // 
    original.close();
    output.close();
}