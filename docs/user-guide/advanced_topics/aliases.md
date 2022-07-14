# Aliases

Aliasing in HELICS allows an interface (publication, input, endpoint, filter, translator) to be linked via a different name than the one given on registration.
This can be used to simplify commonly used names or interfaces, or match up federates with different naming conventions. Aliases can be defined from anywhere in the co-simulation (like connections), including via config files.

## API

Cores, brokers, and Federates all have a single API call

```C++
addAlias(std::string_view interfaceName, std::string_view alias);
```

In the C and language API's the call will have a form such as:

```c
helicsXXXAddAlias(const char * interfaceName, const char * alias);
```

## Rules

The `addAlias` operation creates an equivalence relationship between the two strings.
As such `addAlias("string1","string2")` is equivalent to `addAlias("string2","string1")` in terms of the overall co-simulation connectivity.
That being said there is a small performance difference between the two and it generally is recommended that the first string be an existing interface name, and the underlying interface be created before the alias. Once again, everything will work in other orders but there is a performance difference.

In general, all operations are allowed unless they create a situation which would map a single string to multiple interfaces of the same type, if that situation occurs a global error is generated and the co-simulation fails.

Multiple aliases are allowed including aliases of aliases.

## File Configuration

In addition to the API calls, aliases can be specified in federation and connection configuration files in either TOML or JSON. The keyword is "aliases" followed by an array of string pairs, which is the exact same structure as specifying "globals"

```TOML
 aliases=[["comboFed/pub2","dpub"],["cfed_agasagadsag","c2"]]
```

```JSON
"aliases": [
    ["comboFed/pub2", "dpub"],
    ["cfed_agasagadsag", "c2"]
]
```
