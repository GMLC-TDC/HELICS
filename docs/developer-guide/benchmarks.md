# HELICS Benchmarks

The HELICS repository has a few benchmarks that are intendended to test various aspects of the code and record performance over time

## Baseline benchmarks

### ActionMessage
    microbenchmarks to test some operations concerning the serialization of the underlying message Structure in HELICS
    
### Conversion
    microbenchmarks to test the serialization and deserialization of common data types in HELICS
    
## Simulation benchmarks

### Echo
    a set of federates representing a hub and spoke model of communication for value based interfaces
    
### Echo Message
    a set of federates representing a hub and spoke model of communications for message based interfaces
    
### Filter
    a variant of the Echo message test that add filters to the messages
    
### ring Benchmark 
    a ring like structure that passes a value token around a bunch of times
    
### timing Benchmark
    similar to echo but doesn't actually send any data just pure test of the timing messages
    
## Message Benchmarks

benchmarks testing various aspects of the messaging structure in HELICS

### MessageLookup
Benchmarks sends messages to random federates, varying the total number of interfaces and federates.

### MessageSend
sending messages between 2 federates varying the message size and count

## Standardized Tests

### PHold
    a standard PHOLD benchmark varying the number of federates.  