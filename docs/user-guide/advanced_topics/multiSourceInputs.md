# Multi-Source Inputs

On occasion it is useful to allow multiple source to feed an input, creating an _n_-to-one relationship (publications to input). This could occur in situations like a summing junction, or a switch that can be turned on or off from multiple other federates. Alternatively, a multi-source input can be a convenient way to collecting multiple inputs into a vector for processing by the federate. While technically supported prior to [2.5.1](https://github.com/GMLC-TDC/HELICS/releases/tag/v2.5.1) the control and access to this feature of HELICS was not well thought through or straightforward. The developments in 2.5.1 made it possible to specify a mathematical reduce operation on multiple inputs to allow access to them as a single value or vector.

## Mechanics of multi-input handling

Internally, HELICS manages input data in a queue and when a federate is granted a time, the values are read and placed in a holding location by source. In many cases there is likely only to be a single source. But if multiple publications link to a single source the results are placed in a vector. The order of the values in that vector is determined by the order of linking when the federate with the multi-source input is created. If a single publication value is retrieved from the input, the newest value is given as if it were a single source. In case of ties (multiple publishing federates publishing values on the same timestep), the publication that connected first is given priority.

## Controlling the behavior

A few flags are available to control or modify this behavior including limiting the number of connections and adjusting the priority of the different inputs sources. The behavior of inputs is controlled via flags using `helicsInputSetOption()` method.

### The number of connections

There are several flags and handle options which can control this for Inputs

- `helics_handle_option_single_connection_only` : If set to true specified that an input may have only 1 connection
- `helics_handle_option_multiple_connections_allowed`: if set to true then multiple connections are allowed
- `helics_handle_option_connections`: takes an integer number of connections specifying that an input must have N number of connections or an error will be generated.

### Controlling priority

The default priority of the inputs if they are published at the same time and only a single value is retrieved is in order of connection. This may not be desired so a few handle options are available to manipulate it.

- `helics_handle_option_input_priority_location` takes a value specifying the input source index to which it will give priority. If given multiple times it establishes an ordering of the input priority with the most recent call being highest priority. This allows signals that are received at the same time to be prioritized. For example, let's say the option is called first with a given value of "2" then again with a value of "1". If all signals are sent at the same simulated time, the source with index 1 will have highest priority, and in the case of a tie between sources 0 and 2, source 2 will have priority.
- `helics_handle_option_clear_priority_list` will erase the existing priority list.

## Reduction operations on multiple inputs

The priority of the inputs is only applicable if the default operation to retrieve a single value is used. The option
`helics_handle_option_multi_input_handling_method` can be used to specify a reduction operation on all the inputs to process them in some fashion a number of operations are available.

```{eval-rst}
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| method                                    | Description                                                                                                                                  |
+===========================================+==============================================================================================================================================+
| ``helics_multi_input_no_op``              | default operation to pick the highest priority value                                                                                         |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_vectorize_operation``| take all the values and collapse to a single vector, converts strings into a JSON string vector                                              |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
|``helics_multi_input_and_operation``       | treat all inputs as booleans and perform an `and` operation                                                                                  |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_or_operation``       | treat all inputs as boolean and perform an `or` operation                                                                                    |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
|``helics_multi_input_sum_operation``       | sum all the inputs after converting to double vector, except if input type is string then concatenate all inputs as a single string          |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_diff_operation``     | if the input type is specified as a double subtract the sum of remaining values from the first, if it is a vector do a vector diff operation |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_max_operation``      | pick the biggest value                                                                                                                       |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_min_operation``      | pick the smallest value                                                                                                                      |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
| ``helics_multi_input_average_operation``  | take a numerical average of all values                                                                                                       |
+-------------------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
```

The handling method specifies how the reduction operation occurs the value can then retrieved normally via any of the `getValue` methods on an input.

## Configuration

Multi Input handling can be configured through the programming API or through a file based configuration.

**C++**

```cpp
auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::options::multi_input_handling_method,
                  helics::multi_input_handling_method::average);
```

**C**

```c
/*errors are ignored here*/
helics_input in1 = helicsFederateRegisterInput("",helics_data_type_double,"",nullptr);
helicsInputAddTarget(in1,"pub1",nullptr);
helicsInputAddTarget(in1,"pub2",nullptr);
helicsInputAddTarget(in1,"pub2",nullptr);
helicsInputSetOption(in1,helics_handle_option_multi_input_handling_method,helics_multi_input_average_operation, nullptr);

```

**Python**

```python
in1 = h.helicsFederateRegisterInput("", h.helics_data_type_double, "")
h.helicsInputAddTarget(in1, "pub1")
h.helicsInputAddTarget(in1, "pub2")
h.helicsInputAddTarget(in1, "pub2")
h.helicsInputSetOption(
    in1,
    helics_handle_option_multi_input_handling_method,
    helics_multi_input_average_operation,
)
```

The handling can also be configured in the configuration file for the federate

**TOML**

```toml
inputs=[
{key="ipt2",  type="double", targets=["pub1","pub2"], connections=2, multi_input_handling_method="average"}
]
```

**JSON**

```json
"inputs": [
    {
      "key": "ipt2",
      "type": "double",
      "connections":2,
      "multi_input_handling_method":"average",
      "targets": ["pub1","pub2"]
    }
 ]
```

The priority of the inputs in most cases determined by the order of adding the publications as a target. This is not strictly guaranteed to occur but is a general rule and only applies in the default case, and possibly the diff operation.

## Example

An [explanation of a full co-simulation example](../examples/advanced_examples/advanced_multi_input.md) showing how a multi-source input might be used in a federation is provided in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_message_comm/multi_input).
