%include "various.i"
%apply(char *STRING, int LENGTH) { (char *str, int maxlen) };
%include "csharp_maps.i"

%include "../helics.i"


