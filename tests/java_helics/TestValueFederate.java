import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.SWIGTYPE_p_double;
//import com.java.helics.SWIGTYPE_p_int64_t;
import com.java.helics.helics;
import com.java.helics.helics_status;
import com.java.helics.helicsConstants;

public class TestValueFederate {
	public static SWIGTYPE_p_void AddBroker(String core_type) {
		String initstring = "1 --name=mainbroker";
		String version = helics.helicsGetVersion();
		// Create broker
		SWIGTYPE_p_void broker = helics.helicsCreateBroker(core_type, "", initstring);
		int isconnected = helics.helicsBrokerIsConnected(broker);
		assert isconnected == 1;
		return broker;
	}

	public static SWIGTYPE_p_void AddFederate(SWIGTYPE_p_void broker, String core_type,
			int count, double deltat, String name_prefix) {

	    // Create Federate Info object that describes the federate properties #
	    SWIGTYPE_p_void fedinfo = helics.helicsFederateInfoCreate();

	    // Set Federate name
	    helics_status status = helics.helicsFederateInfoSetFederateName(fedinfo, name_prefix + "TestA Federate");

	    // Set core type from string
	    status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq");

	    // Federate init string
	    String fedinitstring = "--broker=mainbroker --federates="+ Integer.toString(count);
	    status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

	    // Set one second message interval
	    status = helics.helicsFederateInfoSetTimeDelta(fedinfo, deltat);

	    status = helics.helicsFederateInfoSetLoggingLevel(fedinfo, 1);

	    SWIGTYPE_p_void vFed = helics.helicsCreateValueFederate(fedinfo);

	    return vFed;
	}

	public static void test_value_federate_initialize(SWIGTYPE_p_void vFed) {
		int[] state = new int[1];
		helics_status status = helics.helicsFederateGetState(vFed, state);
		assert state[0] == 0;
		helics.helicsFederateEnterExecutionMode(vFed);
		status = helics.helicsFederateGetState(vFed, state);
		assert state[0] == 3;
	}

	public static void test_value_federate_publisher_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterTypePublication(vFed, "pub1",
								 helicsConstants.HELICS_DATA_TYPE_STRING, "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalTypePublication(vFed, "pub2",
				 helicsConstants.HELICS_DATA_TYPE_INT, "");
		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterTypePublication(vFed, "pub3",
				 helicsConstants.HELICS_DATA_TYPE_DOUBLE, "V");
		helics.helicsFederateEnterExecutionMode(vFed);
	}

	public static void test_value_federate_publication_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterPublication(vFed, "pub1",
				 "string", "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalPublication(vFed, "pub2",
				"int", "");
		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterPublication(vFed, "pub3",
				"double", "V");
		helics.helicsFederateEnterExecutionMode(vFed);

	    int maxLen = 100;
	    byte[] publication_key = new byte[100];
	    helics_status status = helics.helicsPublicationGetKey(pubid1, publication_key);
	    assert status == helics_status.helics_ok;
		String pubKey = new String(publication_key);
	    assert pubKey == "TestA Federate/pub1";
	    status = helics.helicsPublicationGetKey(pubid2, publication_key);
		assert status== helics_status.helics_ok;
		String pubKey2 = new String(publication_key);
	    assert pubKey2 == "pub2";
	    status = helics.helicsPublicationGetKey(pubid3, publication_key);
		assert status == helics_status.helics_ok;
		String pubKey3 = new String(publication_key);
	    assert pubKey3 == "TestA Federate/pub3";
	    byte[] publication_type = new byte[100];
	    status = helics.helicsPublicationGetType(pubid3, publication_type);
		String pub_type = new String(publication_key);
		assert status == helics_status.helics_ok;
	    assert pub_type == "double";
	    byte[] publication_units = new byte[100];
	    status = helics.helicsPublicationGetUnits(pubid3, publication_units);
		assert status == helics_status.helics_ok;
		String pub_units = new String(publication_units);
	    assert pub_units == "V";
	}

	public static void test_value_federate_subscription_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void subid1 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V");
		SWIGTYPE_p_void subid2 = helics.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2",
				helicsConstants.HELICS_DATA_TYPE_INT, "");
		SWIGTYPE_p_void subid3 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub3", "double", "V");
		helics.helicsFederateEnterExecutionMode(vFed);
	}

	public static void test_value_federate_subscription_and_publication_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterTypePublication(vFed, "pub1",
				helicsConstants.HELICS_DATA_TYPE_STRING, "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalTypePublication(vFed, "pub2",
				helicsConstants.HELICS_DATA_TYPE_INT, "");

		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterPublication(vFed, "pub3", "double", "V");

		SWIGTYPE_p_void subid1 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V");
		SWIGTYPE_p_void subid2 = helics.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2",
				helicsConstants.HELICS_DATA_TYPE_INT, "");

		SWIGTYPE_p_void subid3 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub3",
				"double", "V");
	}

	public static void test_value_federate_single_transfer(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		helicsConstants.HELICS_DATA_TYPE_STRING, "");
		SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "string", "");

	    helics.helicsFederateEnterExecutionMode(vFed);

	    helics.helicsPublicationPublishString(pubid, "string1");

	    double requestTime = 0.0;
	    double[] timeout = {1.0};
	    helics_status status = helics.helicsFederateRequestTime(vFed, requestTime, timeout);
	    assert status == helics_status.helics_ok;
	    byte[] s = new byte[100];
		int[] len = new int[1];
	    status = helics.helicsSubscriptionGetString(subid, s, len);

	    assert status == helics_status.helics_ok;
		String sub_string = new String(s);
	    assert sub_string == "string1";
	}

	public static void test_value_federate_runFederateTestDouble(SWIGTYPE_p_void vFed) {
	    double defaultValue = 1.0;
	    double testValue = 2.0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		helicsConstants.HELICS_DATA_TYPE_DOUBLE, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
	    helics.helicsSubscriptionSetDefaultDouble(subid, defaultValue);

	    helics.helicsFederateEnterExecutionMode (vFed);

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishDouble(pubid, testValue);

	    double[] value = new double[1];
	    helics_status status = helics.helicsSubscriptionGetDouble(subid, value);
	    assert value[0] == defaultValue;

	    double requesttime = 0.0;
	    double[] timeout = {1.0};
	    status = helics.helicsFederateRequestTime (vFed, requesttime, timeout);
	    assert requesttime == 0.1;

	    status = helics.helicsSubscriptionGetDouble(subid, value);
	    assert value[0] == testValue;

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishDouble(pubid, testValue + 1);
	    timeout[0] = 2.0;
	    status = helics.helicsFederateRequestTime (vFed, requesttime, timeout);
	    assert requesttime == 0.02;

	    status = helics.helicsSubscriptionGetDouble(subid, value);
	    assert value[0] == testValue + 1;
	}

	public static void test_value_federate_runFederateTestComplex(SWIGTYPE_p_void vFed) {
	    double rDefaultValue = 1.0;
	    double iDefaultValue = 1.0;
	    double rTestValue = 2.0;
	    double iTestValue = 2.0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		helicsConstants.HELICS_DATA_TYPE_COMPLEX, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
	    helics.helicsSubscriptionSetDefaultComplex(subid, rDefaultValue, iDefaultValue);

	    helics.helicsFederateEnterExecutionMode (vFed);

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishComplex(pubid, rTestValue, iTestValue);
	    double[] value1 = new double[1];
	    double[] value2 = new double[1];
	    helics_status status = helics.helicsSubscriptionGetComplex(subid, value1, value2);
	    assert value2[0] == iDefaultValue;

	    double requesttime = 0.0;
	    double[] timeout = {1.0};
	    status = helics.helicsFederateRequestTime (vFed, requesttime, timeout);
	    assert requesttime == 0.01;

	    status = helics.helicsSubscriptionGetComplex(subid, value1, value2);
	    assert value1[0] == rTestValue;
	    assert value2[1] == iTestValue;
	}


	public static void test_value_federate_runFederateTestInteger(SWIGTYPE_p_void vFed) {
	    double defaultValue = 1;
	    double testValue = 2;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", helicsConstants.HELICS_DATA_TYPE_INT, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "int", "");
	    //SWIGTYPE_p_int64_t pDf = null;
	    //helics.helicsSubscriptionSetDefaultInteger(subid, pDf);

	    helics.helicsFederateEnterExecutionMode (vFed);
	    //SWIGTYPE_p_int64_t pValue = null;
	    //helics.helicsPublicationPublishInteger(pubid, pValue);
	    long[] value = new long[1];
	    helics_status status = helics.helicsSubscriptionGetInteger(subid, value);
	    assert value[0] == defaultValue;
	    double requesttime = 0.0;
	    double[] timeout = {1.0};
	    status = helics.helicsFederateRequestTime(vFed, requesttime, timeout);
	    assert requesttime == 0.01;

	    status = helics.helicsSubscriptionGetInteger(subid, value);
	    assert value[0] == testValue;

	    //helics.helicsPublicationPublishInteger(pubid, pValue);
	    timeout[0] = 2.0;
	    status = helics.helicsFederateRequestTime (vFed, requesttime, timeout);
	    assert requesttime == testValue;

	    status = helics.helicsSubscriptionGetInteger(subid, value);
	    assert value[0] == testValue + 1;
	}

	public static void test_value_federate_runFederateTestString(SWIGTYPE_p_void vFed) {
	    String defaultValue = "String1";
	    String testValue = "String2";
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		helicsConstants.HELICS_DATA_TYPE_STRING, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "string", "");
	    helics.helicsSubscriptionSetDefaultString(subid, defaultValue);

	    helics.helicsFederateEnterExecutionMode(vFed);

	    // TODO: Fix error with the following function
	    helics.helicsPublicationPublishString(pubid, testValue);
	    byte[] value = new byte[100];
		int[] len = new int[1];
	    helics_status status = helics.helicsSubscriptionGetString(subid, value, len);
		String val = new String(value);
	    assert val == defaultValue;
	    double requesttime = 0.0;
	    double[] timeout = {1.0};
	    status = helics.helicsFederateRequestTime (vFed, requesttime, timeout);
	    assert requesttime == 0.01;

	    status = helics.helicsSubscriptionGetString(subid, value, len);
		String val2 = new String(value);
	    assert val2 == testValue;
	}

	public static void test_value_federate_runFederateTestVectorD(SWIGTYPE_p_void vFed) {
	    double[] defaultValue = {0, 1, 2};
	    double[] testValue = {3, 4, 5};
	    int len = 0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", helicsConstants.HELICS_DATA_TYPE_VECTOR, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "vector", "");
	    helics.helicsSubscriptionSetDefaultVector(subid, defaultValue, 3);

	    helics.helicsFederateEnterExecutionMode(vFed);

	    // TODO: Fix error with the following function
	    double[] data = new double[100];
	    helics.helicsPublicationPublishVector(pubid, data, 100);

	    int[] actualLen = new int[1];
		SWIGTYPE_p_double pData = null;
	    helics_status status = helics.helicsSubscriptionGetVector(subid, pData, 3,actualLen);
	    //assert value == [0, 1, 2];
	    double requesttime = 0.0;
	    double[] timeout = {1.0};
	    status = helics.helicsFederateRequestTime(vFed, requesttime, timeout);
	    assert requesttime == 0.01;

	    double[] value = new double[1];
	    status = helics.helicsSubscriptionGetVector(subid, pData, 3,actualLen);
	    //assert value == [3, 4, 5]
	}

	public static void main(String[] args) {
		SWIGTYPE_p_void broker = AddBroker("zmq");
		String initstring = "--broker=";
		byte[] identifier = new byte[100];
		helics_status status = helics.helicsBrokerGetIdentifier(broker, identifier);
		assert status == helics_status.helics_ok;
		String id = new String(identifier);
		initstring = initstring + id;
		initstring = initstring + " --broker_address";
		byte[] address = new byte[100];
		status = helics.helicsBrokerGetAddress(broker, address);
		assert status == helics_status.helics_ok;
		helics.helicsBrokerDisconnect(broker);
		helics.helicsCloseLibrary();
	}

}
