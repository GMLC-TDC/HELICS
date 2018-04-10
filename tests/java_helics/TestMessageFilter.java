import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics;
import com.java.helics.helics_status;
import com.java.helics.helics_filter_type_t;

public class TestMessageFilter {
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
	    helics_status status = helics.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate");

	    // Set core type from string 
	    status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq");

	    // Federate init string 
	    String fedinitstring = "--broker=mainbroker --federates="+ Integer.toString(count);
	    status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

	    // Set one second message interval 
	    status = helics.helicsFederateInfoSetTimeDelta(fedinfo, deltat);

	    status = helics.helicsFederateInfoSetLoggingLevel(fedinfo, 1);

	    SWIGTYPE_p_void mFed = helics.helicsCreateMessageFederate(fedinfo);
	    
	    return mFed;
	}
	
	public static void FreeFederate(SWIGTYPE_p_void fed) {
	    helics_status status = helics.helicsFederateFinalize(fed);
	    int[] state = new int[1];
	    status = helics.helicsFederateGetState(fed, state);
	    assert state[0] == 3;

	    helics.helicsFederateFree(fed);
	}

	
	public static void test_message_filter_registration(SWIGTYPE_p_void broker) {
		SWIGTYPE_p_void fFed = AddFederate(broker, "zmq", 1, 1, "filter");
		SWIGTYPE_p_void mFed = AddFederate(broker, "zmq", 1, 1, "message");

	    helics.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "");
	    helics.helicsFederateRegisterGlobalEndpoint(mFed, "port2", "");

	    SWIGTYPE_p_void f1 = helics.helicsFederateRegisterSourceFilter (fFed, helics_filter_type_t.helics_custom_filter, "filter1", "port1");
	    SWIGTYPE_p_void f2 = helics.helicsFederateRegisterDestinationFilter (fFed, helics_filter_type_t.helics_custom_filter, "filter2", "port2");
	    SWIGTYPE_p_void ep1 = helics.helicsFederateRegisterEndpoint (fFed, "fout", "");
	    SWIGTYPE_p_void f3 = helics.helicsFederateRegisterSourceFilter (fFed, helics_filter_type_t.helics_custom_filter, "", "filter0/fout");

	    FreeFederate(fFed);
	    FreeFederate(mFed);
	}

	
	public static void main(String[] args) {
		SWIGTYPE_p_void broker = AddBroker("zmq");
		String initstring = "--broker=";
		String identifier = "";
		helics_status status = helics.helicsBrokerGetIdentifier(broker, identifier, 100);
		assert status == helics_status.helics_ok;
		initstring = initstring + identifier;
		initstring = initstring + " --broker_address";
		String address = "";
		status = helics.helicsBrokerGetAddress(broker, address, 100);
		assert status == helics_status.helics_ok;
		test_message_filter_registration(broker);
		helics.helicsBrokerDisconnect(broker);
		helics.helicsCloseLibrary();
	}
	
}
