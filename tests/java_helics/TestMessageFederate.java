import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics;
import com.java.helics.federate_state;
import com.java.helics.message_t;
import com.java.helics.helics_filter_type_t;
import com.java.helics.helics_time_properties;

public class TestMessageFederate {

	public static SWIGTYPE_p_void createMessageFederate(final String fedName, final double timeDelta) {
		String fedinitstring = "--broker=mainbroker --federates=1";
		// Create Federate Info object that describes the federate properties
		SWIGTYPE_p_void fi = helics.helicsCreateFederateInfo();
		// TIME_DELTA_PROPERTY = 137
		helics.helicsFederateInfoSetTimeProperty(fi, helics_time_properties.helics_time_property_time_delta.swigValue(), timeDelta);
		helics.helicsFederateInfoSetCoreTypeFromString(fi, "zmq");
		// Federate init string
		helics.helicsFederateInfoSetCoreInitString(fi, fedinitstring);
		SWIGTYPE_p_void msgFed = helics.helicsCreateMessageFederate(fedName, fi);
		// Wait for sometime

		return msgFed;
	}

	public static void test_message_federate_initialize(SWIGTYPE_p_void msgFederate) {
		int[] state = new int[1];
		federate_state status = helics.helicsFederateGetState(msgFederate);
		assert status.swigValue() == 0;

		// helics.helicsFederateEnterExecutingMode(msgFederate);

		status = helics.helicsFederateGetState(msgFederate);
		assert status.swigValue() == 2;
	}

	public static void test_message_federate_endpoint_registration(SWIGTYPE_p_void msgFederate) {
		federate_state status = helics.helicsFederateGetState(msgFederate);
		
		assert status.swigValue() == 0;
		SWIGTYPE_p_void epid1 = helics.helicsFederateRegisterEndpoint(msgFederate, "ep1", "");
		SWIGTYPE_p_void epid2 = helics.helicsFederateRegisterGlobalEndpoint(msgFederate, "ep2", "random");

		helics.helicsFederateEnterExecutingMode(msgFederate);
		assert status.swigValue() == 3;
		
		int maxLen = 100;
		// byte[] endpoint_name = new byte[100];
		String endpoint_name = helics.helicsEndpointGetName(epid1);

		// String ep = new String(endpoint_name);
		assert endpoint_name == "TestA Federate/ep1";

		endpoint_name = helics.helicsEndpointGetName(epid2);
		// String ep2 = new String(endpoint_name);
		assert endpoint_name == "ep2";

		String endpoint_type = helics.helicsEndpointGetType(epid1);
		// String et = new String(endpoint_type);
		assert endpoint_type == "";

		endpoint_type = helics.helicsEndpointGetType(epid2);
		// String et2 = new String(endpoint_type);
		assert endpoint_type == "random";

	}

	
	public static void main(String[] args) {
		String initString = "1 --name=mainbroker";
		// Create broker
		SWIGTYPE_p_void broker = helics.helicsCreateBroker("zmq", "", initString);
		int isConnected = helics.helicsBrokerIsConnected(broker);
		// if isConnected == 1:
		// Test message federate creation
		SWIGTYPE_p_void myFederate = TestMessageFederate.createMessageFederate("TestA Federate", 1);


		// Test endpoint registration
		test_message_federate_endpoint_registration(myFederate);

		//
		// Wait for sometime
		helics.helicsFederateFinalize(myFederate);
		federate_state state = helics.helicsFederateGetState(myFederate);
		assert state.swigValue() == 3;
		while (helics.helicsBrokerIsConnected(broker) == 1) {
			try {
				Thread.sleep(1000); // 1000 milliseconds is one second.
			} catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
			}
		}
		helics.helicsFederateFree(myFederate);
		helics.helicsCloseLibrary();

		// Test
	}

}
