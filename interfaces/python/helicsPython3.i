%feature("doxygen:ignore:forcpponly", range="end");

%include "cstring.i"
%include "python_maps3.i"

%include "helics.i"

/* Add atexit */
%pythoncode %{
import atexit
atexit.register(helicsCloseLibrary)
%}
