import com.java.helics.helics;
import com.java.helics.SWIGTYPE_p_double;
import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics_status;
import java.util.concurrent.TimeUnit;
public class HelloWorld {
	public static void main(String[] args) {
		System.loadLibrary("JNIhelics");
		//System.loadLibrary("helicsSharedLib");

		System.out.println(helics.helicsGetVersion());
		System.out.println("pi_receiver_java");
		SWIGTYPE_p_void fi = helics.helicsFederateInfoCreate();
		String coreInit="--federates=1";
		String fedName="TestB Federate";
		String coreName="zmq";
		double deltat=0.01;
		double currenttime=0.0;
		double value = 0.0;
		double[] val={0.0};
		double[] grantedtime={0.0};
		System.out.println(coreInit);
		helics_status status = helics.helicsFederateInfoSetFederateName(fi, fedName);
		System.out.println(status);
		status = helics.helicsFederateInfoSetCoreTypeFromString(fi, coreName);
		System.out.println(status);
		status = helics.helicsFederateInfoSetCoreInitString(fi, coreInit);
		System.out.println(status);
		status = helics.helicsFederateInfoSetTimeDelta(fi, deltat);
		System.out.println(status);
		status = helics.helicsFederateInfoSetLoggingLevel(fi, 1);
		System.out.println(status);
		SWIGTYPE_p_void vFed = helics.helicsCreateValueFederate(fi);

		SWIGTYPE_p_void sub = helics.helicsFederateRegisterSubscription(vFed, "testA", "double","");

		status = helics.helicsFederateEnterInitializationMode(vFed);
		System.out.println(status);

		status = helics.helicsFederateEnterExecutionMode(vFed);
		System.out.println(status);

		while(currenttime < 0.20) {
			status = helics.helicsFederateRequestTime(vFed, currenttime, grantedtime );
			try {
                TimeUnit.SECONDS.sleep(5);
            }
            catch(InterruptedException e) {
                System.out.println("exception occurred");
            }
            int isupdated = helics.helicsSubscriptionIsUpdated(sub);
			currenttime=grantedtime[0];
			if(isupdated==1) {
			      /* NOTE: The value sent by sender at time t is received by receiver at time t+deltat */
				  status = helics.helicsSubscriptionGetDouble(sub, val);

				  System.out.printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",val[0],currenttime);

			}
		}
		status = helics.helicsFederateFinalize(vFed);
		helics.helicsCloseLibrary();
	}
}

