%{
  #include "api-data.h"

%}

//typemap for short maxLength strings
%typemap(in) (char *outputString, int maxLength) {
  $1 = (char*)JCALL2(GetByteArrayElements, jenv, $input, 0);
  $2 = (int)JCALL1(GetArrayLength, jenv, $input);
}

%typemap(argout) (char *outputString, int maxLength) {
  JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte*)$1, 0);
}

%typemap(jni) (char *outputString, int maxLength) "jbyteArray"
%typemap(jtype) (char *outputString, int maxLength) "byte[]"
%typemap(jstype) (char *outputString, int maxLength) "byte[]"
%typemap(javain) (char *outputString, int maxLength) "$javainput"

%apply (char *outputString, int maxLength) { (char *outputString, int maxStringLength) };

////typemap for large string output with a length return in C
//%typemap(in, numinputs=0) (char *outputString, int maxStringLength, int *actualLength) {
//  $3=&($2);
//}
//
//%typemap(freearg) (char *outputString, int maxStringLength, int *actualLength) {
//   if ($1) free($1);
//}
//
//%typemap(check)(char *outputString, int maxStringLength, int *actualLength) {
//    $2=helicsInputGetStringSize(arg1)+2;
//    $1 = (char *) malloc($2);
//}
//
//%typemap(argout) (char *outputString, int maxStringLength, int *actualLength) {
//  PyObject *o2=PyString_FromString($1);
//  $result = SWIG_Python_AppendOutput($result, o2);
//}
//
//
%typemap(in) (int argc, char **argv) {
  int i = 0;
  $1=JCALL1(GetArrayLength,jenv,$input);
  $2 = (char **) malloc(($1+1)*sizeof(char *));
  /* make a copy of each string */
  for (i = 0; i<$1; i++) {
    jstring j_string = (jstring)JCALL2(GetObjectArrayElement,jenv,$input,i);
    const char * c_string = JCALL2(GetStringUTFChars,jenv,j_string,0);
    $2[i] = malloc((strlen(c_string)+1)*sizeof(char));
    strcpy($2[i], c_string);
    JCALL2(ReleaseStringUTFChars,jenv, j_string, c_string);
    JCALL1(DeleteLocalRef,jenv, j_string);
  }
  $2[i] = 0;
}

%typemap(freearg) (int argc, char **argv) {
  int i;
  for (i=0; i<$1-1; i++)
    free($2[i]);
  free($2);
}

%typemap(jni) (int argc, char **argv) "jobjectArray"
%typemap(jtype) (int argc, char **argv) "String[]"
%typemap(jstype) (int argc, char **argv) "String[]"

%typemap(javain) (int argc, char **argv) "$javainput"

%typemap(in, numinputs=0) helics_error * (helics_error etemp) {
    etemp=helicsErrorInitialize();
    $1=&etemp;
}

%typemap(freearg) helics_error *
{
    if ($1->error_code!=helics_ok)
    {
        jclass clazz = (*jenv)->FindClass(jenv, "java/lang/Exception");
        (*jenv)->ThrowNew(jenv, clazz, $1->message);
    }
}


//
//// typemap for vector input functions
//%typemap(in) (const double *vectorInput, int vectorLength) {
//  int i;
//  if (!PyList_Check($input)) {
//    PyErr_SetString(PyExc_ValueError,"Expected a list");
//    return NULL;
//  }
//  $2=PyList_Size($input);
//  $1 = (double *) malloc($2*sizeof(double));
//
//  for (i = 0; i < $2; i++) {
//    PyObject *o = PyList_GetItem($input,i);
//    if (PyFloat_Check(o)) {
//      $1[i] = PyFloat_AsDouble(o);
//    }else if (PyInt_Check(o))
//    {
//        $1[i] = (double)(PyInt_AsLong(o));
//    } else {
//      PyErr_SetString(PyExc_ValueError,"List elements must be numbers");
//      free($1);
//      return NULL;
//    }
//  }
//}
//
//%typemap(argout) (const double *vectorInput, int vectorLength)
//{
//}
//
//%typemap(freearg) (const double *vectorInput, int vectorLength) {
//   if ($1) free($1);
//}
//
//// typemap for vector output functions
//
//%typemap(arginit) (double data[], int maxlen, int *actualSize) {
//  $1=(double *)(NULL);
//}
//
//%typemap(in, numinputs=0) (double data[], int maxlen, int *actualSize) {
//  $3=&($2);
//}
//
//%typemap(freearg) (double data[], int maxlen, int *actualSize) {
//   if ($1) free($1);
//}
//
//// Set argument to NULL before any conversion occurs
//%typemap(check)(double data[], int maxlen, int *actualSize) {
//    $2=helicsSubscriptionGetVectorSize(arg1);
//    $1 = (double *) malloc($2*sizeof(double));
//}
//
//%typemap(argout) (double data[], int maxlen, int *actualSize) {
//  int i;
//  PyObject *o2=PyList_New(*$3);
//  for (i = 0; i < *$3; i++) {
//    PyObject *o_item=PyFloat_FromDouble($1[i]);
//      PyList_SetItem(o2, i, o_item);
//      }
//
//  $result = SWIG_Python_AppendOutput($result, o2);
//}
//
//

//// typemap for raw data input
//%typemap(in) (const void *data, int inputDataLength) {
//  if (PyUnicode_Check($input)) {
//    int kind=PyUnicode_KIND($input);
//    $1=PyUnicode_DATA($input);
//    switch(kind)
//    {
//    case PyUnicode_1BYTE_KIND:
//    default:
//        $2=PyUnicode_GetLength($input);
//    break;
//    case PyUnicode_2BYTE_KIND:
//    case PyUnicode_WCHAR_KIND:
//        $2=PyUnicode_GetLength($input)*2;
//    break;
//    case PyUnicode_4BYTE_KIND:
//        $2=PyUnicode_GetLength($input)*4;
//    break;
//    }
//  }
//  else if (PyBytes_Check($input)) {
//    $1=PyBytes_AsString($input);
//    $2=PyBytes_Size($input);
//  }
//  else
//  {
//    PyErr_SetString(PyExc_ValueError,"Expected a string or bytes");
//    return NULL;
//  }
//}
//
//%typemap(argout) (const void *data, int inputDataLength)
//{
//}
//
//
//// typemap for raw data output function
//%typemap(in, numinputs=0) (void *data, int maxDataLength, int *actualSize) {
//  $3=&($2);
//}
//
//%typemap(freearg) (void *data, int maxDataLength, int *actualSize) {
//   if ($1) free($1);
//}
//
//// Set argument to NULL before any conversion occurs
//%typemap(check)(void *data, int maxDataLength, int *actualSize) {
//    $2=helicsSubscriptionGetValueSize(arg1)+2;
//    $1 =  malloc($2);
//}
//
//%typemap(argout) (void *data, int maxDataLength, int *actualSize) {
//  PyObject *o2=PyBytes_FromStringAndSize($1,*$3);
//  $result = SWIG_Python_AppendOutput($result, o2);
//}
