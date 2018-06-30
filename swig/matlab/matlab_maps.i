
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


//typemap for large string output with a length return in C
%typemap(in, numinputs=0) (char *outputString, int maxStringlen, int *actualLength) {
  $3=&($2);
}

%typemap(freearg) (char *outputString, int maxStringlen, int *actualLength) {
   if ($1) free($1);
}

%typemap(check)(char *outputString, int maxStringlen, int *actualLength) {
    $2=helicsSubscriptionGetStringSize(arg1)+2;
    $1 = (char *) malloc($2);
}

%typemap(argout) (char *outputString, int maxStringlen, int *actualLength) {
  if (--resc>=0) *resv++ = SWIG_FromCharPtrAndSize($1,*$3);
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
    $2=helicsSubscriptionGetVectorSize(arg1);
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
