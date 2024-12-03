/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (https://www.swig.org).
 * Version 4.3.0
 *
 * Do not make changes to this file unless you know what you are doing - modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

/**
 *  enumeration of options that apply to handles
 */
public final class HelicsHandleOptions {
  /**
   *  specify that a connection is required for an interface and will generate an error<br>
   *                   if not available
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_CONNECTION_REQUIRED = new HelicsHandleOptions("HELICS_HANDLE_OPTION_CONNECTION_REQUIRED", helicsJNI.HELICS_HANDLE_OPTION_CONNECTION_REQUIRED_get());
  /**
   *  specify that a connection is NOT required for an interface and will only be made<br>
   *                   if available no warning will be issues if not available
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL = new HelicsHandleOptions("HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL", helicsJNI.HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL_get());
  /**
   *  specify that only a single connection is allowed for an interface
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY = new HelicsHandleOptions("HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY", helicsJNI.HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY_get());
  /**
   *  specify that multiple connections are allowed for an interface
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED = new HelicsHandleOptions("HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED", helicsJNI.HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED_get());
  /**
   *  specify that the last data should be buffered and sent on subscriptions after<br>
   *                   init
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_BUFFER_DATA = new HelicsHandleOptions("HELICS_HANDLE_OPTION_BUFFER_DATA", helicsJNI.HELICS_HANDLE_OPTION_BUFFER_DATA_get());
  /**
   *  specify that the handle can be reconnected for reentrant federates
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_RECONNECTABLE = new HelicsHandleOptions("HELICS_HANDLE_OPTION_RECONNECTABLE", helicsJNI.HELICS_HANDLE_OPTION_RECONNECTABLE_get());
  /**
   *  specify that the types should be checked strictly for pub/sub and filters
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING = new HelicsHandleOptions("HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING", helicsJNI.HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING_get());
  /**
   *  specify that the handle is receive only
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_RECEIVE_ONLY = new HelicsHandleOptions("HELICS_HANDLE_OPTION_RECEIVE_ONLY", helicsJNI.HELICS_HANDLE_OPTION_RECEIVE_ONLY_get());
  /**
   *  specify that the handle is source only
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_SOURCE_ONLY = new HelicsHandleOptions("HELICS_HANDLE_OPTION_SOURCE_ONLY", helicsJNI.HELICS_HANDLE_OPTION_SOURCE_ONLY_get());
  /**
   *  specify that the mismatching units should be ignored
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH = new HelicsHandleOptions("HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH", helicsJNI.HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH_get());
  /**
   *  specify that an interface will only transmit on change(only applicable to<br>
   *                   publications)
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE = new HelicsHandleOptions("HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE", helicsJNI.HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE_get());
  /**
   *  specify that an interface will only update if the value has actually changed
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE = new HelicsHandleOptions("HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE", helicsJNI.HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE_get());
  /**
   *  specify that an interface does not participate in determining time interrupts
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS = new HelicsHandleOptions("HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS", helicsJNI.HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS_get());
  /**
   *  specify the multi-input processing method for inputs
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD = new HelicsHandleOptions("HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD", helicsJNI.HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD_get());
  /**
   *  specify the source index with the highest priority
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION = new HelicsHandleOptions("HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION", helicsJNI.HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION_get());
  /**
   *  specify that the priority list should be cleared or question if it is cleared
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST = new HelicsHandleOptions("HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST", helicsJNI.HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST_get());
  /**
   *  specify the required number of connections or get the actual number of<br>
   *                   connections
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_CONNECTIONS = new HelicsHandleOptions("HELICS_HANDLE_OPTION_CONNECTIONS", helicsJNI.HELICS_HANDLE_OPTION_CONNECTIONS_get());
  /**
   *  specify that the interface only sends or receives data at specified intervals
   */
  public final static HelicsHandleOptions HELICS_HANDLE_OPTION_TIME_RESTRICTED = new HelicsHandleOptions("HELICS_HANDLE_OPTION_TIME_RESTRICTED", helicsJNI.HELICS_HANDLE_OPTION_TIME_RESTRICTED_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static HelicsHandleOptions swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + HelicsHandleOptions.class + " with value " + swigValue);
  }

  private HelicsHandleOptions(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private HelicsHandleOptions(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private HelicsHandleOptions(String swigName, HelicsHandleOptions swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static HelicsHandleOptions[] swigValues = { HELICS_HANDLE_OPTION_CONNECTION_REQUIRED, HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL, HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY, HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED, HELICS_HANDLE_OPTION_BUFFER_DATA, HELICS_HANDLE_OPTION_RECONNECTABLE, HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING, HELICS_HANDLE_OPTION_RECEIVE_ONLY, HELICS_HANDLE_OPTION_SOURCE_ONLY, HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH, HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE, HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE, HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS, HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD, HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION, HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST, HELICS_HANDLE_OPTION_CONNECTIONS, HELICS_HANDLE_OPTION_TIME_RESTRICTED };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

