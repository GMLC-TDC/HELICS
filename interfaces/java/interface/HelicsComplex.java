/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

/**
 *  structure defining a basic complex type
 */
public class HelicsComplex {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected HelicsComplex(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(HelicsComplex obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  @SuppressWarnings("deprecation")
  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        helicsJNI.delete_HelicsComplex(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setReal(double value) {
    helicsJNI.HelicsComplex_real_set(swigCPtr, this, value);
  }

  public double getReal() {
    return helicsJNI.HelicsComplex_real_get(swigCPtr, this);
  }

  public void setImag(double value) {
    helicsJNI.HelicsComplex_imag_set(swigCPtr, this, value);
  }

  public double getImag() {
    return helicsJNI.HelicsComplex_imag_get(swigCPtr, this);
  }

  public HelicsComplex() {
    this(helicsJNI.new_HelicsComplex(), true);
  }

}