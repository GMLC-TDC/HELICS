%include "various.i"

%apply (char[] OUTPUT, size_t LENGTH) { (char *str, int maxlen) }

%include "../helics.i"

