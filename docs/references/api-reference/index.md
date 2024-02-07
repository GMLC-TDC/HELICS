# API Reference

```{eval-rst}
.. toctree::
    :maxdepth: 1

    C_API
    CPP
    rest_queries_api

```

## [C API](./C_API)

## [Python API](https://python.helics.org/api/)

## [matHELICS API](./C_API)

There's no dedicated documentation for the matHELICS API at this time as the API is virtually identical to the other higher level languages such as [Python](https://python.helics.org/api/) and [Julia](https://julia.helics.org/stable/). Of particular note, as in those two languages, matHELICS supports the use of complex as a native data type. So `helicsInputGetComplex()` returns a complex value not a list of two floats, one for the real and one for the imaginary.

There is also inline documentation in the [source code files](https://github.com/GMLC-TDC/matHELICS/tree/main/matlabBindings/%2Bhelics) that may be helpful in providing insight.

## [Julia API](https://gmlc-tdc.github.io/HELICS.jl/latest/api/)

## [REST Queries API](https://docs.helics.org/en/latest/references/api-reference/rest_queries_api.html)
