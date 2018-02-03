%include output.i
#define __attribute__(x)
#pragma SWIG nowarn=451

%module helics

%{
#include "api-data.h"
#include "helics.h"
#include "ValueFederate.h"
#include "MessageFederate.h"
#include "MessageFilters.h"
%}

%apply double *OUTPUT {double*};

%include "api-data.h"
%include "helics.h"
%include "ValueFederate.h"
%include "MessageFederate.h"
%include "MessageFilters.h"
