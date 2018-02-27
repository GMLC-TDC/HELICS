/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


public final class helics_iteration_request {
  public final static helics_iteration_request no_iteration = new helics_iteration_request("no_iteration");
  public final static helics_iteration_request force_iteration = new helics_iteration_request("force_iteration");
  public final static helics_iteration_request iterate_if_needed = new helics_iteration_request("iterate_if_needed");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static helics_iteration_request swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + helics_iteration_request.class + " with value " + swigValue);
  }

  private helics_iteration_request(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private helics_iteration_request(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private helics_iteration_request(String swigName, helics_iteration_request swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static helics_iteration_request[] swigValues = { no_iteration, force_iteration, iterate_if_needed };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

