import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics;
import com.java.helics.federate_state;
import com.java.helics.message_t;

public class TestMessageFederate {
	
	public static SWIGTYPE_p_void createMessageFederate(final String fedName, final double timeDelta) {
		String fedinitstring = "--broker=mainbroker --federates=1";
		// Create Federate Info object that describes the federate properties 
		SWIGTYPE_p_void fi = helics.helicsCreateFederateInfo();
		//TIME_DELTA_PROPERTY = 137
		helics.helicsFederateInfoSetTimeProperty(fi, 137, timeDelta);
	    helics.helicsFederateInfoSetCoreTypeFromString(fi, "zmq");
		// Federate init string 
		helics.helicsFederateInfoSetCoreInitString(fi, fedinitstring);
		SWIGTYPE_p_void msgFed = helics.helicsCreateMessageFederate(fedName, fi);
		//Wait for sometime
		
		return msgFed;
	}

	public static void test_message_federate_initialize(SWIGTYPE_p_void msgFederate) {
		int[] state = new int[1];
	    federate_state status = helics.helicsFederateGetState(msgFederate);
	    assert status.swigValue() == 0;

	    helics.helicsFederateEnterExecutingMode(msgFederate);

	    status = helics.helicsFederateGetState(msgFederate);
	    assert status.swigValue() == 3;		
	}
	
    public static void test_message_federate_endpoint_registration(SWIGTYPE_p_void msgFederate) {
    	SWIGTYPE_p_void epid1 = helics.helicsFederateRegisterEndpoint(msgFederate, 
				"ep1", "");
		SWIGTYPE_p_void epid2 = helics.helicsFederateRegisterGlobalEndpoint(msgFederate, 
						"ep2", "random");
		
		helics.helicsFederateEnterExecutingMode(msgFederate);
		int maxLen = 100;
		//byte[] endpoint_name = new byte[100];
		String endpoint_name = helics.helicsEndpointGetName(epid1);
		
		// String ep = new String(endpoint_name);
		assert endpoint_name == "TestA Federate/ep1";
		
		endpoint_name = helics.helicsEndpointGetName(epid2);
		//assert status == helics_status.helics_ok;
		// String ep2 = new String(endpoint_name);
		// assert ep2 == "ep2";
		
	    String endpoint_type = helics.helicsEndpointGetType(epid1);
		// String et = new String(endpoint_type);
		assert endpoint_type == "";
		
		endpoint_type = helics.helicsEndpointGetType(epid2);
		// String et2 = new String(endpoint_type);
		assert endpoint_type == "random";
    	
    }
    
    public static void test_message_federate_endpoint_registrationxx(SWIGTYPE_p_void msgFederate) {
    	SWIGTYPE_p_void epid1 = helics.helicsFederateRegisterEndpoint(msgFederate, 
				"ep1", "");
		SWIGTYPE_p_void epid2 = helics.helicsFederateRegisterGlobalEndpoint(msgFederate, 
						"ep2", "random");
		double timeDelta = 1.0;
		//TIME_DELTA_PROPERTY = 137
		helics.helicsFederateInfoSetTimeProperty(msgFederate, 137, timeDelta);
        helics.helicsFederateEnterExecutingMode(msgFederate);

        //String data = "random-data";
		SWIGTYPE_p_void data = null;
		helics.helicsEndpointSendEventRaw(epid1, "ep2", data, 10, 1.0);
        
        double request_time = 1.0;
        double timeout = helics.helicsFederateRequestTime(msgFederate, request_time);

        int res = helics.helicsFederateHasMessage(msgFederate);
        assert res == 1;

        res = helics.helicsEndpointHasMessage(epid1);
        // TODO: Figure out why this is returning zero
        assert res == 0;

        res = helics.helicsEndpointHasMessage (epid2);
        assert res == 1;

        // This causes a segfault
       message_t message = helics.helicsEndpointGetMessage(epid2);

//        assert message.getData() == String("random-data");
//        assertEquals(message.length, 11);
//        assertEquals(message.original_dest, String(""));
//        assertEquals(message.getOriginal_source(), String("TestA Federate/ep1"));
//        assertEquals(message.getSource(), String("TestA Federate/ep1"));
//        assertEquals(message.getTime(), 1.0);
    }
        
	public static void main(String[] args) {
		String initString = "1 --name=mainbroker";
		// Create broker
		SWIGTYPE_p_void broker = helics.helicsCreateBroker("zmq", "", initString);
		int isConnected = helics.helicsBrokerIsConnected(broker);
		//if isConnected == 1:
		// Test message federate creation
		SWIGTYPE_p_void myFederate = TestMessageFederate.createMessageFederate("TestA Federate", 1);
		
		// Test message federate intialization
		test_message_federate_initialize(myFederate);
		
		// Test endpoint registration
		test_message_federate_endpoint_registration(myFederate);
		
		//
		// Wait for sometime
		helics.helicsFederateFinalize(myFederate);
		federate_state state = helics.helicsFederateGetState(myFederate);
		assert state.swigValue() == 3;
	    while (helics.helicsBrokerIsConnected(broker) == 1) {
	    	try {
	    	    Thread.sleep(1000);                 //1000 milliseconds is one second.
	    	} catch(InterruptedException ex) {
	    	    Thread.currentThread().interrupt();
	    	}
	    }
	    helics.helicsFederateFree(myFederate);
	    helics.helicsCloseLibrary();
	    
	    // Test 
	}
	
}
