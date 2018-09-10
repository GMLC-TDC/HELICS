%include "matlab_maps.i"

%ignore helicsInputGetComplex;
%ignore helics_complex;

%rename (helicsGetComplex) helicsInputGetComplexParts;

%include "../helics.i"

