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
%apply double *OUTPUT {double data[]};
%apply helics_time_t *OUTPUT {helics_time_t*};
%apply char *OUTPUT{char *};
%apply int *OUTPUT{int *};
%apply int64_t *OUTPUT{int64_t *};
%apply federate_state *OUTPUT{federate_state  *state};
%apply helics_iteration_status *OUTPUT{helics_iteration_status *};

%include "api-data.h"
%include "helics.h"
%include "ValueFederate.h"
%include "MessageFederate.h"
%include "MessageFilters.h"
