import com.java.helics.helics;
import com.java.helics.SWIGTYPE_p_double;
import com.java.helics.SWIGTYPE_p_void;
import com.java.helics.helics_status;
import java.util.concurrent.TimeUnit;
public class HelloWorld {
	public static void main(String[] args) {
		System.loadLibrary("JNIhelics");
		// System.loadLibrary("helicsSharedLib");

		System.out.println(helics.helicsGetVersion());

		SWIGTYPE_p_void fi = helics.helicsFederateInfoCreate();
		String coreInit="--federates=1";
		String fedName="TestA Federate";
		String coreName="zmq";
		double deltat=0.01;
		double currenttime=0.0;
		double value = 0.0;
		double[] val={0.0};
		double[] grantedtime={0.0};
		System.out.println(coreInit);
		helics_status status = helics.helicsFederateInfoSetFederateName(fi, fedName);
		// System.out.println(status);
		status = helics.helicsFederateInfoSetCoreTypeFromString(fi, coreName);
		// System.out.println(status);
		status = helics.helicsFederateInfoSetCoreInitString(fi, coreInit);
		// System.out.println(status);
		status = helics.helicsFederateInfoSetTimeDelta(fi, deltat);
		// System.out.println(status);
		status = helics.helicsFederateInfoSetLoggingLevel(fi, 1);
		// System.out.println(status);
		SWIGTYPE_p_void vFed = helics.helicsCreateValueFederate(fi);

		// SWIGTYPE_p_void sub = helics.helicsFederateRegisterSubscription(vFed, "testB", "double","");
		SWIGTYPE_p_void pub = helics.helicsFederateRegisterPublication(vFed, "testA", "double","");

		status = helics.helicsFederateEnterInitializationMode(vFed);
		// System.out.println(status);

		status = helics.helicsFederateEnterExecutionMode(vFed);
		// System.out.println(status);

        for (int t = 5; t <= 10; x++) {
			status = helics.helicsFederateRequestTime(vFed, currenttime, grantedtime );
			try {
                TimeUnit.SECONDS.sleep(1);
            }
            catch(InterruptedException e) {
                System.out.println("exception occurred");
            }
            status = helics.helicsPublicationPublishDouble(pub, val);
			currenttime=grantedtime[0];
		}

		status = helics.helicsFederateFinalize(vFed);
		helics.helicsCloseLibrary();
	}
}


