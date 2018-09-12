
//typemap for short maxlen strings
%typemap(in, numinputs=0) (char *outputString, int maxlen) {
  $2=256;
  $1=(char *)malloc(256);
}

%typemap(argout) (char *outputString, int maxlen) {
  PyObject *str=PyString_FromString($1);
  $result = SWIG_Python_AppendOutput($result, str);
}

%typemap(freearg) (char *outputString, int maxlen) {
   if ($1) free($1);
}


//typemap for large string output with a length return in C
%typemap(in, numinputs=0) (char *outputString, int maxStringlen, int *actualLength) {
  $3=&($2);
}

%typemap(freearg) (char *outputString, int maxStringlen, int *actualLength) {
   if ($1) free($1);
}

%typemap(check)(char *outputString, int maxStringlen, int *actualLength) {
    $2=helicsInputGetStringSize(arg1)+2;
    $1 = (char *) malloc($2);
}

%typemap(argout) (char *outputString, int maxStringlen, int *actualLength) {
  PyObject *o2=PyString_FromString($1);
  $result = SWIG_Python_AppendOutput($result, o2);
}


//typemap for the input arguments
%typemap(in) (int argc, const char *const *argv) {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int i;
    $1 = PyList_Size($input);
    $2 = (char **) malloc(($1+1)*sizeof(char *));
    for (i = 0; i < $1; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$2[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($2);
	return NULL;
      }
    }
    $2[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

%typemap(freearg) (int argc, const char *const *argv) {
  free((char *) $2);
}

// typemap for vector input functions
%typemap(in) (const double *vectorInput, int vectorlength) {
  int i;
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a list");
    return NULL;
  }
  $2=PyList_Size($input);
  $1 = (double *) malloc($2*sizeof(double));

  for (i = 0; i < $2; i++) {
    PyObject *o = PyList_GetItem($input,i);
    if (PyFloat_Check(o)) {
      $1[i] = PyFloat_AsDouble(o);
	}else if (PyInt_Check(o))
	{
		$1[i] = (double)(PyInt_AsLong(o));
	} else {
      PyErr_SetString(PyExc_ValueError,"List elements must be numbers");
      free($1);
      return NULL;
    }
  }
}

%typemap(argout) (const double *vectorInput, int vectorlength)
{
}

%typemap(freearg) (const double *vectorInput, int vectorlength) {
   if ($1) free($1);
}

// typemap for vector output functions

%typemap(arginit) (double data[], int maxlen, int *actualSize) {
  $1=(double *)(NULL);
}

%typemap(in, numinputs=0) (double data[], int maxlen, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (double data[], int maxlen, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(double data[], int maxlen, int *actualSize) {
    $2=helicsInputGetVectorSize(arg1);
    $1 = (double *) malloc($2*sizeof(double));
}

%typemap(argout) (double data[], int maxlen, int *actualSize) {
  int i;
  PyObject *o2=PyList_New(*$3);
  for (i = 0; i < *$3; i++) {
	PyObject *o_item=PyFloat_FromDouble($1[i]);
      PyList_SetItem(o2, i, o_item);
      }

  $result = SWIG_Python_AppendOutput($result, o2);
}

%apply (char *STRING, size_t LENGTH) { (const void *data, int inputDataLength) };

%apply (char *outputString, int maxStringlen, int *actualLength) {(void *data, int maxDatalen, int *actualSize)};
