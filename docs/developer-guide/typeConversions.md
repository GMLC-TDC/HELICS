# HELICS Type Conversions

HELICS has the ability to convert data between different types.  In the C api methods are available to send and receive data as strings, integers, doubles, boolean, times, char, complex, vector of doubles, and named points.  

## Available types

The specification of publication allows setting the publication to one of following enum values


``` c++
    HELICS_DATA_TYPE_UNKNOWN = -1,
    /** a sequence of characters*/
    HELICS_DATA_TYPE_STRING = 0,
    /** a double precision floating point number*/
    HELICS_DATA_TYPE_DOUBLE = 1,
    /** a 64 bit integer*/
    HELICS_DATA_TYPE_INT = 2,
    /** a pair of doubles representing a complex number*/
    HELICS_DATA_TYPE_COMPLEX = 3,
    /** an array of doubles*/
    HELICS_DATA_TYPE_VECTOR = 4,
    /** a complex vector object*/
    HELICS_DATA_TYPE_COMPLEX_VECTOR = 5,
    /** a named point consisting of a string and a double*/
    HELICS_DATA_TYPE_NAMED_POINT = 6,
    /** a boolean data type*/
    HELICS_DATA_TYPE_BOOLEAN = 7,
    /** time data type*/
    HELICS_DATA_TYPE_TIME = 8,
    /** raw data type*/
    HELICS_DATA_TYPE_RAW = 25,
    /** type converts to a valid json string*/
    HELICS_DATA_TYPE_JSON = 30,
    /** the data type can change*/
    HELICS_DATA_TYPE_MULTI = 33,
    /** open type that can be anything*/
    HELICS_DATA_TYPE_ANY = 25262
```

When this data is received it can be received as any of the available types, thus there are no restrictions on which function is used based on the data that was sent.  That being said not all conversions are lossless.  The following image shows which conversions are lossless through round trip operations.  On the sending side the same is also true in that in any of the setValue methods may be used regardless of which the actual transmission data type is set to.

![Lossless type conversion](../img/LossLessConversion_chart.svg)

## How Types are use in HELICS

### Interface specification

For Value based interfaces types are used in a number of locations.  For publications the most important part is specifying the type of the publication.  This determines the type of data that gets transmitted.  It can be a specific known type,  a custom type string that is user defined or an `any` type.  

Inputs or subscriptions may also optionally specify a type as well.  The use of this is for information to other federates, and for some type compatibility checking.  It defaults to an "any" type so if nothing is specified it will match to any publication type.  

### Data publication and extraction

As mentioned before when using any of the `publication` methods this is not limited based on the type given in the interface specification.  What is does is define a conversion if necessary.  The same is true on the output.  

SetVelueType ->  PublicationType -> GetValueType

Thus there are always two conversions occurring in the data translation pipeline.  The first from the setValue to the type specified in the publication registration.  The second is from the publish transmission type to the value requested in a respective getValue method.

## Data Representation

`int` and `double` are base level types in C++.  `complex` is two doubles,  `vector` and `complex_vector` are variable length arrays of doubles.  `named_point` is `string` and double.  String is a variable length sequence of 8 bit values.  `char` is a string of length 1, `boolean` is a single char either `0` or `1`.  Time is representation of internal HELICS time as a 64 bit integer (most commonly number of nanoseconds).  The JSON type is a string compatible with JSON with two fields "type" and "value" The custom type is variable length sequence of bytes which HELICS will not attempt to convert.  

## Data conversions

There are defined conversions from all known available types to all others.  

### Conversion from Double

 - INT  -> round(val)
 - DOUBLE -> val
 - COMPLEX -> val+0j
 - VECTOR - > [val]
 - COMPLEX_VECTOR -> [val+0j]
 - NAMED_POINT ->{"value", val}
 - STRING -> std::to_string(val)
 - BOOL ->(val!=0)?"1":"0"

### Conversion from INT

  - INT  -> val
  - DOUBLE -> val (*)
  - COMPLEX -> val+0j
  - VECTOR - > [val]
  - COMPLEX_VECTOR -> [val+0j]
  - NAMED_POINT ->{"value", val} (*)
  - STRING -> std::to_string(val)
  - BOOL ->(val!=0)?"1":"0"

for conversion to double lossless only if value actually fits in a double matissa value.  For a named point conversion.  If the value doesn't fit in double the string translation is placed in the string field and a NAN value in the value segment.  

### Conversion from String

  - INT  -> getDoubleFromString(val)
  - DOUBLE -> round(getDoubleFromString(val))
  - COMPLEX -> getComplexFromString(val)
  - VECTOR - > helicsGetVector(val)
  - COMPLEX_VECTOR -> helicsGetComplexVector(val)
  - NAMED_POINT ->{val, NAN}
  - STRING -> val
  - BOOL ->(helicsBoolValue(val))?"1":"0"

#### helicsGetComplexVector

This method will read a vector of numbers from a string in either JSON format, or `c[X1,X2,...XN]` where XN is a complex number of the format R+Ij.  
It can also interpret `v[X1,X2, ..., XN]` in which case the values are assumed to be alternating real and imaginary values.  If the string is a single value either real or complex it is placed in a vector of length 1.  

#### helicsGetVector

This function is largely simiarly in capabilites to helicsGetComplexVector with distinction that string like `v[X1,X2, ..., XN]` are all assumed separate values, and complex vectors are generated as alternating real and imaginary values.  

#### getDoubleFromString

Converts a string into a single double, a complex, or a vector of real or complex numbers.  If the vector is more than a single element the output is the vector norm of the vector.  If the string is not convertable the invalid_double is returned  (-1e49).

#### helicsGetComplexFromString
Similar to getDoubleFromString in conversion of vectors.  It will convert most representations of complex number patterns using a trailing `[ij]`` for the imaginary component and assumes the imaginary component is last.  The real component can be omitted if not present.  For Example `4.7+2.7j` or `99.453i`

#### helicsBoolValue

"0", "00","0000", "\0","false","f","F","FALSE","N","no","n","OFF","off","","disable","disabled" all return false everything else return true

### Conversion from vector double

  - INT  -> trunc(vectorNorm(val))
  - DOUBLE -> vectorNorm(val)
  - COMPLEX -> val[0]+val[1]j
  - VECTOR - > val
  - COMPLEX_VECTOR -> [val[0]+val[1]j,...,val[2*N]+val[2N+1]j]
  - NAMED_POINT ->{vectorString(val), NAN}
  - STRING -> vectorString(val)
  - BOOL ->(vectorNorm(val)!=0)?"1":"0"

  vector norm is the sqrt of the inner product of the vector.

  vectorString is comma separated string of the numerical values enclosed in `[]` for example `[45.7,22.7,17.8]`  this is a JSON compatible string format.

  if the vector is a single element the NAMED_POINT translation is equivalent to a double translation.

### Conversion from Complex Vector

- INT  -> trunc(vectorNorm(val)) *
- DOUBLE -> vectorNorm(val) *
- COMPLEX -> val[0]
- VECTOR - > [val[0].real,val[0].imag,...val[N].real,val[N].imag]
- COMPLEX_VECTOR -> val
- NAMED_POINT ->{vectorString(val), NAN}
- STRING -> complexVectorString(val)
- BOOL ->(vectorNorm(val)!=0)?"1":"0"

### Conversion from Complex

- INT  -> trunc((val.imag() == 0)?val.real(): std::abs(val))
- DOUBLE -> vectorNorm((val.imag() == 0)?val.real(): std::abs(val))
- COMPLEX -> val
- VECTOR - > [val[0].real,val[0].imag,...val[N].real,val[N].imag]
- COMPLEX_VECTOR -> val
- NAMED_POINT ->{helicsComplexString(val), NAN} *
- STRING -> helicsComplexString(val)
- BOOL ->(std::abs(val)!=0)?"1":"0"

If the imaginary value == 0, the value is treated the same as a double

### Conversion from a NamedPoint

if the value of the named point is a Nan treat it the same as a string
otherwise use the numerical value as a double and convert appropriately.  the exception is a string which has a dedicated operation to generate a JSON string with two fields {"name" and "value"}

### Conversion from Bool

- INT  -> (val)?1:0
- DOUBLE -> (val)?1.0:0.0
- COMPLEX -> (val)?1.0:0.0 + 0.0j
- VECTOR - > [(val)?1.0:0.0]
- COMPLEX_VECTOR -> [(val)?1.0:0.0 +0.0j]
- NAMED_POINT ->{"value",(val)?1.0:0.0}
- STRING ->val?"1":"0";
- BOOL ->val?"1":"0";

### Conversion from Time

Time is transmitted as a 64 bit integer so conversion rules of an integer apply with the note that a double as a time is assumed as seconds and the integer represents nanoseconds.
