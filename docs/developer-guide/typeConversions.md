# HELICS Type Conversions

HELICS has the ability to convert data between different types. In the C api methods are available to send and receive data as strings, integers, doubles, boolean, times, char, complex, vector of doubles, and named points.

## Available types

The specification of publication allows setting the publication to one of following enum values:

```c++
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

When this data is received it can be received as any of the available types, thus there are no restrictions on which function is used based on the data that was sent. That being said not all conversions are lossless. The following image shows which conversions are lossless through round trip operations. On the sending side the same is also true in that any of the setValue methods may be used regardless of what the actual transmission data type is set to.

![Lossless type conversion](../img/LossLessConversion_chart.svg)

For the remaining conversions the loss is typically numerical or comes with conditions. For example converting a vector to a complex number you can think of a complex number as a two element vector. Thus for 1 or 2 element vectors the conversion will be lossless. For 3 or more element vectors the remaining elements are discarded. For conversion of any vector to a single value, double, or integral the value is collapsed using vector normalization. Thus for single element vectors or real valued complex numbers the result is equivalent, whereas for others there is a reduction in information, which may be desired in some cases. Bool values get reduced to `false` if the numerical value is anything but `0`, and true otherwise.

Conversion from strings to numeric values assume the string encodes a number, otherwise it results in an invalid value.

### Invalid Values

Invalid values are returned if there is no value or the conversion is invalid for some reason, typically string to numeric value failures, in most cases an empty value equivalent is returned.

- INT -> -9,223,372,036,854,775,808 ( `numeric_limits<std::int64_t>::min()`)
- DOUBLE -> `-1e49`
- COMPLEX -> {-1e49,0.0}
- VECTOR -> {}
- COMPLEX_VECTOR -> {}
- STRING -> ""
- NAMED_POINT -> {"", NaN}
- BOOL -> false
- TIME -> Time::minVal() = `Time(numeric_limits<std::int64_t>::min())`

## How types are used in HELICS

### Interface specification

For Value based interfaces types are used in a number of locations. For publications the most important part is specifying the type of the publication. This determines the type of data that gets transmitted. It can be a specific known type, a custom type string that is user defined, or an `any` type.

Inputs or subscriptions may also optionally specify a type as well. The use of this is for information to other federates, and for some type compatibility checking. It defaults to an "any" type so if nothing is specified it will match to any publication type.

### Data publication and extraction

As mentioned before when using any of the `publication` methods the extraction method call is not limited based on the type given in the interface specification. What the publication type does is define a conversion if necessary. The same is true on the output.

SetValueType -> PublicationType -> GetValueType

Thus there are always two conversions occurring in the data translation pipeline. The first from the setValue to the type specified in the publication registration. The second is from the publish transmission type to the value requested in a respective getValue method.

## Data Representation

`int` and `double` are base level types in C++. `complex` is two doubles, `vector` and `complex_vector` are variable length arrays of doubles. `named_point` is `string` and double. String is a variable length sequence of 8 bit values. `char` is a string of length 1, `boolean` is a single char either `0` or `1`. Time is a representation of internal HELICS time as a 64 bit integer (most commonly number of nanoseconds). The JSON type is a string compatible with JSON with two fields "type" and "value". The custom type is a variable length sequence of bytes which HELICS will not attempt to convert, it transmits a sequence of bytes and it would be up to the user to define any conversions or endianness concerns.

## Data conversions

There are defined conversions from all known available types to all others.

### Conversion from Double

- INT -> trunc(val)
- DOUBLE -> val
- COMPLEX -> val+0j
- VECTOR -> [val]
- COMPLEX_VECTOR -> [val+0j]
- NAMED_POINT -> {"value", val}
- STRING -> string representation such that all required bits are included
- BOOL -> (val!=0)?"1":"0"

### Conversion from INT

- INT -> val
- DOUBLE -> val [^1]
- COMPLEX -> val+0j
- VECTOR -> [val]
- COMPLEX_VECTOR -> [val+0j]
- NAMED_POINT -> {"value", val} [^2]
- STRING -> std::to_string(val)
- BOOL -> (val!=0)?"1":"0"

[^1]: conversion to double is lossless only if the value actually fits in a double mantissa value.

[^2]: for a named point conversion, if the value doesn't fit in double the string translation is placed in the string field and a NaN value in the value segment to ensure no data loss.

### Conversion from String

- INT -> `getIntFromString(val)`
- DOUBLE -> `getDoubleFromString(val)`
- COMPLEX -> `getComplexFromString(val)`
- VECTOR -> `helicsGetVector(val)`
- COMPLEX_VECTOR -> `helicsGetComplexVector(val)`
- NAMED_POINT -> {val, NaN}
- STRING -> val
- BOOL -> (helicsBoolValue(val))?"1":"0"

#### helicsGetComplexVector

This method will read a vector of numbers from a string in either JSON format, or `c[X1,X2,...XN]` where XN is a complex number of the format R+Ij.
It can also interpret `v[X1,X2, ..., XN]` in which case the values are assumed to be alternating real and imaginary values. If the string is a single value either real or complex it is placed in a vector of length 1.

#### helicsGetVector

This function is similar to `helicsGetComplexVector` with distinction that string like `v[X1,X2, ..., XN]` are all assumed separate values, and complex vectors are generated as alternating real and imaginary values.

#### getIntFromString

Converts a string into 64 bit integer. If the string has properties of a double or vector it will truncate the values from `getDoubleFromString`.

#### getDoubleFromString

Converts a string into a double value, a complex, or a vector of real or complex numbers. If the vector is more than a single element the output is the vector norm of the vector. If the string is not convertible the invalid_double is returned (-1e49).

#### helicsGetComplexFromString

Similar to getDoubleFromString in conversion of vectors. It will convert most representations of complex number patterns using a trailing `i` or `j` for the imaginary component and assumes the imaginary component is last. The real component can be omitted if not present, for example `4.7+2.7j`or`99.453i`

#### helicsBoolValue

"0", "00","0000", "\0","false","f","F","FALSE","N","no","n","OFF","off","","disable","disabled" all return false, everything else returns true.

### Conversion from vector double

- INT -> trunc(vectorNorm(val)) [^3]
- DOUBLE -> vectorNorm(val)
- COMPLEX -> val[0]+val[1]j
- VECTOR -> val
- COMPLEX_VECTOR -> [val[0],val[1],...,val[N]]
- STRING -> vectorString(val) [^4]
- NAMED_POINT -> {vectorString(val), NaN} [^5]
- BOOL -> (vectorNorm(val)!=0)?"1":"0"

[^3]: vectorNorm is the sqrt of the inner product of the vector.

[^4]: vectorString is comma-separated string of the numerical values enclosed in `[]`, for example `[45.7,22.7,17.8]`. This is a JSON compatible string format.

[^5]: if the vector is a single element the NAMED_POINT translation is equivalent to a double translation.

### Conversion from Complex Vector

- INT -> trunc(vectorNorm(val))
- DOUBLE -> vectorNorm(val)
- COMPLEX -> val[0]
- VECTOR -> [abs(val[0]),abs(val[1]),...abs(val[N])][^6]
- COMPLEX_VECTOR -> val
- NAMED_POINT -> {vectorString(val), NaN}
- STRING -> complexVectorString(val)
- BOOL -> (vectorNorm(val)!=0)?"1":"0"

See [Conversion from vector double](#conversion-from-vector-double) for definitions of vectorNorm.
[^6]: if the imaginary part of these values is 0, then the real part is used; otherwise it uses the absolute value.

### Conversion from Complex

- INT -> trunc((val.imag() == 0)?val.real(): std::abs(val))
- DOUBLE -> val.imag() == 0)?val.real(): std::abs(val)
- COMPLEX -> val
- VECTOR -> [val.real,val.imag]
- COMPLEX_VECTOR -> [val]
- NAMED_POINT -> {helicsComplexString(val), NaN}
- STRING -> helicsComplexString(val)
- BOOL -> (std::abs(val)!=0)?"1":"0"

If the imaginary value == 0, the value is treated the same as a double.

### Conversion from a Named Point

If the value of the named point is `NaN` then treat the name part the same as a string,
otherwise use the numerical value as a double and convert appropriately. The exception is a string which has a dedicated operation to generate a JSON string with two fields {"name" and "value"}.

### Conversion from Bool

- INT -> (val)?1:0
- DOUBLE -> (val)?1.0:0.0
- COMPLEX -> (val)?1.0:0.0 + 0.0j
- VECTOR -> [(val)?1.0:0.0]
- COMPLEX_VECTOR -> [(val)?1.0:0.0 +0.0j]
- NAMED_POINT -> {"value",(val)?1.0:0.0}
- STRING -> val?"1":"0";
- BOOL -> val?"1":"0";

### Conversion from Time

Time is transmitted as a 64 bit integer so conversion rules of an integer apply with the note that a double as a time is assumed as seconds and the integer represents nanoseconds so a double (as long as not too big) will be transmitted without loss as long as the precision is 9 decimal digits or less.

## Unit conversions

HELICS also handles unit conversions if units are specified on the publication and subscription and can be understood by the units library. This applies primarily for pub/sub of numerical types.
HELICS uses [Units](https://github.com/LLNL/units) as the units library.
