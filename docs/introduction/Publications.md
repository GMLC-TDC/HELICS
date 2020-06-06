# Publications

Publications are interfaces that send data out of a federate. They are defined through a Value Federate in the Application API.

The definition consists of a name, a type, and a unit.

The name of a publication is the identifier used for subscribers to link to it. The name can be left empty to generate a nameless publication that is not accessible from outside the creating federate. It can still send data out but all links must be specified from inside the creating federate.

Publications can be global or local. The only difference is that local Publications have the name prepended by the federate name so the global name would be like "federateName/publicationName" whereas a global publication would just have "publicationName"

The type of publication is represented as an open string but the Application API recognizes several well supported types, including:

- int64
- double
- string
- vector (a series of doubles)
- complex (a pair of doubles)
- vector_complex (a series of complex numbers)
- char
- bool
- time (a HELICS time value)
- named point (a value with a string and a double)

These are all convertible and known to the Application API but some conversions are lossy, so there is a flag to allow only string matching if that is required.

Publication can add a target which is an [Input](./Inputs). A publication can target as many inputs as desired.
