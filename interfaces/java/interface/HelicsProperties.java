/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public final class HelicsProperties {
  public final static HelicsProperties HELICS_PROPERTY_TIME_DELTA = new HelicsProperties("HELICS_PROPERTY_TIME_DELTA", helicsJNI.HELICS_PROPERTY_TIME_DELTA_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_PERIOD = new HelicsProperties("HELICS_PROPERTY_TIME_PERIOD", helicsJNI.HELICS_PROPERTY_TIME_PERIOD_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_OFFSET = new HelicsProperties("HELICS_PROPERTY_TIME_OFFSET", helicsJNI.HELICS_PROPERTY_TIME_OFFSET_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_RT_LAG = new HelicsProperties("HELICS_PROPERTY_TIME_RT_LAG", helicsJNI.HELICS_PROPERTY_TIME_RT_LAG_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_RT_LEAD = new HelicsProperties("HELICS_PROPERTY_TIME_RT_LEAD", helicsJNI.HELICS_PROPERTY_TIME_RT_LEAD_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_RT_TOLERANCE = new HelicsProperties("HELICS_PROPERTY_TIME_RT_TOLERANCE", helicsJNI.HELICS_PROPERTY_TIME_RT_TOLERANCE_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_INPUT_DELAY = new HelicsProperties("HELICS_PROPERTY_TIME_INPUT_DELAY", helicsJNI.HELICS_PROPERTY_TIME_INPUT_DELAY_get());
  public final static HelicsProperties HELICS_PROPERTY_TIME_OUTPUT_DELAY = new HelicsProperties("HELICS_PROPERTY_TIME_OUTPUT_DELAY", helicsJNI.HELICS_PROPERTY_TIME_OUTPUT_DELAY_get());
  public final static HelicsProperties HELICS_PROPERTY_INT_MAX_ITERATIONS = new HelicsProperties("HELICS_PROPERTY_INT_MAX_ITERATIONS", helicsJNI.HELICS_PROPERTY_INT_MAX_ITERATIONS_get());
  public final static HelicsProperties HELICS_PROPERTY_INT_LOG_LEVEL = new HelicsProperties("HELICS_PROPERTY_INT_LOG_LEVEL", helicsJNI.HELICS_PROPERTY_INT_LOG_LEVEL_get());
  public final static HelicsProperties HELICS_PROPERTY_INT_FILE_LOG_LEVEL = new HelicsProperties("HELICS_PROPERTY_INT_FILE_LOG_LEVEL", helicsJNI.HELICS_PROPERTY_INT_FILE_LOG_LEVEL_get());
  public final static HelicsProperties HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL = new HelicsProperties("HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL", helicsJNI.HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static HelicsProperties swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + HelicsProperties.class + " with value " + swigValue);
  }

  private HelicsProperties(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private HelicsProperties(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private HelicsProperties(String swigName, HelicsProperties swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static HelicsProperties[] swigValues = { HELICS_PROPERTY_TIME_DELTA, HELICS_PROPERTY_TIME_PERIOD, HELICS_PROPERTY_TIME_OFFSET, HELICS_PROPERTY_TIME_RT_LAG, HELICS_PROPERTY_TIME_RT_LEAD, HELICS_PROPERTY_TIME_RT_TOLERANCE, HELICS_PROPERTY_TIME_INPUT_DELAY, HELICS_PROPERTY_TIME_OUTPUT_DELAY, HELICS_PROPERTY_INT_MAX_ITERATIONS, HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_PROPERTY_INT_FILE_LOG_LEVEL, HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

