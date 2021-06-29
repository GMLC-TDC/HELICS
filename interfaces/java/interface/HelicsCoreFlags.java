/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public final class HelicsCoreFlags {
  public final static HelicsCoreFlags HELICS_FLAG_DELAY_INIT_ENTRY = new HelicsCoreFlags("HELICS_FLAG_DELAY_INIT_ENTRY", helicsJNI.HELICS_FLAG_DELAY_INIT_ENTRY_get());
  public final static HelicsCoreFlags HELICS_FLAG_ENABLE_INIT_ENTRY = new HelicsCoreFlags("HELICS_FLAG_ENABLE_INIT_ENTRY", helicsJNI.HELICS_FLAG_ENABLE_INIT_ENTRY_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static HelicsCoreFlags swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + HelicsCoreFlags.class + " with value " + swigValue);
  }

  private HelicsCoreFlags(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private HelicsCoreFlags(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private HelicsCoreFlags(String swigName, HelicsCoreFlags swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static HelicsCoreFlags[] swigValues = { HELICS_FLAG_DELAY_INIT_ENTRY, HELICS_FLAG_ENABLE_INIT_ENTRY };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

