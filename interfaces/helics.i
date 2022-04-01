%include "typemaps.i"
#define HELICS_EXPORT
#define HELICS_DEPRECATED
#define HELICS_DEPRECATED_EXPORT

#pragma SWIG nowarn=451

%module helics

%{
#include "helics/helics.h"
%}

%apply double *OUTPUT {double*};
%apply HelicsTime *OUTPUT {HelicsTime*};

%apply int *OUTPUT{int *};
%apply long long *OUTPUT{int64_t *};
%apply int *OUTPUT{HelicsFederateState *state};
%apply int *OUTPUT{HelicsIterationResult *};
%apply long long {int64_t};

%ignore helicsErrorInitialize;
%ignore helicsErrorClear;
%ignore HelicsError;
%ignore helicsMessageGetRawDataPointer;
%ignore helicsMessageResize;
%ignore helicsBrokerSetLoggingCallback;
%ignore helicsCoreSetLoggingCallback;
%ignore helicsFederateSetLoggingCallback;
%ignore helicsFilterSetCustomCallback;
%ignore helicsTranslatorSetCustomCallback;
%ignore helicsFederateSetQueryCallback;
%ignore helicsQueryBufferFill;
%ignore helicsLoadSignalHandlerCallback;

%include "helics/helics.h"
