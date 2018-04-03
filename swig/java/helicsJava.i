%include "various.i"
%apply(char *STRING, int LENGTH) { (char *str, int maxlen) };

%include "../helics.i"

