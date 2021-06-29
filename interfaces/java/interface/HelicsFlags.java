/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public final class HelicsFlags {
  public final static HelicsFlags HELICS_FLAG_SLOW_RESPONDING = new HelicsFlags("HELICS_FLAG_SLOW_RESPONDING", helicsJNI.HELICS_FLAG_SLOW_RESPONDING_get());
  public final static HelicsFlags HELICS_FLAG_DEBUGGING = new HelicsFlags("HELICS_FLAG_DEBUGGING", helicsJNI.HELICS_FLAG_DEBUGGING_get());
  public final static HelicsFlags HELICS_FLAG_TERMINATE_ON_ERROR = new HelicsFlags("HELICS_FLAG_TERMINATE_ON_ERROR", helicsJNI.HELICS_FLAG_TERMINATE_ON_ERROR_get());
  public final static HelicsFlags HELICS_FLAG_FORCE_LOGGING_FLUSH = new HelicsFlags("HELICS_FLAG_FORCE_LOGGING_FLUSH", helicsJNI.HELICS_FLAG_FORCE_LOGGING_FLUSH_get());
  public final static HelicsFlags HELICS_FLAG_DUMPLOG = new HelicsFlags("HELICS_FLAG_DUMPLOG", helicsJNI.HELICS_FLAG_DUMPLOG_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static HelicsFlags swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + HelicsFlags.class + " with value " + swigValue);
  }

  private HelicsFlags(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private HelicsFlags(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private HelicsFlags(String swigName, HelicsFlags swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static HelicsFlags[] swigValues = { HELICS_FLAG_SLOW_RESPONDING, HELICS_FLAG_DEBUGGING, HELICS_FLAG_TERMINATE_ON_ERROR, HELICS_FLAG_FORCE_LOGGING_FLUSH, HELICS_FLAG_DUMPLOG };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

