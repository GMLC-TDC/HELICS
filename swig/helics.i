%include output.i
%include "typemaps.i"
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
%apply helics_time_t *OUTPUT {helics_time_t*};
%apply (char *STRING, size_t LENGTH) { (const char *data, int len) };

%apply int *OUTPUT{int *};
%apply long long *OUTPUT{int64_t *};
%apply int *OUTPUT{federate_state *state};
%apply int *OUTPUT{helics_iteration_status *};
%apply long long {int64_t};

%apply (char *outputString, int maxlen) { (char *identifier, int maxlen) };
%apply (char *outputString, int maxlen) { (char *address, int maxlen) };

%include "api-data.h"
%include "helics.h"
%include "ValueFederate.h"
%include "MessageFederate.h"
%include "MessageFilters.h"

