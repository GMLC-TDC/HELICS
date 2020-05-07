//import java.lang.reflect.InvocationTargetException;
//import java.lang.reflect.Method;
//import java.util.ArrayList;
//import java.util.List;

import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics;

public class ApplicationApiTest {

    public static SWIGTYPE_p_void createValueFederate(final String fedName, final double timeDelta) {
        SWIGTYPE_p_void fi = helics.helicsFederateInfoCreate();
        helics.helicsFederateInfoSetFederateName(fi,fedName);
        helics.helicsFederateInfoSetTimeDelta(fi, timeDelta);
        helics.helicsFederateInfoSetCoreTypeFromString(fi, "zmq");
        helics.helicsFederateInfoSetCoreInitString(fi, "1");
        SWIGTYPE_p_void valFed = helics.helicsCreateValueFederate(fi);
        return valFed;
    }

    public static void main(String[] args) {
        SWIGTYPE_p_void broker = helics.helicsCreateBroker("zmq", "", "1");
        SWIGTYPE_p_void myFederate = ApplicationApiTest.createValueFederate("javaFederate", 1);
        SWIGTYPE_p_void fedPublication = helics.helicsFederateRegisterPublication(myFederate, "pub1", "integer", null);
        SWIGTYPE_p_void fedSubscription = helics.helicsFederateRegisterOptionalSubscription(myFederate,"player/pub", "integer", null);
        helics.helicsFederateEnterInitializationMode(myFederate);
        System.out.println(String.format("%s has successfully joined the co-simulation.", "javaFederate"));
        // do any necessary initialization for your federate
        Integer pubVal = 0;
        long[] subVal = {0};
        // enter execution state when you are ready to begin moving in time.
        helics.helicsFederateEnterExecutionMode(myFederate);
        for (int i = 0; i < 10; i++) {
            // its a good practice to check for new subscription messages at the beginning of each timestep
            if (helics.helicsSubscriptionIsUpdated(fedSubscription) == 1) {
                helics.helicsSubscriptionGetInteger(fedSubscription, subVal);
                System.out.println(String.format("%s received a new value on topic, player/pub. Value = %d.", "javaFederate", subVal[0]));
            }
            //do any necessary functionality with the subscription values you received
            pubVal = (int) subVal[0];
            pubVal = pubVal + (i+1)*2;
            // its a good practice to publish your publications at the end of your timestep loop.
            helics.helicsPublicationPublishString(fedPublication, pubVal.toString());
            // now we can request to go to the next timestep
            double[] returnTime = {0};
            double requestTime = (double)(i+1);
            helics.helicsFederateRequestTime(myFederate, requestTime, returnTime);
            System.out.println(String.format("finished timestep %d. moving to time step %d", i, (int)(returnTime[0])));
        }
        // we have exited our time loop so we are done simulating. Lets exit the co-simulation.
        rv = helics.helicsFederateFinalize(myFederate);
        helics.helicsBrokerDestroy(broker);
        System.out.println(String.format("%s has successfully exited the co-simulation.", "javaFederate"));
        helics.helicsCloseLibrary();
    }
}
