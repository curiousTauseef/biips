#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "include/common/Error.hpp"

using namespace std;
using namespace Biips;

void parse_one_varname(const string & to_parse,
                  string & var_name,
                  vector<size_t> & lower,
                  vector<size_t> & upper) {

   using boost::regex;
   using boost::algorithm::split;
   using boost::is_any_of;
   using boost::lexical_cast;

   // regexp alpha sequence not beginning with
   // a number followed eventually by some characters,
   // between brackets
   regex re("(\\D[^\\[\\]]*){1}(\\[([\\d|,|:]+)\\])?", boost::regex::perl);
   boost::cmatch matches;

   if (boost::regex_match(to_parse.c_str(), matches, re))
   {
     var_name = string(matches[1].first, matches[1].second);
     lower.resize(0);
     upper.resize(0);
     // les crochets contiennent quelque chose
     if(matches[3].first != matches[3].second){
        string tmp = string(matches[3].first, matches[3].second); 
        
        vector<string> result;
        split(result, tmp, is_any_of(","));
        
        lower.resize(result.size());
        upper.resize(result.size());
        
        for(int i = 0; i < result.size() ; ++i) {
            vector<string> res2;
            if (result[i] == "") {
                lower[i] = 0;
                upper[i] = 0;
            }
            else {  
                split(res2, result[i], is_any_of(":"));
                // pas de ":" trouve
                if (res2.size() == 1) {
                    try {
                      upper[i] = lower[i] = lexical_cast<size_t>(result[i]);
                    }
                    catch (boost::bad_lexical_cast const&) {
                       throw RuntimeError("bad bound conversion number in a a:b domaine");
                    }
                }    
                // trouve un ":"
                else if (res2.size() == 2) {
                    try {
                      lower[i] = lexical_cast<size_t>(res2[0]);
                      upper[i] = lexical_cast<size_t>(res2[1]);
                      if (lower[i] > upper[i])
                       throw RuntimeError("a must be strictly less than b in a  a:b domaine");
                    
                    }
                    catch (boost::bad_lexical_cast const&){
                       throw RuntimeError("bad bound conversion number in a a:b domaine");
                    }
                    assert(lower[i]<=upper[i]);
                }
                else {
                    throw RuntimeError("the domain variable is not correct : to much : between ,");
                }
            }
        } 

     }
   }
   else
   {
      string message = to_parse  + string("does not correspond to a valid variable expression");
      throw Biips::RuntimeError(message.c_str());
   }
}                  

void parse_varnames(const vector<string> & to_parse,
                   vector<string> & var_names,
                   vector<vector<size_t> > & lower,
                   vector<vector<size_t> > & upper) {
     
     var_names.resize(to_parse.size());
     lower.resize(to_parse.size());
     upper.resize(to_parse.size());
     
     for(int i = 0; i < to_parse.size(); ++i) 
         parse_one_varname(to_parse[i], var_names[i], lower[i], upper[i]);
}
