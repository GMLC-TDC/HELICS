/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public final class HelicsSequencingModes {
  public final static HelicsSequencingModes HELICS_SEQUENCING_MODE_FAST = new HelicsSequencingModes("HELICS_SEQUENCING_MODE_FAST", helicsJNI.HELICS_SEQUENCING_MODE_FAST_get());
  public final static HelicsSequencingModes HELICS_SEQUENCING_MODE_ORDERED = new HelicsSequencingModes("HELICS_SEQUENCING_MODE_ORDERED", helicsJNI.HELICS_SEQUENCING_MODE_ORDERED_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static HelicsSequencingModes swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + HelicsSequencingModes.class + " with value " + swigValue);
  }

  private HelicsSequencingModes(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private HelicsSequencingModes(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private HelicsSequencingModes(String swigName, HelicsSequencingModes swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static HelicsSequencingModes[] swigValues = { HELICS_SEQUENCING_MODE_FAST, HELICS_SEQUENCING_MODE_ORDERED };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}
