# Style Guide

The goal of the style guide is to describe in detail naming conventions for
developing HELICS. Style conventions are encapsulated in the .clang_format
files in the project.

We have an EditorConfig file that has basic formatting rules code editors and
IDEs can use. See [https://editorconfig.org/](https://editorconfig.org/#download)
for how to setup support in your preferred editor or IDE.

## Naming Conventions

1. All functions should be `camelCase`

   ```cpp
   PublicationID registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "");
   ```

   EXCEPTION: when the functionality matches a function defined in the standard library e.g. to_string()

2. All classes should be `PascalCase`

   ```cpp
   class ValueFederate : public virtual Federate
   {
   public:
       ValueFederate (const FederateInfo &fedInfo);
   }
   ```

3. class methods should be `camelCase`

   ```cpp
   Publication &registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "");
   ```

   Exceptions: functions that match standard library functions e.g. to_string()

4. Enumeration names should be `PascalCase`. Enumeration values should be CAPITAL_SNAKE_CASE

   ```cpp
   /* Type definitions */
   typedef enum {
       HELICS_OK,
       HELICS_DISCARD,
       HELICS_WARNING,
       HELICS_ERROR,
   } HelicsStatus;

   ```

5. Constants and macros should CAPITAL_SNAKE_CASE

6. Variable names:
   local variable names should be `camelCase`
   member variable names should be `mPascalCase`
   static const members should be `CAPITAL_SNAKE_CASE`
   function input names should be `camelCase`
   index variables can be `camelCase` or `ii,jj,kk,mm, nn` or similar if appropriate
   global variables should `gPascalCase`

7. All C++ functions and types should be contained in the helics
   namespace with subnamespaces used as appropriate

   ```cpp
   namespace helics
   {
       ...
   } // namespace helics
   ```

8. C interface functions should begin with helicsXXXX

   ```cpp
   HelicsBool helicsBrokerIsConnected (HelicsBroker broker);
   ```

9. C interface function should be of the format helics{Class}{Action}
   or helics{Action} if no class is appropriate

   ```cpp
   HelicsBool helicsBrokerIsConnected (HelicsBroker broker);

   const char *helicsGetVersion ();
   ```

10. All cmake commands (those defined in cmake itself) should be lower case

    ```text
    if as opposed to IF
    install vs INSTALL
    ```

11. Public interface functions should be documented consistent with Doxygen style comments
    non public ones should be documented as well with doxygen but we are a ways from that goal

    ```cpp
    /** get an identifier for the core
        @param core the core to query
        @return a string with the identifier of the core
    */
    HELICS_EXPORT const char *helicsCoreGetIdentifier (HelicsCore core);
    ```

12. File names should match class names if possible

13. All user-facing options (_e.g._ `log_level`) should be expressed to the user as a single word or phrase using `nocase`, `camelCase`, and `snake_case`. Do not use more than one synonymous term for the same option; that is, do not define a single option that is expressed to the user as both `best_ice_cream_flavor` and `most_popular_ice_cream_flavor`.

Exception: when defining the command line options, the command-line parser already handles underscores and casing so there is no need to define all three cases for that parser.
