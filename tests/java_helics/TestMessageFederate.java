import java.lang.reflect.Method;
import java.util.*;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics;
import com.java.helics.helics_status;
import com.java.helics.message_t;

public class TestMessageFederate {
	public static SWIGTYPE_p_void createMessageFederate(final String fedName, final double timeDelta) {
		String fedinitstring = "--broker=mainbroker --federates=1";
		// Create Federate Info object that describes the federate properties 
		SWIGTYPE_p_void fi = helics.helicsFederateInfoCreate();
		helics_status rv = helics.helicsFederateInfoSetFederateName(fi,fedName);
		rv = helics.helicsFederateInfoSetTimeDelta(fi, timeDelta);
		rv = helics.helicsFederateInfoSetCoreTypeFromString(fi, "zmq");
		// Federate init string 
	    helics_status status = helics.helicsFederateInfoSetCoreInitString(fi, fedinitstring);
		SWIGTYPE_p_void msgFed = helics.helicsCreateMessageFederate(fi);
		//Wait for sometime
		
		return msgFed;
	}
	
	public static void test_message_federate_initialize(SWIGTYPE_p_void msgFederate) {
		int[] state = new int[1];
	    helics_status status = helics.helicsFederateGetState(msgFederate, state);
	    assert state[0] == 0;

	    helics.helicsFederateEnterExecutionMode(msgFederate);

	    status = helics.helicsFederateGetState(msgFederate, state);
	    assert state[0] == 2;		
	}
	
    public static void test_message_federate_endpoint_registration(SWIGTYPE_p_void msgFederate) {
    	SWIGTYPE_p_void epid1 = helics.helicsFederateRegisterEndpoint(msgFederate, 
				"ep1", "");
		SWIGTYPE_p_void epid2 = helics.helicsFederateRegisterGlobalEndpoint(msgFederate, 
						"ep2", "random");
		
		helics.helicsFederateEnterExecutionMode(msgFederate);
		int maxLen = 100;
		byte[] endpoint_name = new byte[100];
		helics_status status = helics.helicsEndpointGetName(epid1, endpoint_name);
		assert status == helics_status.helics_ok;
		String ep = new String(endpoint_name);
		assert ep == "TestA Federate/ep1";
		
		status = helics.helicsEndpointGetName(epid2, endpoint_name);
		assert status == helics_status.helics_ok;
		String ep2 = new String(endpoint_name);
		assert ep2 == "ep2";
		
		byte[] endpoint_type = new byte[100];
		status = helics.helicsEndpointGetType(epid1, endpoint_type);
		assert status == helics_status.helics_ok;
		String et = new String(endpoint_type);
		assert et == "";
		
		status = helics.helicsEndpointGetType(epid2, endpoint_type);
		assert status == helics_status.helics_ok;
		String et2 = new String(endpoint_type);
		assert et2 == "random";
    	
    }
    
    public static void test_message_federate_endpoint_registrationxx(SWIGTYPE_p_void msgFederate) {
    	SWIGTYPE_p_void epid1 = helics.helicsFederateRegisterEndpoint(msgFederate, 
				"ep1", "");
		SWIGTYPE_p_void epid2 = helics.helicsFederateRegisterGlobalEndpoint(msgFederate, 
						"ep2", "random");
		double timeDelta = 1.0;
		helics_status status = helics.helicsFederateInfoSetTimeDelta(msgFederate, timeDelta);
        helics.helicsFederateEnterExecutionMode(msgFederate);

        //String data = "random-data";
		SWIGTYPE_p_void data = null;
        status = helics.helicsEndpointSendEventRaw(epid1, "ep2", data, 10, 1.0);
        
        double request_time = 1.0;
        double[] timeout = {1.0};
        status = helics.helicsFederateRequestTime(msgFederate, request_time, timeout);

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
		helics_status status = helics.helicsFederateFinalize(myFederate);
		int[] state = new int[1];
		status =  helics.helicsFederateGetState(myFederate, state);
	    assert state[0] == 3;
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
