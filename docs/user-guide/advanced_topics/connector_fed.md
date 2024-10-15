# Connector Federate

For some co-simulation workflows or software architectures, it is convenient for the mapping of HELICS interfaces (publications, inputs, and endpoints) between federates to be managed outside the federate configuration. For example, federates could be configured to expose all their interfaces and allow another software entity to actually make the mapping between the interfaces using a definition that can change based on the use case. This allows the federates to remain untouched and effectively be use-case independent (at least as far as their interfaces go) while still having their connections in the federation adjusted as a given use case requires.

To support this HELICS comes with a connector application called "helics_connector" that uses a "match-file" to create the connections between federates.

## "helics_connector" Federate

The role of the "helics_connector" federate is to evaluate a mapping of the connections between federate interfaces and create the connections between these interfaces. Under the hood, it's using public APIs that allow the sources ("targets") for inputs and endpoints to be defined and destinations ("targets") to be added to publications. (Which is to say, if the stock "helics_connector" federate doesn't work the way you would like, you could actually make your own.) The documentation of the "helics_connector" app can be found on its [dedicated documentation page](TODO) but I'll summarize the highlights here:

- Matches are defined in a plain text or JSON structured text file.
- Matches can be defined as one-to-one connections or use regular expressions to match many interfaces in one succinct line.
- Tags can be used to add an extra dimension to the matching process, only allowing federates with certain tags to be have their interfaces be candidates for the matching process.
- If federates support it, the connector can query a federate about the interfaces it could make and then, based on the results of the matching operation, command said federate to create the necessary interfaces.

## Callback Complications

The "query-and-command" form of operation is the most complex, largely because it requires the implementation of callback functions. Unless you're using the C API directly, implementing the callback functions takes a little bit of extra effort to get the C-based libraries under the hood working. Since most of the User Guide examples are in Python and most of our users are using Python, we've [documented callbacks in PyHELICS](TODO). Take a look at that page for further details to get a slightly better understanding why C-based callbacks are more complicated in Python.

## "helics_connector" Interface creation

The order in which "helics*connector" creates the interfaces in the federate is not deterministic. This is especially important when using the APIs that get the interface by index (\_e.g* `helicsFederateGetInputByIndex)`, `helicsFederateGetPublicationByIndex`, `helicsFederateGetEndpointByIndex`, `helicsFederateGetEndpointByIndex`). There is no guarantee that the order the interfaces are added to the federate will be the same from computer to computer. To continue using this API

## Examples

Three examples have been created to demonstrate the use of the "helics_connector" federate: one uses a direct match-file, one uses a regex match-file and the the last uses a direct match-file while using the query-and-command method of interface creation. Links to the examples are provided below; all produce identical results.

An example utilizing both kinds of iteration is available [here](../examples/advanced_examples/advanced_iteration.md).
