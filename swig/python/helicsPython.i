%include "cstring.i"

%cstring_output_maxsize(char *outputString, int maxlen);

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

%typemap(in) double value[ANY] (const double data[], int len) {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  $2=PySequence_Length($input);
  $1 = (double *) malloc($2*sizeof(double));
  
  for (i = 0; i < $2; i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyFloat_Check(o)) {
      $1[i] = PyFloat_AsDouble(o);
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");      
      free($1);
      return NULL;
    }
  }
}

%typemap(freearg) float value[ANY] {
   if ($1) free($1);
}

%typemap(argout) (helics_subscription sub, double data[], int maxlen, int *actualSize) {
  PyObject *o=$input;
  PyObject *o2;
  $3=helicsSubscriptionGetSize(o);
  if ($3 == 0)
  {
	return 2;
  }
  o2=PyList_New($3);
  $2 = (double *) malloc($3*sizeof(double));
  int actSize;
  $4=&actSize;
  
  for (i = 0; i < actSize; i++) {
	PyObjct *o_item=PyFloat_FromDouble($2[i]);
      PyList_SetItem(o2, i, o_item);
      }
	  
  $result = SWIG_Python_AppendOutput($result, o2);
  Py_DECREF(o2);
}

%include carrays.i
%array_class(double, doubleArray);

%include "../helics.i"

