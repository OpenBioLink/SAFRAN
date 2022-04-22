%module safran_wrapper

%{
#define SWIG_FILE_WITH_INIT
#include "safran_wrapper.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_pair.i>
%include "safran_wrapper.h"

namespace std {
    %template(query_triples_t) vector<string>;
    %template(query_p1) pair<float, int>;
    %template(query_p2) pair<string, pair<float, int>>;
    %template(query_p3) pair<string, pair<string, pair<float, int>>>;
    %template(query_p4) pair<string, pair<string, pair<string, pair<float, int>>>>;
    %template(query_output_t) vector<pair<string, pair<string, pair<string, pair<float, int>>>>>;
}
