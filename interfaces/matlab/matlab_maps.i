
%{
#include "api-data.h"
/* throw a helics error */
static void throwHelicsMatlabError(helics_error *err) {
  switch (err->error_code)
  {
  case helics_ok:
    return;
  case helics_error_registration_failure:
    mexErrMsgIdAndTxt( "helics:registration_failure", err->message);
    break;
  case   helics_error_connection_failure:
  mexErrMsgIdAndTxt( "helics:connection_failure", err->message);
    break;
  case   helics_error_invalid_object:
  mexErrMsgIdAndTxt( "helics:invalid_object", err->message);
    break;
  case   helics_error_invalid_argument:
  mexErrMsgIdAndTxt( "helics:invalid_argument", err->message);
    break;
  case   helics_error_discard:
  mexErrMsgIdAndTxt( "helics:discard", err->message);
    break;
  case helics_error_system_failure:
    mexErrMsgIdAndTxt( "helics:system_failure", err->message);
    break;
  case   helics_error_invalid_state_transition:
  mexErrMsgIdAndTxt( "helics:invalid_state_transition", err->message);
    break;
  case   helics_error_invalid_function_call:
  mexErrMsgIdAndTxt( "helics:invalid_function_call", err->message);
    break;
  case   helics_error_execution_failure:
  mexErrMsgIdAndTxt( "helics:execution_failure", err->message);
    break;
  case   helics_error_insufficient_space:
    mexErrMsgIdAndTxt( "helics:insufficient_space", err->message);
    break;
  case   helics_error_other:
  case   helics_error_external_type:
  default:
  mexErrMsgIdAndTxt( "helics:error", err->message);
    break;
  }
}

%}

%typemap(in, numinputs=0) helics_error * (helics_error etemp) {
    etemp=helicsErrorInitialize();
    $1=&etemp;
}

%typemap(freearg) helics_error *
{
    if ($1->error_code!=helics_ok)
    {
        throwHelicsMatlabError($1);
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

  if (--resc>=0) *resv++ = SWIG_FromCharPtrAndSize($1,*$3-1);
}

%typemap(in, numinputs=0)(double *real, double *imag)(double vals[2])
{
    $1=&(vals[0]);
    $2=&(vals[1]);
}

%typemap(argout)(double *real, double *imag)
{
    mxArray *out=mxCreateDoubleMatrix(1, 1, mxCOMPLEX);
    double *r=mxGetPr(out);
    double *i=mxGetPi(out);
    *r=*$1;
    *i=*$2;
    if (--resc>=0) *resv++ =out;
}


%typemap(in) (double real, double imag)
{
    if(mxIsComplex($input))
    {

        $1=mxGetPr($input)[0];
        $2=mxGetPi($input)[0];
    }
    else if (mxIsDouble($input))
    {
        $2=0.0;
        $1=mxGetPr($input)[0];
    }
    else
    {
        $1=0.0;
        $2 = 0.0;
    }
}

//typemap for the input arguments
%typemap(in) (int argc, const char *const *argv) {
  /* Check if is a list */
  if (mxIsCell($input)) {
    int ii;
    int allocation2=0;
    char *buffer_cell=NULL;
    int cellSize=static_cast<int>(mxGetNumberOfElements($input));
    $2 = (char **) malloc((cellSize+1)*sizeof(char *));
    for (ii=0;ii<cellSize;++ii)
    {
        mxArray *cellElement=mxGetCell($input, ii);
        int resCode = SWIG_AsCharPtrAndSize(cellElement, &buffer_cell, NULL, &allocation2);
        if (!SWIG_IsOK(resCode)) {
            SWIG_exception_fail(SWIG_ArgError(resCode), "cell elements must be a string");
        }
        $2[ii+1]=buffer_cell;
    }

  }
  else if (mxIsChar($input))
  {
  int retval=0;
  char *buffer=NULL;
  int allocation=0;
    $1=2;
    $2 = (char **) malloc(2*sizeof(char *));
    retval = SWIG_AsCharPtrAndSize($input, &buffer, NULL, &allocation);
  if (!SWIG_IsOK(retval)) {
    SWIG_exception_fail(SWIG_ArgError(retval), "conversion to string failed");
  }
    $2[0]=buffer;
    $2[1]=buffer;
  }
  else
  {
    SWIG_exception_fail(SWIG_ArgError(3), "argument must be a cell array or string");
    return NULL;
  }
}

%typemap(freearg) (int argc, const char *const *argv) {
  free((char **) $2);
}

// typemap for vector input functions
%typemap(in) (const double *vectorInput, int vectorLength) {
  if (!mxIsDouble($input)) {
    SWIG_exception_fail(SWIG_ArgError(3), "argument must be a double array");
    return NULL;
  }
  $2=static_cast<int>(mxGetNumberOfElements($input));
  $1=mxGetPr($input);
}

%typemap(argout) (const double *vectorInput, int vectorLength)
{
}

// typemap for vector output functions

%typemap(arginit) (double data[], int maxlen, int *actualSize) {
  $1=(double *)(NULL);
}

%typemap(in, numinputs=0) (double data[], int maxlen, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (double data[], int maxlen, int *actualSize) {
   //if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(double data[], int maxlen, int *actualSize) {
    $2=helicsInputGetVectorSize(arg1);
    $1 = (double *) mxCalloc($2,sizeof(double));
}

%typemap(argout) (double data[], int maxlen, int *actualSize) {

    mxArray *mat=mxCreateDoubleMatrix(*$3,1,mxREAL);
    mxSetPr(mat,$1);
  if (--resc>=0) *resv++ = mat;
}

%apply (char *STRING, size_t LENGTH) { (const void *data, int inputDataLength) };
/*
// typemap for raw data input
%typemap(in) (const void *data, int inputDataLength) {
  if (PyUnicode_Check($input)) {
    int kind=PyUnicode_KIND($input);
    $1=PyUnicode_DATA($input);
    switch(kind)
    {
    case PyUnicode_1BYTE_KIND:
    default:
        $2=PyUnicode_GetLength($input);
    break;
    case PyUnicode_2BYTE_KIND:
    case PyUnicode_WCHAR_KIND:
        $2=PyUnicode_GetLength($input)*2;
    break;
    case PyUnicode_4BYTE_KIND:
        $2=PyUnicode_GetLength($input)*4;
    break;
    }
  }
  else if (PyBytes_Check($input)) {
    $1=PyBytes_AsString($input);
    $2=PyBytes_Size($input);
  }
  else
  {
    PyErr_SetString(PyExc_ValueError,"Expected a string or bytes");
   return NULL;
 }
}
*/

//%typemap(argout) (const void *data, int inputDataLength)
//{
//}


// typemap for raw data output function
%typemap(in, numinputs=0) (void *data, int maxDataLength, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (void *data, int maxDataLength, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(void *data, int maxDataLength, int *actualSize) {
    $2 = helicsInputGetRawValueSize(arg1) + 2;
    $1 =  malloc($2);
}

%typemap(argout) (void *data, int maxDataLength, int *actualSize) {
 if (--resc>=0) *resv++ = SWIG_FromCharPtrAndSize((char*)$1,*$3);
}

// typemap for raw message data functions
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
 if (--resc>=0) *resv++ = SWIG_FromCharPtrAndSize((char*)$1,*$3);
}
