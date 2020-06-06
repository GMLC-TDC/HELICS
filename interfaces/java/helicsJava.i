%feature("doxygen:ignore:forcpponly", range="end");

%include "various.i"
%apply(char *STRING, int LENGTH) { (char *str, int maxlen) };
%include "java_maps.i"

%include "../helics.i"
