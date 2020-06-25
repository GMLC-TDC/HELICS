
%{
#include "api-data.h"
#include "CMatrix.h"
#include "ov-complex.h"

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
  case   helics_error_insufficient_space:
    return "helics:insufficient_space";
  case   helics_error_other:
  case   helics_error_external_type:
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
    _outv = SWIG_FromCharPtrAndSize($1,*$3-1);
  if (_outv.is_defined()) _outp = SWIG_Octave_AppendOutput(_outp, _outv);
}

%typemap(in, numinputs=0)(double *real, double *imag)(double vals[2])
{
    $1=&(vals[0]);
    $2=&(vals[1]);
}

%typemap(argout)(double *real, double *imag)
{
    Complex c(*$1,*$2);
    //octave_complex cv(c);
    octave_value v(c);
    _outp = SWIG_Octave_AppendOutput(_outp, v);
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
    return octave_value_list();
  }
}

%typemap(freearg) (int argc, const char *const *argv) {
  free((char **) $2);
}

// typemap for vector input functions
%typemap(in) (const double *vectorInput, int vectorLength) {
  if (!$input.is_real_matrix()) {
    SWIG_exception_fail(SWIG_ArgError(3), "argument must be a double array");
    return octave_value_list();
  }
  $2=static_cast<int>($input.numel());
  $1=reinterpret_cast<double *>($input.mex_get_data());
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
   if ($1) free($1);
}

// Set argument to NULL before any conversion occurs
%typemap(check)(double data[], int maxlen, int *actualSize) {
    $2=helicsInputGetVectorSize(arg1);
    $1=(double *)malloc(sizeof(double)*$2);
}

%typemap(argout) (double data[], int maxlen, int *actualSize) {

    Matrix a(*$3,1);
    double *dat=a.fortran_vec();
    for (int ii=0;ii<*$3;++ii)
    {
        dat[ii]=$1[ii];
    }

  _outp = SWIG_Octave_AppendOutput(_outp, a);
}

%apply (char *STRING, size_t LENGTH) { (const void *data, int inputDataLength) };


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
 _outv = SWIG_FromCharPtrAndSize(static_cast<char *>($1),*$3);
  if (_outv.is_defined()) _outp = SWIG_Octave_AppendOutput(_outp, _outv);
}

// typemap for raw data message functions
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
_outv = SWIG_FromCharPtrAndSize(static_cast<char *>($1),*$3);
  if (_outv.is_defined()) _outp = SWIG_Octave_AppendOutput(_outp, _outv);
}
