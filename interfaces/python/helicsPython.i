%include "cstring.i"
%include "python_maps.i"

%include "../helics.i"

/* Add atexit */
%pythoncode %{
import atexit
atexit.register(helicsCloseLibrary)
%}

