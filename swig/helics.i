#define __attribute__(x)

#define HELICS_Export __attribute__ ((visibility ("default")))

%module helics
%{
#include "api-data.h"
#include "helics.h"
#include "ValueFederate_c.h"
%}

%apply double *OUTPUT {double*};

%include "api-data.h"
%include "helics.h"
%include "ValueFederate_c.h"



