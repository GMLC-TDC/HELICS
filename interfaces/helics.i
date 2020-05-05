%include "typemaps.i"
#define HELICS_EXPORT
#define HELICS_DEPRECATED
#define HELICS_DEPRECATED_EXPORT

#pragma SWIG nowarn=451

%module helics

%{
#include "ValueFederate.h"
#include "MessageFederate.h"
#include "MessageFilters.h"
%}

%apply double *OUTPUT {double*};
%apply helics_time *OUTPUT {helics_time*};

%apply int *OUTPUT{int *};
%apply long long *OUTPUT{int64_t *};
%apply int *OUTPUT{helics_federate_state *state};
%apply int *OUTPUT{helics_iteration_result *};
%apply long long {int64_t};

%ignore helicsErrorInitialize;
%ignore helicsErrorClear;
%ignore helics_error;
%ignore helicsMessageGetRawDataPointer;
%ignore helicsMessageResize;

%include "../helics_enums.h"
%include "api-data.h"
%include "helics.h"
%include "ValueFederate.h"
%include "MessageFederate.h"
%include "MessageFilters.h"
