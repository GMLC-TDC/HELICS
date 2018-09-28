import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.SWIGTYPE_p_double;
//import com.java.helics.SWIGTYPE_p_int64_t;
import com.java.helics.helics;
import com.java.helics.helicsConstants;
import com.java.helics.federate_state;

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
	    SWIGTYPE_p_void fedinfo = helics.helicsCreateFederateInfo();

	    // Set core type from string
	    helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq");

	    // Federate init string
	    String fedinitstring = "--broker=mainbroker --federates="+ Integer.toString(count);
	    helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

	    // Set one second message interval
	    helics.helicsFederateInfoSetTimeProperty(fedinfo, 137, deltat);

	    helics.helicsFederateInfoSetIntegerProperty(fedinfo, 271, 1);

	    SWIGTYPE_p_void vFed = helics.helicsCreateValueFederate(name_prefix + "TestA Federate", fedinfo);

	    return vFed;
	}

	public static void test_value_federate_initialize(SWIGTYPE_p_void vFed) {
		federate_state status = helics.helicsFederateGetState(vFed);
		assert status.swigValue() == 0;
		helics.helicsFederateEnterExecutingMode(vFed);
		status = helics.helicsFederateGetState(vFed);
		assert status.swigValue() == 3;
	}

	public static void test_value_federate_publisher_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterTypePublication(vFed, "pub1",
								 "string", "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalTypePublication(vFed, "pub2",
				 "int", "");
		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterTypePublication(vFed, "pub3",
				 "double", "V");
		helics.helicsFederateEnterExecutingMode(vFed);
	}

	public static void test_value_federate_publication_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterPublication(vFed, "pub1",
				 helicsConstants.HELICS_DATA_TYPE_STRING, "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalPublication(vFed, "pub2",
				helicsConstants.HELICS_DATA_TYPE_INT, "");
		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterPublication(vFed, "pub3",
				helicsConstants.HELICS_DATA_TYPE_DOUBLE, "V");
		helics.helicsFederateEnterExecutingMode(vFed);

	    int maxLen = 100;
	    String pubKey = helics.helicsPublicationGetKey(pubid1);
	    assert pubKey == "TestA Federate/pub1";
	    String pubKey2 = helics.helicsPublicationGetKey(pubid2);
		assert pubKey2 == "pub2";
	    String pubKey3 = helics.helicsPublicationGetKey(pubid3);
		assert pubKey3 == "TestA Federate/pub3";
	    String pub_type = helics.helicsPublicationGetType(pubid3);
		assert pub_type == "double";
	    String pub_units = helics.helicsPublicationGetUnits(pubid3);
	    assert pub_units == "V";
	}

	// public static void test_value_federate_subscription_registration(SWIGTYPE_p_void vFed) {
	// 	SWIGTYPE_p_void subid1 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V");
	// 	SWIGTYPE_p_void subid2 = helics.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2",
	// 			helicsConstants.HELICS_DATA_TYPE_INT, "");
	// 	SWIGTYPE_p_void subid3 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub3", "double", "V");
	// 	helics.helicsFederateEnterExecutingMode(vFed);
	// }

	public static void test_value_federate_subscription_and_publication_registration(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid1 = helics.helicsFederateRegisterTypePublication(vFed, "pub1",
				"string", "");
		SWIGTYPE_p_void pubid2 = helics.helicsFederateRegisterGlobalTypePublication(vFed, "pub2",
				"int", "");

		SWIGTYPE_p_void pubid3 = helics.helicsFederateRegisterPublication(vFed, "pub3", 
				helicsConstants.HELICS_DATA_TYPE_INT, "V");

		// SWIGTYPE_p_void subid1 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V");
		// SWIGTYPE_p_void subid2 = helics.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2",
		// 		helicsConstants.HELICS_DATA_TYPE_INT, "");

		// SWIGTYPE_p_void subid3 = helics.helicsFederateRegisterOptionalSubscription(vFed, "sub3",
		// 		"double", "V");
	}

	public static void test_value_federate_single_transfer(SWIGTYPE_p_void vFed) {
		SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		"string", "");
		SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");

	    helics.helicsFederateEnterExecutingMode(vFed);

	    helics.helicsPublicationPublishString(pubid, "string1");

	    double requestTime = 0.0;
	    double timeout = helics.helicsFederateRequestTime(vFed, requestTime);
	    byte[] s = new byte[100];
		int[] len = new int[1];
	    helics.helicsInputGetString(subid, s, len);
		String sub_string = new String(s);
	    assert sub_string == "string1";
	}

	public static void test_value_federate_runFederateTestDouble(SWIGTYPE_p_void vFed) {
	    double defaultValue = 1.0;
	    double testValue = 2.0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		"double", "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");
	    helics.helicsInputSetDefaultDouble(subid, defaultValue);

	    helics.helicsFederateEnterExecutingMode (vFed);

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishDouble(pubid, testValue);

	    double value = helics.helicsInputGetDouble(subid);
	    assert value == defaultValue;

	    double requesttime = 0.0;
	    double timeout = helics.helicsFederateRequestTime (vFed, requesttime);
	    assert requesttime == 0.1;

	    value = helics.helicsInputGetDouble(subid);
	    assert value == testValue;

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishDouble(pubid, testValue + 1);
	    
	    timeout = helics.helicsFederateRequestTime (vFed, requesttime);
	    assert requesttime == 0.02;

	    value = helics.helicsInputGetDouble(subid);
	    assert value == testValue + 1;
	}

	public static void test_value_federate_runFederateTestComplex(SWIGTYPE_p_void vFed) {
	    double rDefaultValue = 1.0;
	    double iDefaultValue = 1.0;
	    double rTestValue = 2.0;
	    double iTestValue = 2.0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		"complex", "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");
	    helics.helicsInputSetDefaultComplex(subid, rDefaultValue, iDefaultValue);

	    helics.helicsFederateEnterExecutingMode (vFed);

	    // publish string1 at time=0.0;
	    helics.helicsPublicationPublishComplex(pubid, rTestValue, iTestValue);
	    double[] value1 = new double[1];
	    double[] value2 = new double[1];
	    helics.helicsInputGetComplex(subid, value1, value2);
	    assert value2[0] == iDefaultValue;

	    double requesttime = 0.0;
	    double timeout = helics.helicsFederateRequestTime (vFed, requesttime);
	    assert requesttime == 0.01;

	    helics.helicsInputGetComplex(subid, value1, value2);
	    assert value1[0] == rTestValue;
	    assert value2[1] == iTestValue;
	}


	public static void test_value_federate_runFederateTestInteger(SWIGTYPE_p_void vFed) {
	    double defaultValue = 1;
	    double testValue = 2;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", "int", "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");
	    //SWIGTYPE_p_int64_t pDf = null;
	    //helics.helicsSubscriptionSetDefaultInteger(subid, pDf);

	    helics.helicsFederateEnterExecutingMode (vFed);
	    //SWIGTYPE_p_int64_t pValue = null;
	    //helics.helicsPublicationPublishInteger(pubid, pValue);

	    long value = helics.helicsInputGetInteger(subid);
	    assert value == defaultValue;
	    double requesttime = 0.0;
	    double timeout = helics.helicsFederateRequestTime(vFed, requesttime);
	    assert requesttime == 0.01;

	    value = helics.helicsInputGetInteger(subid);
	    assert value == testValue;

	    //helics.helicsPublicationPublishInteger(pubid, pValue);
	    timeout = helics.helicsFederateRequestTime (vFed, requesttime);
	    assert requesttime == testValue;

	    value = helics.helicsInputGetInteger(subid);
	    assert value == testValue + 1;
	}

	public static void test_value_federate_runFederateTestString(SWIGTYPE_p_void vFed) {
	    String defaultValue = "String1";
	    String testValue = "String2";
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalTypePublication (vFed, "pub1",
	    		"string", "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");
	    helics.helicsInputSetDefaultString(subid, defaultValue);

	    helics.helicsFederateEnterExecutingMode(vFed);

	    // TODO: Fix error with the following function
	    helics.helicsPublicationPublishString(pubid, testValue);
	    byte[] value = new byte[100];
		int[] len = new int[1];
	    helics.helicsInputGetString(subid, value, len);
		String val = new String(value);
	    assert val == defaultValue;
	    double requesttime = 0.0;
	    double timeout = helics.helicsFederateRequestTime (vFed, requesttime);
	    assert requesttime == 0.01;

	    helics.helicsInputGetString(subid, value, len);
		String val2 = new String(value);
	    assert val2 == testValue;
	}

	public static void test_value_federate_runFederateTestVectorD(SWIGTYPE_p_void vFed) {
	    double[] defaultValue = {0, 1, 2};
	    double[] testValue = {3, 4, 5};
	    int len = 0;
	    SWIGTYPE_p_void pubid = helics.helicsFederateRegisterGlobalInput (vFed, "pub1", helicsConstants.HELICS_DATA_TYPE_VECTOR, "");
	    SWIGTYPE_p_void subid = helics.helicsFederateRegisterSubscription (vFed, "pub1", "");
	    helics.helicsInputSetDefaultVector(subid, defaultValue, 3);

	    helics.helicsFederateEnterExecutingMode(vFed);

	    // TODO: Fix error with the following function
	    double[] data = new double[100];
	    helics.helicsPublicationPublishVector(pubid, data, 100);

	    int[] actualLen = new int[1];
		SWIGTYPE_p_double pData = null;
	    helics.helicsInputGetVector(subid, pData, 3,actualLen);
	    //assert value == [0, 1, 2];
	    double requesttime = 0.0;
	    double timeout = helics.helicsFederateRequestTime(vFed, requesttime);
	    assert requesttime == 0.01;

	    double[] value = new double[1];
	    helics.helicsInputGetVector(subid, pData, 3,actualLen);
	    //assert value == [3, 4, 5]
	}

	public static void main(String[] args) {
		SWIGTYPE_p_void broker = AddBroker("zmq");
		String initstring = "--broker=";
		String id = helics.helicsBrokerGetIdentifier(broker);
		initstring = initstring + id;
		initstring = initstring + " --broker_address";
		String address = helics.helicsBrokerGetAddress(broker);
		helics.helicsBrokerDisconnect(broker);
		helics.helicsCloseLibrary();
	}

}
