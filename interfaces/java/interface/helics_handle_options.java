/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

/**
 *  enumeration of options that apply to handles
 */
public final class helics_handle_options {
  /**
   *  specify that a connection is required for an interface and will generate an error if not<br>
   *        available
   */
  public final static helics_handle_options helics_handle_option_connection_required = new helics_handle_options("helics_handle_option_connection_required", helicsJNI.helics_handle_option_connection_required_get());
  /**
   *  specify that a connection is NOT required for an interface and will only be made if<br>
   *        available no warning will be issues if not available
   */
  public final static helics_handle_options helics_handle_option_connection_optional = new helics_handle_options("helics_handle_option_connection_optional", helicsJNI.helics_handle_option_connection_optional_get());
  /**
   *  specify that only a single connection is allowed for an interface
   */
  public final static helics_handle_options helics_handle_option_single_connection_only = new helics_handle_options("helics_handle_option_single_connection_only", helicsJNI.helics_handle_option_single_connection_only_get());
  /**
   *  specify that multiple connections are allowed for an interface
   */
  public final static helics_handle_options helics_handle_option_multiple_connections_allowed = new helics_handle_options("helics_handle_option_multiple_connections_allowed", helicsJNI.helics_handle_option_multiple_connections_allowed_get());
  /**
   *  specify that the last data should be buffered and sent on subscriptions after init
   */
  public final static helics_handle_options helics_handle_option_buffer_data = new helics_handle_options("helics_handle_option_buffer_data", helicsJNI.helics_handle_option_buffer_data_get());
  /**
   *  specify that the types should be checked strictly for pub/sub and filters
   */
  public final static helics_handle_options helics_handle_option_strict_type_checking = new helics_handle_options("helics_handle_option_strict_type_checking", helicsJNI.helics_handle_option_strict_type_checking_get());
  /**
   *  specify that the mismatching units should be ignored
   */
  public final static helics_handle_options helics_handle_option_ignore_unit_mismatch = new helics_handle_options("helics_handle_option_ignore_unit_mismatch", helicsJNI.helics_handle_option_ignore_unit_mismatch_get());
  /**
   *  specify that an interface will only transmit on change(only applicable to<br>
   *        publications)
   */
  public final static helics_handle_options helics_handle_option_only_transmit_on_change = new helics_handle_options("helics_handle_option_only_transmit_on_change", helicsJNI.helics_handle_option_only_transmit_on_change_get());
  /**
   *  specify that an interface will only update if the value has actually changed
   */
  public final static helics_handle_options helics_handle_option_only_update_on_change = new helics_handle_options("helics_handle_option_only_update_on_change", helicsJNI.helics_handle_option_only_update_on_change_get());
  /**
   *  specify that an interface does not participate in determining time interrupts
   */
  public final static helics_handle_options helics_handle_option_ignore_interrupts = new helics_handle_options("helics_handle_option_ignore_interrupts", helicsJNI.helics_handle_option_ignore_interrupts_get());
  /**
   *  specify the multi-input processing method for inputs
   */
  public final static helics_handle_options helics_handle_option_multi_input_handling_method = new helics_handle_options("helics_handle_option_multi_input_handling_method", helicsJNI.helics_handle_option_multi_input_handling_method_get());
  /**
   *  specify the source index with the highest priority
   */
  public final static helics_handle_options helics_handle_option_input_priority_location = new helics_handle_options("helics_handle_option_input_priority_location", helicsJNI.helics_handle_option_input_priority_location_get());
  /**
   *  specify that the priority list should be cleared or question if it is cleared
   */
  public final static helics_handle_options helics_handle_option_clear_priority_list = new helics_handle_options("helics_handle_option_clear_priority_list", helicsJNI.helics_handle_option_clear_priority_list_get());
  /**
   *  specify the required number of connections or get the actual number of connections
   */
  public final static helics_handle_options helics_handle_option_connections = new helics_handle_options("helics_handle_option_connections", helicsJNI.helics_handle_option_connections_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static helics_handle_options swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + helics_handle_options.class + " with value " + swigValue);
  }

  private helics_handle_options(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private helics_handle_options(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private helics_handle_options(String swigName, helics_handle_options swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static helics_handle_options[] swigValues = { helics_handle_option_connection_required, helics_handle_option_connection_optional, helics_handle_option_single_connection_only, helics_handle_option_multiple_connections_allowed, helics_handle_option_buffer_data, helics_handle_option_strict_type_checking, helics_handle_option_ignore_unit_mismatch, helics_handle_option_only_transmit_on_change, helics_handle_option_only_update_on_change, helics_handle_option_ignore_interrupts, helics_handle_option_multi_input_handling_method, helics_handle_option_input_priority_location, helics_handle_option_clear_priority_list, helics_handle_option_connections };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

