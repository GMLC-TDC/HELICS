
%{
#include "api-data.h"

static PyObject* pHelicsException;

/* throw a helics error */
static void throwHelicsPythonException(helics_error *err) {
  char str[256];
  switch (err->error_code)
  {
  case helics_ok:
    return;
  case helics_error_registration_failure:
    strcat(str, "helics:registration_failure - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_connection_failure:
    strcat(str, "helics:connection_failure - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_invalid_object:
    strcat(str, "helics:invalid_object - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_invalid_argument:
    strcat(str, "helics:invalid_argument - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_discard:
    strcat(str, "helics:discard - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case helics_error_system_failure:
    strcat(str, "helics:system_failure - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_invalid_state_transition:
    strcat(str, "helics:invalid_state_transition - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_invalid_function_call:
    strcat(str, "helics:invalid_function_call - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_execution_failure:
    strcat(str, "helics:execution_failure - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  case   helics_error_other:
  case   helics_error_external_type:
  default:
    strcat(str, "helics:error - ");
    strcat(str, err->message);
    PyErr_SetString(pHelicsException, str);
    break;
  }
}

%}

%init %{
pHelicsException = PyErr_NewException("_helics.HelicsException", NULL, NULL);
Py_INCREF(pHelicsException);
PyModule_AddObject(m, "HelicsException", pHelicsException);
%}


%typemap(in, numinputs=0) helics_error * (helics_error etemp) {
    etemp=helicsErrorInitialize();
    $1=&etemp;
}

%typemap(freearg) helics_error *
{
    if ($1->error_code!=helics_ok)
    {
        throwHelicsPythonException($1);
        return NULL;
    }
}

//typemap for large string output with a length return in C
%typemap(in, numinputs=0) (char *outputString, int maxStringLength, int *actualLength) {
  $3=&($2);
}

%typemap(freearg) (char *outputString, int maxStringLength, int *actualLength) {
   if ($1) free($1);
}

%typemap(check)(char *outputString, int maxStringLength, int *actualLength) {
    $2=helicsInputGetStringSize(arg1)+2;
    $1 = (char *) malloc($2);
}

%typemap(argout) (char *outputString, int maxStringLength, int *actualLength) {
  PyObject *o2=PyString_FromString($1);
  $result = SWIG_Python_AppendOutput($result, o2);
}


//typemap for the input arguments
%typemap(in) (int argc, const char *const *argv) {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int i;
    $1 = (int)(PyList_Size($input));
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
%typemap(in) (const double *vectorInput, int vectorLength) {
  int i;
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a list");
    return NULL;
  }
  $2=(int)(PyList_Size($input));
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

%typemap(argout) (const double *vectorInput, int vectorLength)
{
}

%typemap(freearg) (const double *vectorInput, int vectorLength) {
   if ($1) free($1);
}

// typemap for vector output functions

%typemap(arginit) (double data[], int maxLength, int *actualSize) {
  $1=(double *)(NULL);
}

%typemap(in, numinputs=0) (double data[], int maxLength, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (double data[], int maxLength, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(double data[], int maxLength, int *actualSize) {
    $2=helicsInputGetVectorSize(arg1);
    $1 = (double *) malloc($2*sizeof(double));
}

%typemap(argout) (double data[], int maxLength, int *actualSize) {
  int i;
  PyObject *o2=PyList_New(*$3);
  for (i = 0; i < *$3; i++) {
    PyObject *o_item=PyFloat_FromDouble($1[i]);
      PyList_SetItem(o2, i, o_item);
      }

  $result = SWIG_Python_AppendOutput($result, o2);
}



// typemap for raw data output function
%typemap(in, numinputs=0) (void *data, int maxDataLength, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (void *data, int maxDataLength, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(void *data, int maxDataLength, int *actualSize) {
    $2=helicsInputGetRawValueSize(arg1)+2;
    $1 =  malloc($2);
}

%typemap(argout) (void *data, int maxDataLength, int *actualSize) {
  PyObject *o2=PyBytes_FromStringAndSize($1,*$3);
  $result = SWIG_Python_AppendOutput($result, o2);
}


// typemap for raw message data output
%typemap(in, numinputs=0) (void *data, int maxMessageLength, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (void *data, int maxMessageLength, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(void *data, int maxMessageLength, int *actualSize) {
    $2=helicsMessageGetRawDataSize(arg1)+2;
    $1 =  malloc($2);
}

%typemap(argout) (void *data, int maxMessageLength, int *actualSize) {
  PyObject *o2=PyBytes_FromStringAndSize($1,*$3);
  $result = SWIG_Python_AppendOutput($result, o2);
}

%apply (char *STRING, size_t LENGTH) { (const void *data, int inputDataLength) };
