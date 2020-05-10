# Filters

Filters are interfaces which can modify messages including routes, destinations, times, existence, and payloads.
This is useful for inserting communication modules into a data stream as an optional component they can also be used to clone messages and randomly drop them.

## Filter Creation

Filters are registered with the core or through the application API.
There are also Filter object that hide some of the API calls in a slightly nicer interface.
Generally a filter will define a target endpoint as either a source filter or destination filter.
Source filters can be chained, as in there can be more than one of them.
At present there can only be a single non-cloning destination filter attached to an endpoint.

Non-cloning filters can modify the message in some way, cloning filters just copy the message and may send it to multiple destinations.

On creation, filters have a target endpoint and an optional name.
Custom filters may have input and output types associated with them.
This is used for chaining and automatic ordering of filters.
Filters do not have to be defined on the same core as the endpoint, and in fact can be anywhere in the federation, any messages will be automatically routed appropriately.

## predefined filters

Several predefined filters are available, these are parameterized so they can be tailored to suite the simulation needs

### reroute

This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if some patterns are matching the patterns should be specified by "condition" in the set string the conditions are regular expression pattern matching strings

### delay

This filter will delay a message by a certain amount

### randomdelay

This filter will randomly delay a message according to specified random distribution
available options include distribution selection, and 2 parameters for the distribution
some distributions only take one parameter in which case the second is ignored. The distributions available are based on those available in the C++ [<random\>](http://www.cplusplus.com/reference/random/) library

- constant - param1="value" this just generates a constant value
- [uniform](http://www.cplusplus.com/reference/random/uniform_real_distribution/) param1="min", param2="max"
- [bernoulli](http://www.cplusplus.com/reference/random/bernoulli_distribution/)
  param1="prob", param2="value" the bernoulli distribution will return param2 if the bernoulli trial returns true, 0.0 otherwise. Param1 is the probability of returning param2
- [binomial](http://www.cplusplus.com/reference/random/binomial_distribution/) param1=t (cast to int) param2="p"
- [geometric](http://www.cplusplus.com/reference/random/geometric_distribution/)
  param 1="prob" the output is param2\*geom(param1) so multiplies the integer output of the geometric distribution by param2 to get discrete chunks
- [poisson](http://www.cplusplus.com/reference/random/poisson_distribution/) param1="mean"
- [exponential](http://www.cplusplus.com/reference/random/exponential_distribution/) param1="lambda"
- [gamma](http://www.cplusplus.com/reference/random/gamma_distribution/) param1="alpha" param2="beta"
- [weibull](http://www.cplusplus.com/reference/random/weibull_distribution/) param1="a" param2="b"
- [extreme_value](http://www.cplusplus.com/reference/random/extreme_value_distribution/) param1="a" param2="b"
- [normal](http://www.cplusplus.com/reference/random/normal_distribution/)
  param1="mean", param2="stddev"
- [lognormal](http://www.cplusplus.com/reference/random/lognormal_distribution/) param1="mean", param2="stddev"
- [chi_squared](http://www.cplusplus.com/reference/random/chi_squared_distribution/)
  param1="n"
- [cauchy](http://www.cplusplus.com/reference/random/cauchy_distribution/) param1="a" param2="b"
- [fisher_f](http://www.cplusplus.com/reference/random/fisher_f_distribution/)
  param1="m" param2="n"
- [student_t](http://www.cplusplus.com/reference/random/student_t_distribution/) param1="n"

### randomdrop

This filter will randomly drop a message, the drop probability is specified, and is modeled as a uniform distribution.

### clone

this message will copy a message and send it to the original destination plus a new one.

### firewall

The firewall filter will eventually be able to execute firewall like rules on messages and perform certain actions on them, that can set flags, or drop or reroute the message. The nature of this is still in development and will be available at a later release.

### custom filters

Custom filters are allowed as well, these require a callback operator that can be called from any thread
and modify the message in some way.
