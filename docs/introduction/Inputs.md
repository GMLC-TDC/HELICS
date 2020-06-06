# Inputs

Inputs are part of a matching pair with [Publications](./Publications). They are the input side of a federate for data exchange.

The definition consists of a name, a type, and a unit.

The name of an Input is the identifier used for publishers to send data to it. The name can be left empty to generate a nameless subscription that is not addressable from outside the creating federate. There are some wrapper functions that generate a subsciption. This is simply a wrapper for a nameless input and an `addTarget` call to link to a publication.

Named Inputs can be global or local. The only difference is that local Inputs have the name prepended by the federate name so the global name would be in the form "federateName/inputName" whereas a global input would just have "inputName"

The type of input is represented as an open string but the Application API recognizes several well supported types, including:

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

Inputs that just target a single publication (or any for that matter) can leave the type empty and it will take on the type of the publication.
These are all convertible and known to the Application API. The data can be retrieved as any of these types though some are lossy. There are API functions to query the type of the input and the type of the publication that sends it data.

Inputs can add a target which is a [Publication](./Publications). An input can be targeted by multiple publications though the interface for dealing with this is not well developed and will be undergoing development in the coming revisions, currently the latest update in time from any publication is used as the value. Other options will be available in the future.
