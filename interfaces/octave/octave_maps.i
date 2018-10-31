
%{
#include "api-data.h"
#include "CMatrix.h"
/* throw a helics error */
static octave_value Helics_ErrorType(helics_error *err) {
switch (err->error_code)
  {
  case helics_error_registration_failure:
	return "helics:registration_failure";
  case   helics_error_connection_failure:
    return "helics:connection_failure";
  case   helics_error_invalid_object:
    return "helics:invalid_object";
  case   helics_error_invalid_argument:
    return "helics:invalid_argument";
  case   helics_error_discard:
    return "helics:discard";
  case helics_error_system_failure:
	return "helics:system_failure";
  case   helics_error_invalid_state_transition:
    return "helics:invalid_state_transition";
  case   helics_error_invalid_function_call:
    return "helics:invalid_function_call";
  case   helics_error_execution_failure:
	return "helics:execution_failure";
  case   helics_error_other:
  case   other_error_type:
  default:
    return "helics:error";
  }
}
static octave_value throwHelicsOctaveError(helics_error *err) {
 octave_value type(Helics_ErrorType(err));
  std::string r(err->message);
  r += " (" + type.string_value() + ")";
  error(r.c_str());
  return octave_value(r);
}

%}

//typemap for short maxlen strings
%typemap(in, numinputs=0) (char *outputString, int maxlen) {
  $2=256;
  $1=(char *)malloc(256);
}

%typemap(argout) (char *outputString, int maxlen) {
  if (--resc>=0) *resv++ = SWIG_FromCharPtr($1);
}

%typemap(freearg) (char *outputString, int maxlen) {
   if ($1) free($1);
}

%typemap(in, numinputs=0) helics_error * (helics_error etemp) {
	etemp=helicsErrorInitialize();
	$1=&etemp;
}

%typemap(freearg) helics_error *
{
	if ($1->error_code!=helics_ok)
	{
		throwHelicsOctaveError($1);
	}
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
	_outv = SWIG_FromCharPtrAndSize($1,*$3);
  if (_outv.is_defined()) _outp = SWIG_Octave_AppendOutput(_outp, _outv);
}

%typemap(in, numinputs=0)(double *real, double *imag)(double vals[2])
{
	$1=&(vals[0]);
	$2=&(vals[1]);
}

%typemap(argout)(double *real, double *imag)
{
	ComplexMatrix mat(1,1);
	mat(1,1)=std::complex<double>($1,$2);
	if (_outv.is_defined()) _outp = SWIG_Octave_AppendOutput(_outp, _outv);
}


%typemap(in) (double real, double imag)
{
	if($input.is_complex_scalar())
	{
		Complex arg=$input.complex_value();
		$1=arg.real();
		$2=arg.imag();
	}  
    else if ($input.is_float_type())
	{
		$2=0.0;
		$1=$input.double_value();
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
  if ($input.is_cellstr()) {
	Cell cellargs=octave_value_extract<Cell>($input);
	$2 = (char **) malloc((cellargs.numel()+1)*sizeof(char *));
	for (int ii=0;ii<cellargs.numel();++ii)
	{
        octave_value arg=cellargs(ii);
        int alloc;
		SWIG_AsCharPtrAndSize(arg, &$2[ii+1], NULL, &alloc);
	}
    
  } 
  else if ($input.is_string())
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
  }
}

%typemap(freearg) (int argc, const char *const *argv) {
  free((char **) $2);
}

// typemap for vector input functions
%typemap(in) (const double *vectorInput, int vectorlength) {
  if (!mxIsDouble($input)) {
    SWIG_exception_fail(SWIG_ArgError(3), "argument must be a double array");
    return NULL;
  }
  $2=static_cast<int>(mxGetNumberOfElements($input));
  $1=mxGetPr($input);
}

%typemap(argout) (const double *vectorInput, int vectorlength)
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
%typemap(in, numinputs=0) (void *data, int maxDatalen, int *actualSize) {
  $3=&($2);
}

%typemap(freearg) (void *data, int maxDatalen, int *actualSize) {
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(void *data, int maxDatalen, int *actualSize) {
    $2=helicsSubscriptionGetValueSize(arg1)+2;
    $1 =  malloc($2);
}

%typemap(argout) (void *data, int maxDatalen, int *actualSize) {
 if (--resc>=0) *resv++ = SWIG_FromCharPtrAndSize($1,*$3);
}
