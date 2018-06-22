/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public class helics implements helicsConstants {
  public static double getHelics_time_zero() {
    return helicsJNI.helics_time_zero_get();
  }

  public static double getHelics_time_epsilon() {
    return helicsJNI.helics_time_epsilon_get();
  }

  public static int getHelics_true() {
    return helicsJNI.helics_true_get();
  }

  public static int getHelics_false() {
    return helicsJNI.helics_false_get();
  }

  public static String helicsGetVersion() {
    return helicsJNI.helicsGetVersion();
  }

  public static int helicsIsCoreTypeAvailable(String type) {
    return helicsJNI.helicsIsCoreTypeAvailable(type);
  }

  public static SWIGTYPE_p_void helicsCreateCore(String type, String name, String initString) {
    long cPtr = helicsJNI.helicsCreateCore(type, name, initString);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateCoreFromArgs(String type, String name, String[] argc) {
    long cPtr = helicsJNI.helicsCreateCoreFromArgs(type, name, argc);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCoreClone(SWIGTYPE_p_void core) {
    long cPtr = helicsJNI.helicsCoreClone(SWIGTYPE_p_void.getCPtr(core));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateBroker(String type, String name, String initString) {
    long cPtr = helicsJNI.helicsCreateBroker(type, name, initString);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateBrokerFromArgs(String type, String name, String[] argc) {
    long cPtr = helicsJNI.helicsCreateBrokerFromArgs(type, name, argc);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsBrokerClone(SWIGTYPE_p_void broker) {
    long cPtr = helicsJNI.helicsBrokerClone(SWIGTYPE_p_void.getCPtr(broker));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static int helicsBrokerIsConnected(SWIGTYPE_p_void broker) {
    return helicsJNI.helicsBrokerIsConnected(SWIGTYPE_p_void.getCPtr(broker));
  }

  public static int helicsCoreIsConnected(SWIGTYPE_p_void core) {
    return helicsJNI.helicsCoreIsConnected(SWIGTYPE_p_void.getCPtr(core));
  }

  public static helics_status helicsBrokerGetIdentifier(SWIGTYPE_p_void broker, byte[] identifier) {
    return helics_status.swigToEnum(helicsJNI.helicsBrokerGetIdentifier(SWIGTYPE_p_void.getCPtr(broker), identifier));
  }

  public static helics_status helicsCoreGetIdentifier(SWIGTYPE_p_void core, byte[] identifier) {
    return helics_status.swigToEnum(helicsJNI.helicsCoreGetIdentifier(SWIGTYPE_p_void.getCPtr(core), identifier));
  }

  public static helics_status helicsBrokerGetAddress(SWIGTYPE_p_void broker, byte[] address) {
    return helics_status.swigToEnum(helicsJNI.helicsBrokerGetAddress(SWIGTYPE_p_void.getCPtr(broker), address));
  }

  public static helics_status helicsCoreSetReadyToInit(SWIGTYPE_p_void core) {
    return helics_status.swigToEnum(helicsJNI.helicsCoreSetReadyToInit(SWIGTYPE_p_void.getCPtr(core)));
  }

  public static helics_status helicsCoreDisconnect(SWIGTYPE_p_void core) {
    return helics_status.swigToEnum(helicsJNI.helicsCoreDisconnect(SWIGTYPE_p_void.getCPtr(core)));
  }

  public static SWIGTYPE_p_void helicsGetFederateByName(String fedName) {
    long cPtr = helicsJNI.helicsGetFederateByName(fedName);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsBrokerDisconnect(SWIGTYPE_p_void broker) {
    return helics_status.swigToEnum(helicsJNI.helicsBrokerDisconnect(SWIGTYPE_p_void.getCPtr(broker)));
  }

  public static void helicsCoreFree(SWIGTYPE_p_void core) {
    helicsJNI.helicsCoreFree(SWIGTYPE_p_void.getCPtr(core));
  }

  public static void helicsBrokerFree(SWIGTYPE_p_void broker) {
    helicsJNI.helicsBrokerFree(SWIGTYPE_p_void.getCPtr(broker));
  }

  public static SWIGTYPE_p_void helicsCreateValueFederate(SWIGTYPE_p_void fi) {
    long cPtr = helicsJNI.helicsCreateValueFederate(SWIGTYPE_p_void.getCPtr(fi));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateValueFederateFromJson(String json) {
    long cPtr = helicsJNI.helicsCreateValueFederateFromJson(json);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateMessageFederate(SWIGTYPE_p_void fi) {
    long cPtr = helicsJNI.helicsCreateMessageFederate(SWIGTYPE_p_void.getCPtr(fi));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateMessageFederateFromJson(String json) {
    long cPtr = helicsJNI.helicsCreateMessageFederateFromJson(json);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateCombinationFederate(SWIGTYPE_p_void fi) {
    long cPtr = helicsJNI.helicsCreateCombinationFederate(SWIGTYPE_p_void.getCPtr(fi));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCreateCombinationFederateFromJson(String json) {
    long cPtr = helicsJNI.helicsCreateCombinationFederateFromJson(json);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateClone(SWIGTYPE_p_void fed) {
    long cPtr = helicsJNI.helicsFederateClone(SWIGTYPE_p_void.getCPtr(fed));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateInfoCreate() {
    long cPtr = helicsJNI.helicsFederateInfoCreate();
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsFederateInfoLoadFromArgs(SWIGTYPE_p_void fi, String[] argc) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoLoadFromArgs(SWIGTYPE_p_void.getCPtr(fi), argc));
  }

  public static void helicsFederateInfoFree(SWIGTYPE_p_void fi) {
    helicsJNI.helicsFederateInfoFree(SWIGTYPE_p_void.getCPtr(fi));
  }

  public static helics_status helicsFederateInfoSetFederateName(SWIGTYPE_p_void fi, String name) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetFederateName(SWIGTYPE_p_void.getCPtr(fi), name));
  }

  public static helics_status helicsFederateInfoSetCoreName(SWIGTYPE_p_void fi, String corename) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetCoreName(SWIGTYPE_p_void.getCPtr(fi), corename));
  }

  public static helics_status helicsFederateInfoSetCoreInitString(SWIGTYPE_p_void fi, String coreInit) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetCoreInitString(SWIGTYPE_p_void.getCPtr(fi), coreInit));
  }

  public static helics_status helicsFederateInfoSetCoreTypeFromString(SWIGTYPE_p_void fi, String coretype) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetCoreTypeFromString(SWIGTYPE_p_void.getCPtr(fi), coretype));
  }

  public static helics_status helicsFederateInfoSetCoreType(SWIGTYPE_p_void fi, int coretype) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetCoreType(SWIGTYPE_p_void.getCPtr(fi), coretype));
  }

  public static helics_status helicsFederateInfoSetFlag(SWIGTYPE_p_void fi, int flag, int value) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetFlag(SWIGTYPE_p_void.getCPtr(fi), flag, value));
  }

  public static helics_status helicsFederateInfoSetSeparator(SWIGTYPE_p_void fi, char separator) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetSeparator(SWIGTYPE_p_void.getCPtr(fi), separator));
  }

  public static helics_status helicsFederateInfoSetOutputDelay(SWIGTYPE_p_void fi, double outputDelay) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetOutputDelay(SWIGTYPE_p_void.getCPtr(fi), outputDelay));
  }

  public static helics_status helicsFederateInfoSetTimeDelta(SWIGTYPE_p_void fi, double timeDelta) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetTimeDelta(SWIGTYPE_p_void.getCPtr(fi), timeDelta));
  }

  public static helics_status helicsFederateInfoSetInputDelay(SWIGTYPE_p_void fi, double inputDelay) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetInputDelay(SWIGTYPE_p_void.getCPtr(fi), inputDelay));
  }

  public static helics_status helicsFederateInfoSetTimeOffset(SWIGTYPE_p_void fi, double timeOffset) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetTimeOffset(SWIGTYPE_p_void.getCPtr(fi), timeOffset));
  }

  public static helics_status helicsFederateInfoSetPeriod(SWIGTYPE_p_void fi, double period) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetPeriod(SWIGTYPE_p_void.getCPtr(fi), period));
  }

  public static helics_status helicsFederateInfoSetMaxIterations(SWIGTYPE_p_void fi, int maxIterations) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetMaxIterations(SWIGTYPE_p_void.getCPtr(fi), maxIterations));
  }

  public static helics_status helicsFederateInfoSetLoggingLevel(SWIGTYPE_p_void fi, int logLevel) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateInfoSetLoggingLevel(SWIGTYPE_p_void.getCPtr(fi), logLevel));
  }

  public static helics_status helicsFederateFinalize(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateFinalize(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static void helicsFederateFree(SWIGTYPE_p_void fed) {
    helicsJNI.helicsFederateFree(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static void helicsCloseLibrary() {
    helicsJNI.helicsCloseLibrary();
  }

  public static helics_status helicsFederateEnterInitializationMode(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterInitializationMode(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static helics_status helicsFederateEnterInitializationModeAsync(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterInitializationModeAsync(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static int helicsFederateIsAsyncOperationCompleted(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateIsAsyncOperationCompleted(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static helics_status helicsFederateEnterInitializationModeComplete(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterInitializationModeComplete(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static helics_status helicsFederateEnterExecutionMode(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionMode(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static helics_status helicsFederateEnterExecutionModeAsync(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionModeAsync(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static helics_status helicsFederateEnterExecutionModeComplete(SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionModeComplete(SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static helics_status helicsFederateEnterExecutionModeIterative(SWIGTYPE_p_void fed, helics_iteration_request iterate, int[] outIterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionModeIterative(SWIGTYPE_p_void.getCPtr(fed), iterate.swigValue(), outIterate));
  }

  public static helics_status helicsFederateEnterExecutionModeIterativeAsync(SWIGTYPE_p_void fed, helics_iteration_request iterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionModeIterativeAsync(SWIGTYPE_p_void.getCPtr(fed), iterate.swigValue()));
  }

  public static helics_status helicsFederateEnterExecutionModeIterativeComplete(SWIGTYPE_p_void fed, int[] outIterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateEnterExecutionModeIterativeComplete(SWIGTYPE_p_void.getCPtr(fed), outIterate));
  }

  public static helics_status helicsFederateGetState(SWIGTYPE_p_void fed, int[] state) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateGetState(SWIGTYPE_p_void.getCPtr(fed), state));
  }

  public static SWIGTYPE_p_void helicsFederateGetCoreObject(SWIGTYPE_p_void fed) {
    long cPtr = helicsJNI.helicsFederateGetCoreObject(SWIGTYPE_p_void.getCPtr(fed));
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsFederateRequestTime(SWIGTYPE_p_void fed, double requestTime, double[] timeOut) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTime(SWIGTYPE_p_void.getCPtr(fed), requestTime, timeOut));
  }

  public static helics_status helicsFederateRequestTimeIterative(SWIGTYPE_p_void fed, double requestTime, helics_iteration_request iterate, double[] timeOut, int[] outIterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTimeIterative(SWIGTYPE_p_void.getCPtr(fed), requestTime, iterate.swigValue(), timeOut, outIterate));
  }

  public static helics_status helicsFederateRequestTimeAsync(SWIGTYPE_p_void fed, double requestTime) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTimeAsync(SWIGTYPE_p_void.getCPtr(fed), requestTime));
  }

  public static helics_status helicsFederateRequestTimeComplete(SWIGTYPE_p_void fed, double[] timeOut) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTimeComplete(SWIGTYPE_p_void.getCPtr(fed), timeOut));
  }

  public static helics_status helicsFederateRequestTimeIterativeAsync(SWIGTYPE_p_void fed, double requestTime, helics_iteration_request iterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTimeIterativeAsync(SWIGTYPE_p_void.getCPtr(fed), requestTime, iterate.swigValue()));
  }

  public static helics_status helicsFederateRequestTimeIterativeComplete(SWIGTYPE_p_void fed, double[] timeOut, int[] outIterate) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateRequestTimeIterativeComplete(SWIGTYPE_p_void.getCPtr(fed), timeOut, outIterate));
  }

  public static helics_status helicsFederateGetName(SWIGTYPE_p_void fed, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateGetName(SWIGTYPE_p_void.getCPtr(fed), outputString));
  }

  public static helics_status helicsFederateSetTimeDelta(SWIGTYPE_p_void fed, double time) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetTimeDelta(SWIGTYPE_p_void.getCPtr(fed), time));
  }

  public static helics_status helicsFederateSetOutputDelay(SWIGTYPE_p_void fed, double outputDelay) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetOutputDelay(SWIGTYPE_p_void.getCPtr(fed), outputDelay));
  }

  public static helics_status helicsFederateSetInputDelay(SWIGTYPE_p_void fed, double inputDelay) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetInputDelay(SWIGTYPE_p_void.getCPtr(fed), inputDelay));
  }

  public static helics_status helicsFederateSetPeriod(SWIGTYPE_p_void fed, double period, double offset) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetPeriod(SWIGTYPE_p_void.getCPtr(fed), period, offset));
  }

  public static helics_status helicsFederateSetFlag(SWIGTYPE_p_void fed, int flag, int flagValue) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetFlag(SWIGTYPE_p_void.getCPtr(fed), flag, flagValue));
  }

  public static helics_status helicsFederateSetSeparator(SWIGTYPE_p_void fed, char separator) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetSeparator(SWIGTYPE_p_void.getCPtr(fed), separator));
  }

  public static helics_status helicsFederateSetLoggingLevel(SWIGTYPE_p_void fed, int loggingLevel) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetLoggingLevel(SWIGTYPE_p_void.getCPtr(fed), loggingLevel));
  }

  public static helics_status helicsFederateSetMaxIterations(SWIGTYPE_p_void fi, int maxIterations) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateSetMaxIterations(SWIGTYPE_p_void.getCPtr(fi), maxIterations));
  }

  public static helics_status helicsFederateGetCurrentTime(SWIGTYPE_p_void fed, double[] time) {
    return helics_status.swigToEnum(helicsJNI.helicsFederateGetCurrentTime(SWIGTYPE_p_void.getCPtr(fed), time));
  }

  public static SWIGTYPE_p_void helicsCreateQuery(String target, String query) {
    long cPtr = helicsJNI.helicsCreateQuery(target, query);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static String helicsQueryExecute(SWIGTYPE_p_void query, SWIGTYPE_p_void fed) {
    return helicsJNI.helicsQueryExecute(SWIGTYPE_p_void.getCPtr(query), SWIGTYPE_p_void.getCPtr(fed));
  }

  public static String helicsQueryCoreExecute(SWIGTYPE_p_void query, SWIGTYPE_p_void core) {
    return helicsJNI.helicsQueryCoreExecute(SWIGTYPE_p_void.getCPtr(query), SWIGTYPE_p_void.getCPtr(core));
  }

  public static String helicsQueryBrokerExecute(SWIGTYPE_p_void query, SWIGTYPE_p_void broker) {
    return helicsJNI.helicsQueryBrokerExecute(SWIGTYPE_p_void.getCPtr(query), SWIGTYPE_p_void.getCPtr(broker));
  }

  public static helics_status helicsQueryExecuteAsync(SWIGTYPE_p_void query, SWIGTYPE_p_void fed) {
    return helics_status.swigToEnum(helicsJNI.helicsQueryExecuteAsync(SWIGTYPE_p_void.getCPtr(query), SWIGTYPE_p_void.getCPtr(fed)));
  }

  public static String helicsQueryExecuteComplete(SWIGTYPE_p_void query) {
    return helicsJNI.helicsQueryExecuteComplete(SWIGTYPE_p_void.getCPtr(query));
  }

  public static int helicsQueryIsCompleted(SWIGTYPE_p_void query) {
    return helicsJNI.helicsQueryIsCompleted(SWIGTYPE_p_void.getCPtr(query));
  }

  public static void helicsQueryFree(SWIGTYPE_p_void arg0) {
    helicsJNI.helicsQueryFree(SWIGTYPE_p_void.getCPtr(arg0));
  }

  public static void helicsCleanupHelicsLibrary() {
    helicsJNI.helicsCleanupHelicsLibrary();
  }

  public static SWIGTYPE_p_void helicsFederateRegisterSubscription(SWIGTYPE_p_void fed, String key, String type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterSubscription(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterTypeSubscription(SWIGTYPE_p_void fed, String key, int type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterTypeSubscription(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterOptionalSubscription(SWIGTYPE_p_void fed, String key, String type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterOptionalSubscription(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterOptionalTypeSubscription(SWIGTYPE_p_void fed, String key, int type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterOptionalTypeSubscription(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterPublication(SWIGTYPE_p_void fed, String key, String type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterPublication(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterTypePublication(SWIGTYPE_p_void fed, String key, int type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterTypePublication(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterGlobalPublication(SWIGTYPE_p_void fed, String key, String type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterGlobalPublication(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterGlobalTypePublication(SWIGTYPE_p_void fed, String key, int type, String units) {
    long cPtr = helicsJNI.helicsFederateRegisterGlobalTypePublication(SWIGTYPE_p_void.getCPtr(fed), key, type, units);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsPublicationPublishRaw(SWIGTYPE_p_void pub, SWIGTYPE_p_void data, int inputDataLength) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishRaw(SWIGTYPE_p_void.getCPtr(pub), SWIGTYPE_p_void.getCPtr(data), inputDataLength));
  }

  public static helics_status helicsPublicationPublishString(SWIGTYPE_p_void pub, String str) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishString(SWIGTYPE_p_void.getCPtr(pub), str));
  }

  public static helics_status helicsPublicationPublishInteger(SWIGTYPE_p_void pub, long val) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishInteger(SWIGTYPE_p_void.getCPtr(pub), val));
  }

  public static helics_status helicsPublicationPublishBoolean(SWIGTYPE_p_void pub, int val) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishBoolean(SWIGTYPE_p_void.getCPtr(pub), val));
  }

  public static helics_status helicsPublicationPublishDouble(SWIGTYPE_p_void pub, double val) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishDouble(SWIGTYPE_p_void.getCPtr(pub), val));
  }

  public static helics_status helicsPublicationPublishComplex(SWIGTYPE_p_void pub, double real, double imag) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishComplex(SWIGTYPE_p_void.getCPtr(pub), real, imag));
  }

  public static helics_status helicsPublicationPublishVector(SWIGTYPE_p_void pub, double[] vectorInput, int vectorlength) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishVector(SWIGTYPE_p_void.getCPtr(pub), vectorInput, vectorlength));
  }

  public static helics_status helicsPublicationPublishNamedPoint(SWIGTYPE_p_void pub, String str, double val) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationPublishNamedPoint(SWIGTYPE_p_void.getCPtr(pub), str, val));
  }

  public static int helicsSubscriptionGetValueSize(SWIGTYPE_p_void sub) {
    return helicsJNI.helicsSubscriptionGetValueSize(SWIGTYPE_p_void.getCPtr(sub));
  }

  public static helics_status helicsSubscriptionGetRawValue(SWIGTYPE_p_void sub, SWIGTYPE_p_void data, int maxlen, int[] actualLength) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetRawValue(SWIGTYPE_p_void.getCPtr(sub), SWIGTYPE_p_void.getCPtr(data), maxlen, actualLength));
  }

  public static int helicsSubscriptionGetStringSize(SWIGTYPE_p_void sub) {
    return helicsJNI.helicsSubscriptionGetStringSize(SWIGTYPE_p_void.getCPtr(sub));
  }

  public static helics_status helicsSubscriptionGetString(SWIGTYPE_p_void sub, byte[] outputString, int[] actualLength) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetString(SWIGTYPE_p_void.getCPtr(sub), outputString, actualLength));
  }

  public static helics_status helicsSubscriptionGetInteger(SWIGTYPE_p_void sub, long[] val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetInteger(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionGetBoolean(SWIGTYPE_p_void sub, int[] val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetBoolean(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionGetDouble(SWIGTYPE_p_void sub, double[] val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetDouble(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionGetComplex(SWIGTYPE_p_void sub, double[] real, double[] imag) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetComplex(SWIGTYPE_p_void.getCPtr(sub), real, imag));
  }

  public static int helicsSubscriptionGetVectorSize(SWIGTYPE_p_void sub) {
    return helicsJNI.helicsSubscriptionGetVectorSize(SWIGTYPE_p_void.getCPtr(sub));
  }

  public static helics_status helicsSubscriptionGetVector(SWIGTYPE_p_void sub, SWIGTYPE_p_double data, int maxlen, int[] actualSize) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetVector(SWIGTYPE_p_void.getCPtr(sub), SWIGTYPE_p_double.getCPtr(data), maxlen, actualSize));
  }

  public static helics_status helicsSubscriptionGetNamedPoint(SWIGTYPE_p_void sub, byte[] outputString, int[] actualLength, double[] val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetNamedPoint(SWIGTYPE_p_void.getCPtr(sub), outputString, actualLength, val));
  }

  public static helics_status helicsSubscriptionSetDefaultRaw(SWIGTYPE_p_void sub, SWIGTYPE_p_void data, int inputDataLength) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultRaw(SWIGTYPE_p_void.getCPtr(sub), SWIGTYPE_p_void.getCPtr(data), inputDataLength));
  }

  public static helics_status helicsSubscriptionSetDefaultString(SWIGTYPE_p_void sub, String str) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultString(SWIGTYPE_p_void.getCPtr(sub), str));
  }

  public static helics_status helicsSubscriptionSetDefaultInteger(SWIGTYPE_p_void sub, long val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultInteger(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionSetDefaultBoolean(SWIGTYPE_p_void sub, int val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultBoolean(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionSetDefaultDouble(SWIGTYPE_p_void sub, double val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultDouble(SWIGTYPE_p_void.getCPtr(sub), val));
  }

  public static helics_status helicsSubscriptionSetDefaultComplex(SWIGTYPE_p_void sub, double real, double imag) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultComplex(SWIGTYPE_p_void.getCPtr(sub), real, imag));
  }

  public static helics_status helicsSubscriptionSetDefaultVector(SWIGTYPE_p_void sub, double[] vectorInput, int vectorlength) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultVector(SWIGTYPE_p_void.getCPtr(sub), vectorInput, vectorlength));
  }

  public static helics_status helicsSubscriptionSetDefaultNamedPoint(SWIGTYPE_p_void sub, String str, double val) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionSetDefaultNamedPoint(SWIGTYPE_p_void.getCPtr(sub), str, val));
  }

  public static helics_status helicsSubscriptionGetType(SWIGTYPE_p_void sub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetType(SWIGTYPE_p_void.getCPtr(sub), outputString));
  }

  public static helics_status helicsPublicationGetType(SWIGTYPE_p_void pub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationGetType(SWIGTYPE_p_void.getCPtr(pub), outputString));
  }

  public static helics_status helicsSubscriptionGetKey(SWIGTYPE_p_void sub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetKey(SWIGTYPE_p_void.getCPtr(sub), outputString));
  }

  public static helics_status helicsPublicationGetKey(SWIGTYPE_p_void pub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationGetKey(SWIGTYPE_p_void.getCPtr(pub), outputString));
  }

  public static helics_status helicsSubscriptionGetUnits(SWIGTYPE_p_void sub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsSubscriptionGetUnits(SWIGTYPE_p_void.getCPtr(sub), outputString));
  }

  public static helics_status helicsPublicationGetUnits(SWIGTYPE_p_void pub, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsPublicationGetUnits(SWIGTYPE_p_void.getCPtr(pub), outputString));
  }

  public static int helicsSubscriptionIsUpdated(SWIGTYPE_p_void sub) {
    return helicsJNI.helicsSubscriptionIsUpdated(SWIGTYPE_p_void.getCPtr(sub));
  }

  public static double helicsSubscriptionLastUpdateTime(SWIGTYPE_p_void sub) {
    return helicsJNI.helicsSubscriptionLastUpdateTime(SWIGTYPE_p_void.getCPtr(sub));
  }

  public static int helicsFederateGetPublicationCount(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateGetPublicationCount(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static int helicsFederateGetSubscriptionCount(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateGetSubscriptionCount(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static SWIGTYPE_p_void helicsFederateRegisterEndpoint(SWIGTYPE_p_void fed, String name, String type) {
    long cPtr = helicsJNI.helicsFederateRegisterEndpoint(SWIGTYPE_p_void.getCPtr(fed), name, type);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterGlobalEndpoint(SWIGTYPE_p_void fed, String name, String type) {
    long cPtr = helicsJNI.helicsFederateRegisterGlobalEndpoint(SWIGTYPE_p_void.getCPtr(fed), name, type);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsEndpointSetDefaultDestination(SWIGTYPE_p_void endpoint, String dest) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointSetDefaultDestination(SWIGTYPE_p_void.getCPtr(endpoint), dest));
  }

  public static helics_status helicsEndpointSendMessageRaw(SWIGTYPE_p_void endpoint, String dest, SWIGTYPE_p_void data, int inputDataLength) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointSendMessageRaw(SWIGTYPE_p_void.getCPtr(endpoint), dest, SWIGTYPE_p_void.getCPtr(data), inputDataLength));
  }

  public static helics_status helicsEndpointSendEventRaw(SWIGTYPE_p_void endpoint, String dest, SWIGTYPE_p_void data, int inputDataLength, double time) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointSendEventRaw(SWIGTYPE_p_void.getCPtr(endpoint), dest, SWIGTYPE_p_void.getCPtr(data), inputDataLength, time));
  }

  public static helics_status helicsEndpointSendMessage(SWIGTYPE_p_void endpoint, message_t message) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointSendMessage(SWIGTYPE_p_void.getCPtr(endpoint), message_t.getCPtr(message), message));
  }

  public static helics_status helicsEndpointSubscribe(SWIGTYPE_p_void endpoint, String key, String type) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointSubscribe(SWIGTYPE_p_void.getCPtr(endpoint), key, type));
  }

  public static int helicsFederateHasMessage(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateHasMessage(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static int helicsEndpointHasMessage(SWIGTYPE_p_void endpoint) {
    return helicsJNI.helicsEndpointHasMessage(SWIGTYPE_p_void.getCPtr(endpoint));
  }

  public static int helicsFederateReceiveCount(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateReceiveCount(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static int helicsEndpointReceiveCount(SWIGTYPE_p_void endpoint) {
    return helicsJNI.helicsEndpointReceiveCount(SWIGTYPE_p_void.getCPtr(endpoint));
  }

  public static message_t helicsEndpointGetMessage(SWIGTYPE_p_void endpoint) {
    return new message_t(helicsJNI.helicsEndpointGetMessage(SWIGTYPE_p_void.getCPtr(endpoint)), true);
  }

  public static message_t helicsFederateGetMessage(SWIGTYPE_p_void fed) {
    return new message_t(helicsJNI.helicsFederateGetMessage(SWIGTYPE_p_void.getCPtr(fed)), true);
  }

  public static helics_status helicsEndpointGetType(SWIGTYPE_p_void endpoint, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointGetType(SWIGTYPE_p_void.getCPtr(endpoint), outputString));
  }

  public static helics_status helicsEndpointGetName(SWIGTYPE_p_void endpoint, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsEndpointGetName(SWIGTYPE_p_void.getCPtr(endpoint), outputString));
  }

  public static int helicsFederateGetEndpointCount(SWIGTYPE_p_void fed) {
    return helicsJNI.helicsFederateGetEndpointCount(SWIGTYPE_p_void.getCPtr(fed));
  }

  public static SWIGTYPE_p_void helicsFederateRegisterSourceFilter(SWIGTYPE_p_void fed, helics_filter_type_t type, String target, String name) {
    long cPtr = helicsJNI.helicsFederateRegisterSourceFilter(SWIGTYPE_p_void.getCPtr(fed), type.swigValue(), target, name);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterDestinationFilter(SWIGTYPE_p_void fed, helics_filter_type_t type, String target, String name) {
    long cPtr = helicsJNI.helicsFederateRegisterDestinationFilter(SWIGTYPE_p_void.getCPtr(fed), type.swigValue(), target, name);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsFederateRegisterCloningFilter(SWIGTYPE_p_void fed, String deliveryEndpoint) {
    long cPtr = helicsJNI.helicsFederateRegisterCloningFilter(SWIGTYPE_p_void.getCPtr(fed), deliveryEndpoint);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCoreRegisterSourceFilter(SWIGTYPE_p_void core, helics_filter_type_t type, String target, String name) {
    long cPtr = helicsJNI.helicsCoreRegisterSourceFilter(SWIGTYPE_p_void.getCPtr(core), type.swigValue(), target, name);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCoreRegisterDestinationFilter(SWIGTYPE_p_void core, helics_filter_type_t type, String target, String name) {
    long cPtr = helicsJNI.helicsCoreRegisterDestinationFilter(SWIGTYPE_p_void.getCPtr(core), type.swigValue(), target, name);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static SWIGTYPE_p_void helicsCoreRegisterCloningFilter(SWIGTYPE_p_void core, String deliveryEndpoint) {
    long cPtr = helicsJNI.helicsCoreRegisterCloningFilter(SWIGTYPE_p_void.getCPtr(core), deliveryEndpoint);
    return (cPtr == 0) ? null : new SWIGTYPE_p_void(cPtr, false);
  }

  public static helics_status helicsFilterGetTarget(SWIGTYPE_p_void filt, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterGetTarget(SWIGTYPE_p_void.getCPtr(filt), outputString));
  }

  public static helics_status helicsFilterGetName(SWIGTYPE_p_void filt, byte[] outputString) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterGetName(SWIGTYPE_p_void.getCPtr(filt), outputString));
  }

  public static helics_status helicsFilterSet(SWIGTYPE_p_void filt, String property, double val) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterSet(SWIGTYPE_p_void.getCPtr(filt), property, val));
  }

  public static helics_status helicsFilterSetString(SWIGTYPE_p_void filt, String property, String val) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterSetString(SWIGTYPE_p_void.getCPtr(filt), property, val));
  }

  public static helics_status helicsFilterAddDestinationTarget(SWIGTYPE_p_void filt, String dest) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterAddDestinationTarget(SWIGTYPE_p_void.getCPtr(filt), dest));
  }

  public static helics_status helicsFilterAddSourceTarget(SWIGTYPE_p_void filt, String source) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterAddSourceTarget(SWIGTYPE_p_void.getCPtr(filt), source));
  }

  public static helics_status helicsFilterAddDeliveryEndpoint(SWIGTYPE_p_void filt, String deliveryEndpoint) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterAddDeliveryEndpoint(SWIGTYPE_p_void.getCPtr(filt), deliveryEndpoint));
  }

  public static helics_status helicsFilterRemoveDestinationTarget(SWIGTYPE_p_void filt, String dest) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterRemoveDestinationTarget(SWIGTYPE_p_void.getCPtr(filt), dest));
  }

  public static helics_status helicsFilterRemoveSourceTarget(SWIGTYPE_p_void filt, String source) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterRemoveSourceTarget(SWIGTYPE_p_void.getCPtr(filt), source));
  }

  public static helics_status helicsFilterRemoveDeliveryEndpoint(SWIGTYPE_p_void filt, String deliveryEndpoint) {
    return helics_status.swigToEnum(helicsJNI.helicsFilterRemoveDeliveryEndpoint(SWIGTYPE_p_void.getCPtr(filt), deliveryEndpoint));
  }

}
