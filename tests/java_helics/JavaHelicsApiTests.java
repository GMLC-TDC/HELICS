import java.util.ArrayList;
import java.util.List;

import com.java.helics.*;
public class JavaHelicsApiTests {

	public List<AssertionError> failedHelicsTests = new ArrayList<AssertionError>();
	public int numberOfPassingTests = 0;
	public int numberOfFailedTests = 0;
	public void helicsAssert(String errorMessage) {
		AssertionError err = new AssertionError(errorMessage);
		failedHelicsTests.add(err);
		err.printStackTrace();
		--numberOfPassingTests;
		++numberOfFailedTests;
	}

	JavaHelicsApiTests(int numberOfTests) {
		numberOfPassingTests = numberOfTests;
	}

	public static void main(String[] args) {
		JavaHelicsApiTests javaHelicsApiTests = new JavaHelicsApiTests(94);
		try {
			//General HELICS Functions
			double helicsTimeZero = helics.getHelics_time_zero();
			if(helicsTimeZero != 0.0) {
				javaHelicsApiTests.helicsAssert("helicsTimeZero != 0.0.");
			}
			double helicsTimeEpsilon = helics.getHelics_time_epsilon();
			if(helicsTimeEpsilon != 1.0e-9) {
				javaHelicsApiTests.helicsAssert("helicsTimeEpsilon != 1.0E-9");
			}
			int helicsTrue = helics.getHelics_true();
			if(helicsTrue != 1) {
				javaHelicsApiTests.helicsAssert("helicsTrue != 1");
			}
			int helicsFalse = helics.getHelics_false();
			if(helicsFalse != 0) {
				javaHelicsApiTests.helicsAssert("helicsFalse != 0");
			}
			String helicsVersion = helics.helicsGetVersion();
			if(helicsVersion == null) {
				javaHelicsApiTests.helicsAssert("helicsVersion == null");
			}
			int helicsCoreTypeIsAvailabe = helics.helicsIsCoreTypeAvailable("zmq");
			if(helicsCoreTypeIsAvailabe != 1) {
				javaHelicsApiTests.helicsAssert("helicsCoreTypeIsAvailabe != 1");
			}
			//Broker API Functions
			SWIGTYPE_p_void broker1 = helics.helicsCreateBroker("zmq", "broker1", "--federates 3 --loglevel 1");
			if(broker1 == null) {
				javaHelicsApiTests.helicsAssert("broker1 == null");
			}
			SWIGTYPE_p_void broker2 = helics.helicsBrokerClone(broker1);
			if(broker2 == null) {
				javaHelicsApiTests.helicsAssert("broker2 == null");
			}
			byte[] broker1Address = new byte[256];
			helics_status rv = helics.helicsBrokerGetAddress(broker1, broker1Address);
			String broker1AddressString = new String(broker1Address);
			if(!broker1AddressString.contains("tcp://127.0.0.1:23404")) {
				javaHelicsApiTests.helicsAssert("!broker1AddressString.equals(\"tcp://127.0.0.1:23404\")");
			}
			byte[] broker1Identifier = new byte[10];
			rv = helics.helicsBrokerGetIdentifier(broker1, broker1Identifier);
			String broker1IdentifierString = new String(broker1Identifier);
			if(!broker1IdentifierString.contains("broker1")) {
				javaHelicsApiTests.helicsAssert("!broker1IdentifierString.equals(\"broker1\")");
			}
			int broker1IsConnected = helics.helicsBrokerIsConnected(broker1);
			if(broker1IsConnected != 1) {
				javaHelicsApiTests.helicsAssert("broker1IsConnected != 1");
			}
			rv = helics.helicsBrokerDisconnect(broker1);
			broker1IsConnected = helics.helicsBrokerIsConnected(broker1);
			if(broker1IsConnected != 0) {
				javaHelicsApiTests.helicsAssert("broker1IsConnected != 0");
			}
			rv = helics.helicsBrokerDisconnect(broker2);
			helics.helicsBrokerFree(broker1);
			helics.helicsBrokerFree(broker2);
			helics.helicsCloseLibrary();
			//Core API Functions
			SWIGTYPE_p_void core1 = helics.helicsCreateCore("zmq", "core1", "--federates 3 --port 5570");
			if(core1 == null) {
				javaHelicsApiTests.helicsAssert("core1 == null");
			}
			SWIGTYPE_p_void core2 = helics.helicsCoreClone(core1);
			if(core2 == null) {
				javaHelicsApiTests.helicsAssert("core2 == null");
			}
			byte[] core1Identifier = new byte[10];
			rv = helics.helicsCoreGetIdentifier(core1, core1Identifier);
			String core1IdentifierString = new String(core1Identifier);
			if(!core1IdentifierString.contains("core1")) {
				javaHelicsApiTests.helicsAssert("!core1IdentifierString.equals(\"core1\")");
			}
			SWIGTYPE_p_void sourceFilter1 = helics.helicsCoreRegisterSourceFilter(core1, helics_filter_type_t.helics_delay_filter, "ep1", "core1SourceFilter");
			if(sourceFilter1 == null) {
				javaHelicsApiTests.helicsAssert("sourceFilter1 == null");
			}
			SWIGTYPE_p_void destinationFilter1 = helics.helicsCoreRegisterDestinationFilter(core1,helics_filter_type_t.helics_delay_filter, "ep2", "core1DestinationFilter");
			if(destinationFilter1 == null) {
				javaHelicsApiTests.helicsAssert("destinationFilter1 == null");
			}
			SWIGTYPE_p_void cloningFilter1 = helics.helicsCoreRegisterCloningFilter(core1, "ep3");
			if(cloningFilter1 == null) {
				javaHelicsApiTests.helicsAssert("cloningFilter1 == null");
			}
			int core1IsConnected = helics.helicsCoreIsConnected(core1);
			if(core1IsConnected != 0) {
				javaHelicsApiTests.helicsAssert("core1IsConnected != 0");
			}
			rv = helics.helicsCoreSetReadyToInit(core1);
			rv = helics.helicsCoreDisconnect(core1);
			rv = helics.helicsCoreDisconnect(core2);
			helics.helicsCoreFree(core1);
			helics.helicsCoreFree(core2);
			helics.helicsCloseLibrary();
			//Federate Info API Functions
			SWIGTYPE_p_void fedInfo1 = helics.helicsFederateInfoCreate();
			if(fedInfo1 == null) {
				javaHelicsApiTests.helicsAssert("fedInfo1 == null");
			}
			rv = helics.helicsFederateInfoSetCoreInitString(fedInfo1, "1");
			rv = helics.helicsFederateInfoSetCoreName(fedInfo1, "core3");
			rv = helics.helicsFederateInfoSetCoreType(fedInfo1, 3);
			rv = helics.helicsFederateInfoSetCoreTypeFromString(fedInfo1, "zmq");
			rv = helics.helicsFederateInfoSetFederateName(fedInfo1, "fed1");
			rv = helics.helicsFederateInfoSetFlag(fedInfo1, 1, helics.getHelics_true());
			rv = helics.helicsFederateInfoSetInputDelay(fedInfo1, 1.0);
			rv = helics.helicsFederateInfoSetLoggingLevel(fedInfo1, 1);
			rv = helics.helicsFederateInfoSetMaxIterations(fedInfo1, 100);
			rv = helics.helicsFederateInfoSetOutputDelay(fedInfo1, 1.0);
			rv = helics.helicsFederateInfoSetPeriod(fedInfo1, 1.0);
			rv = helics.helicsFederateInfoSetTimeDelta(fedInfo1, 1.0);
			rv = helics.helicsFederateInfoSetTimeOffset(fedInfo1, 0.1);
			helics.helicsFederateInfoFree(fedInfo1);
			//Federate API Functions
			SWIGTYPE_p_void broker3 = helics.helicsCreateBroker("zmq", "broker3", "--federates 1 --loglevel 1");
			SWIGTYPE_p_void fedInfo2 = helics.helicsFederateInfoCreate();
			String coreInitString = "--federates 1";
			rv = helics.helicsFederateInfoSetCoreInitString(fedInfo2, coreInitString);
			rv = helics.helicsFederateInfoSetCoreTypeFromString(fedInfo2, "zmq");
			rv = helics.helicsFederateInfoSetFederateName(fedInfo2, "fed1");
			rv = helics.helicsFederateInfoSetLoggingLevel(fedInfo2, 1);
			rv = helics.helicsFederateInfoSetTimeDelta(fedInfo2, 1.0);
			SWIGTYPE_p_void fed1 = helics.helicsCreateCombinationFederate(fedInfo2);
			if(fed1 == null) {
				javaHelicsApiTests.helicsAssert("fed1 == null");
			}
			SWIGTYPE_p_void fed2 = helics.helicsFederateClone(fed1);
			if(fed2 == null) {
				javaHelicsApiTests.helicsAssert("fed2 == null");
			}
			SWIGTYPE_p_void fed3 = helics.helicsGetFederateByName("fed1");
			if(fed3 == null) {
				javaHelicsApiTests.helicsAssert("fed3 == null");
			}
			rv = helics.helicsFederateSetFlag(fed2, 1, helics.getHelics_false());
			rv = helics.helicsFederateSetInputDelay(fed2, 1.0);
			rv = helics.helicsFederateSetLoggingLevel(fed1, 1);
			rv = helics.helicsFederateSetMaxIterations(fed2, 100);
			rv = helics.helicsFederateSetOutputDelay(fed2, 1.0);
			rv = helics.helicsFederateSetPeriod(fed2, 1.0, 0.0);
			rv = helics.helicsFederateSetTimeDelta(fed2, 1.0);
			SWIGTYPE_p_void fed1CloningFilter = helics.helicsFederateRegisterCloningFilter(fed1, "fed1/Ep1");
			if(fed1CloningFilter == null) {
				javaHelicsApiTests.helicsAssert("fed1CloningFilter == null");
			}
			SWIGTYPE_p_void fed1DestinationFilter = helics.helicsFederateRegisterDestinationFilter(fed1, helics_filter_type_t.helics_delay_filter, "ep2", "fed1DestinationFilter");
			if(fed1DestinationFilter == null) {
				javaHelicsApiTests.helicsAssert("fed1DestinationFilter == null");
			}
			SWIGTYPE_p_void ep1 = helics.helicsFederateRegisterEndpoint(fed1, "Ep1", "string");
			if(ep1 == null) {
				javaHelicsApiTests.helicsAssert("ep1 == null");
			}
			SWIGTYPE_p_void ep2 = helics.helicsFederateRegisterGlobalEndpoint(fed1, "Ep2", "string");
			if(ep2 == null) {
				javaHelicsApiTests.helicsAssert("ep2 == null");
			}
			SWIGTYPE_p_void pub1 = helics.helicsFederateRegisterGlobalPublication(fed1, "pub1", "double", null);
			if(pub1 == null) {
				javaHelicsApiTests.helicsAssert("pub1 == null");
			}
			SWIGTYPE_p_void pub2 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub2", 3, null);
			if(pub2 == null) {
				javaHelicsApiTests.helicsAssert("pub2 == null");
			}
			SWIGTYPE_p_void sub1 = helics.helicsFederateRegisterOptionalSubscription(fed1, "pub1", "double", null);
			if(sub1 == null) {
				javaHelicsApiTests.helicsAssert("sub1 == null");
			}
			SWIGTYPE_p_void sub2 = helics.helicsFederateRegisterOptionalTypeSubscription(fed1,"pub2", 3, null);
			if(sub2 == null) {
				javaHelicsApiTests.helicsAssert("sub2 == null");
			}
			SWIGTYPE_p_void pub3 = helics.helicsFederateRegisterPublication(fed1, "pub3", "string", null);
			if(pub3 == null) {
				javaHelicsApiTests.helicsAssert("pub3 == null");
			}
			byte[] pub1Key = new byte[256];
			byte[] pub1Type = new byte[256];
			byte[] pub1Units = new byte[256];
			byte[] sub1Key = new byte[256];
			byte[] sub1Units = new byte[256];
			rv = helics.helicsPublicationGetKey(pub1, pub1Key);
			rv = helics.helicsPublicationGetType(pub1, pub1Type);
			rv = helics.helicsPublicationGetUnits(pub1, pub1Units);
			rv = helics.helicsSubscriptionGetKey(sub1, sub1Key);
			rv = helics.helicsSubscriptionGetUnits(sub1, sub1Units);
			String pub1KeyString = new String(pub1Key);
			if(!pub1KeyString.contains("pub1")) {
				javaHelicsApiTests.helicsAssert("!pub1KeyString.equals(\"pub1\")");
			}
			String pub1TypeString = new String(pub1Type);
			if(!pub1TypeString.contains("double")) {
				javaHelicsApiTests.helicsAssert("!pub1KeyString.equals(\"double\")");
			}
			String pub1UnitsString = new String(pub1Units);
			if(!pub1UnitsString.contains("")) {
				javaHelicsApiTests.helicsAssert("!pub1UnitsString.equals(\"\")");
			}
			String sub1KeyString = new String(sub1Key);
			if(!sub1KeyString.contains("pub1")) {
				javaHelicsApiTests.helicsAssert("!sub1KeyString.equals(\"pub1\")");
			}
			String sub1UnitsString = new String(sub1Units);
			if(!sub1UnitsString.contains("")) {
				javaHelicsApiTests.helicsAssert("!sub1UnitsString.equals(\"\")");
			}
			if(!sub1UnitsString.contains("")) {
				javaHelicsApiTests.helicsAssert("!sub1UnitsString.equals(\"\")");
			}
			SWIGTYPE_p_void fed1SourceFilter = helics.helicsFederateRegisterSourceFilter(fed1, helics_filter_type_t.helics_delay_filter, "Ep2", "fed1SourceFilter");
			if(fed1SourceFilter == null) {
				javaHelicsApiTests.helicsAssert("fed1SourceFilter == null");
			}
			rv = helics.helicsFilterAddDeliveryEndpoint(fed1SourceFilter, "fed1/Ep1");
			rv = helics.helicsFilterRemoveDeliveryEndpoint(fed1SourceFilter, "fed1/Ep1");
			rv = helics.helicsFilterAddDestinationTarget(fed1SourceFilter, "fed1/Ep1");
			rv = helics.helicsFilterRemoveDestinationTarget(fed1SourceFilter, "fed1/Ep1");
			rv = helics.helicsFilterAddSourceTarget(fed1SourceFilter, "Ep2");
			rv = helics.helicsFilterRemoveSourceTarget(fed1SourceFilter, "Ep2");
			byte[] fed1SourceFilterName = new byte[256];
			byte[] fed1SourceFilterTarget = new byte[256];
			rv = helics.helicsFilterGetName(fed1SourceFilter, fed1SourceFilterName);
			rv = helics.helicsFilterGetTarget(fed1SourceFilter, fed1SourceFilterTarget);
			String fed1SourceFilterNameString = new String(fed1SourceFilterName);
			if(!fed1SourceFilterNameString.contains("fed1SourceFilter")) {
				javaHelicsApiTests.helicsAssert("!fed1SourceFilterNameString.contains(\"fed1SourceFilter\")");
			}
			String fed1SourceFilterTargetString = new String(fed1SourceFilterTarget);
			if(!fed1SourceFilterTargetString.contains("Ep2")) {
				javaHelicsApiTests.helicsAssert("!fed1SourceFilterTargetString.contains(\"Ep2\")");
			}
			SWIGTYPE_p_void sub3 = helics.helicsFederateRegisterSubscription(fed1, "fed1/pub3", "string", null);
			if(sub3 == null) {
				javaHelicsApiTests.helicsAssert("sub3 == null");
			}
			SWIGTYPE_p_void pub4 = helics.helicsFederateRegisterTypePublication(fed1, "pub4", 2, null);
			if(pub4 == null) {
				javaHelicsApiTests.helicsAssert("pub4 == null");
			}
			SWIGTYPE_p_void sub4 = helics.helicsFederateRegisterTypeSubscription(fed1, "fed1/pub4", 2, null);
			if(sub4 == null) {
				javaHelicsApiTests.helicsAssert("sub4 == null");
			}
			SWIGTYPE_p_void pub5 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub5", 7, null);
			if(pub5 == null) {
				javaHelicsApiTests.helicsAssert("pub5 == null");
			}
			SWIGTYPE_p_void sub5 = helics.helicsFederateRegisterTypeSubscription(fed1, "pub5", 7, null);
			if(sub5 == null) {
				javaHelicsApiTests.helicsAssert("sub5 == null");
			}
			SWIGTYPE_p_void pub6 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub6", 4, null);
			if(pub6 == null) {
				javaHelicsApiTests.helicsAssert("pub6 == null");
			}
			SWIGTYPE_p_void sub6 = helics.helicsFederateRegisterTypeSubscription(fed1, "pub6", 4, null);
			if(sub6 == null) {
				javaHelicsApiTests.helicsAssert("sub6 == null");
			}
			SWIGTYPE_p_void pub7 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub7", 6, null);
			if(pub7 == null) {
				javaHelicsApiTests.helicsAssert("pub7 == null");
			}
			SWIGTYPE_p_void sub7 = helics.helicsFederateRegisterTypeSubscription(fed1, "pub7", 6, null);
			if(sub7 == null) {
				javaHelicsApiTests.helicsAssert("sub7 == null");
			}
			rv = helics.helicsSubscriptionSetDefaultBoolean(sub5, helics.getHelics_false());
			rv = helics.helicsSubscriptionSetDefaultComplex(sub2, -9.9, 2.5);
			rv = helics.helicsSubscriptionSetDefaultDouble(sub1, 3.4);
			rv = helics.helicsSubscriptionSetDefaultInteger(sub4, 6);
			rv = helics.helicsSubscriptionSetDefaultNamedPoint(sub7, "hollow", 20.0);
			rv = helics.helicsSubscriptionSetDefaultString(sub3, "default");
			double[] sub6Default = {3.4,  90.9, 4.5};
			rv = helics.helicsSubscriptionSetDefaultVector(sub6,  sub6Default, 3);
			rv = helics.helicsEndpointSubscribe(ep2, "fed1/pub3", "string");
			rv = helics.helicsFederateEnterInitializationModeAsync(fed1);
			int rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
			if(rs != 0) {
				Thread.sleep(500);
				rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
				if(rs != 0) {
					Thread.sleep(500);
					rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
					if(rs != 0) {
						/* this operation should have completed by  now*/
						javaHelicsApiTests.helicsAssert("rs != 0");
					}
				}
			}
			rv = helics.helicsFederateEnterInitializationModeComplete(fed1);
			rv = helics.helicsFederateEnterExecutionModeAsync(fed1);
			rv = helics.helicsFederateEnterExecutionModeComplete(fed1);
			message_t mesg1 = new message_t();
			mesg1.setData("Hello");
			mesg1.setDest("Ep2");
			mesg1.setLength(5);
			mesg1.setOriginal_dest("Ep2");
			mesg1.setOriginal_source("fed1/Ep1");
			mesg1.setSource("fed1/Ep1");
			mesg1.setTime(0.0);
			rv = helics.helicsEndpointSendMessage(ep1, mesg1);
			mesg1.setData("There");
			rv = helics.helicsEndpointSendMessage(ep1, mesg1);
			rv = helics.helicsEndpointSetDefaultDestination(ep2, "fed1/Ep1");
			byte[] ep1Name = new byte[256];
			byte[] ep1Type = new byte[256];
			rv = helics.helicsEndpointGetName(ep1, ep1Name);
			rv = helics.helicsEndpointGetType(ep1, ep1Type);
			String ep1NameString = new String(ep1Name);
			if(!ep1NameString.contains("fed1/Ep1")) {
				javaHelicsApiTests.helicsAssert("!ep1NameString.contains(\"fed1/Ep1\")");
			}
			String ep1TypeString = new String(ep1Type);
			if(!ep1TypeString.contains("string")) {
				javaHelicsApiTests.helicsAssert("!ep1TypeString.contains(\"string\")");
			}
			SWIGTYPE_p_void coreFed1 = helics.helicsFederateGetCoreObject(fed1);
			if(coreFed1 == null) {
				javaHelicsApiTests.helicsAssert("coreFed1 == null");
			}
			double[] fed1Time = {5.3};
			rv = helics.helicsFederateGetCurrentTime(fed1, fed1Time);
			if(fed1Time[0] != 0.0) {
				javaHelicsApiTests.helicsAssert("fed1Time[0] != 0.0");
			}
			int fed1EndpointCount = helics.helicsFederateGetEndpointCount(fed1);
			if(fed1EndpointCount != 2) {
				javaHelicsApiTests.helicsAssert("fed1EndpointCount != 2");
			}
			byte[] fed1Name = new byte[256];
			rv = helics.helicsFederateGetName(fed1, fed1Name);
			String fed1NameString = new String(fed1Name);
			if(!fed1NameString.contains("fed1")) {
				javaHelicsApiTests.helicsAssert("!fed1NameString.contains(\"fed1\")");
			}
			int[] fed1State = {0};
			rv = helics.helicsFederateGetState(fed1, fed1State);
			if(fed1State[0] != 2) {
				javaHelicsApiTests.helicsAssert("fed1State != 2");
			}
			int fed1PubCount = helics.helicsFederateGetPublicationCount(fed1);
			if(fed1PubCount != 7) {
				javaHelicsApiTests.helicsAssert("fed1PubCount != 7");
			}
			int fed1SubCount = helics.helicsFederateGetSubscriptionCount(fed1);
			if(fed1SubCount != 7) {
				javaHelicsApiTests.helicsAssert("fed1SubCount != 7");
			}
			rv = helics.helicsPublicationPublishBoolean(pub5, helics.getHelics_true());
			rv = helics.helicsPublicationPublishComplex(pub2, 5.6, -0.67);
			rv = helics.helicsPublicationPublishDouble(pub1, 457.234);
			rv = helics.helicsPublicationPublishInteger(pub4, 1);
			rv = helics.helicsPublicationPublishNamedPoint(pub7, "Blah Blah", 20.0);
			rv = helics.helicsPublicationPublishString(pub3, "Mayhem");
			double[] pub6Vector = {4.5, 56.5};
			rv = helics.helicsPublicationPublishVector(pub6, pub6Vector, 2);
			Thread.sleep(500);
			rv = helics.helicsFederateRequestTimeAsync(fed1, 1.0);
			double[] returnTime = {4.5};
			rv = helics.helicsFederateRequestTimeComplete(fed1, returnTime);
			if(returnTime[0] != 1.0) {
				javaHelicsApiTests.helicsAssert("returnTime[0] != 1.0");
			}
			int ep2MsgCount = helics.helicsEndpointReceiveCount(ep2);
			if(ep2MsgCount != 3) {
				javaHelicsApiTests.helicsAssert("ep2MsgCount != 3");
			}
			int ep2HasMsg = helics.helicsEndpointHasMessage(ep2);
			if(ep2HasMsg != 1) {
				javaHelicsApiTests.helicsAssert("ep2HasMsg != 1");
			}
			message_t msg2 = helics.helicsEndpointGetMessage(ep2);
			double msg2Time = msg2.getTime();
			if(msg2Time != 1.0) {
				javaHelicsApiTests.helicsAssert("msg2Time != 1.0");
			}
			String msg2Data = msg2.getData();
			if(!"Hello".equals(msg2Data)) {
				javaHelicsApiTests.helicsAssert("!msg2Data.equals(\"Hello\")");
			}
			long msg2Length = msg2.getLength();
			if(msg2Length != 5) {
				javaHelicsApiTests.helicsAssert("msg2Length != 5");
			}
			String msg2OriginalSource = msg2.getOriginal_source();
			if(!"fed1/Ep1".equals(msg2OriginalSource)) {
				javaHelicsApiTests.helicsAssert("!msg2OriginalSource.equals(\"fed1/Ep1\")");
			}
			String msg2Source = msg2.getSource();
			if(!"fed1/Ep1".equals(msg2Source)) {
				javaHelicsApiTests.helicsAssert("!msg2Source.equals(\"fed1/Ep1\")");
			}
			String msg2Destination = msg2.getDest();
			if(!"Ep2".equals(msg2Destination)) {
				javaHelicsApiTests.helicsAssert("!msg2Destination.equals(\"Ep2\")");
			}
			String msg2OriginalDestination = msg2.getOriginal_dest();
			if(!"".equals(msg2OriginalDestination)) {
				javaHelicsApiTests.helicsAssert("!msg2OriginalDestination.equals(\"\")");
			}
			int fed1MsgCount = helics.helicsFederateReceiveCount(fed1);
			if(fed1MsgCount != 2) {
				javaHelicsApiTests.helicsAssert("fed1MsgCount != 2");
			}
			int fed1HasMsg = helics.helicsFederateHasMessage(fed1);
			if(fed1HasMsg != 1) {
				javaHelicsApiTests.helicsAssert("fed1HasMsg != 1");
			}
			message_t msg3 = helics.helicsFederateGetMessage(fed1);
			double msg3Time = msg3.getTime();
			if(msg3Time != 1.0) {
				javaHelicsApiTests.helicsAssert("msg3Time != 1.0");
			}
			String msg3Data = msg3.getData();
			if(!"There".equals(msg3Data)) {
				javaHelicsApiTests.helicsAssert("!msg3Data.equals(\"There\")");
			}
			long msg3Length = msg3.getLength();
			if(msg3Length != 5) {
				javaHelicsApiTests.helicsAssert("msg3Length != 5");
			}
			String msg3OriginalSource = msg3.getOriginal_source();
			if(!"fed1/Ep1".equals(msg3OriginalSource)) {
				javaHelicsApiTests.helicsAssert("!msg3OriginalSource.equals(\"fed1/Ep1\")");
			}
			String msg3Source = msg3.getSource();
			if(!"fed1/Ep1".equals(msg3Source)) {
				javaHelicsApiTests.helicsAssert("!msg3Source.equals(\"fed1/Ep1\")");
			}
			String msg3Destination = msg3.getDest();
			if(!"Ep2".equals(msg3Destination)) {
				javaHelicsApiTests.helicsAssert("!msg3Destination.equals(\"Ep2\")");
			}
			String msg3OriginalDestination = msg3.getOriginal_dest();
			if(!"".equals(msg3OriginalDestination)) {
				javaHelicsApiTests.helicsAssert("!msg3OriginalDestination.equals(\"\")");
			}
			byte[] sub1Type = new byte[256];
			rv = helics.helicsSubscriptionGetType(sub1, sub1Type);
			String sub1TypeString = new String(sub1Type);
			if(!sub1TypeString.contains("double")) {
				javaHelicsApiTests.helicsAssert("!sub1TypeString.equals(\"double\")");
			}
			int sub1Updated = helics.helicsSubscriptionIsUpdated(sub1);
			if(sub1Updated != 1) {
				javaHelicsApiTests.helicsAssert("sub1Updated != 1");
			}
			double sub2UpdateTime = helics.helicsSubscriptionLastUpdateTime(sub2);
			if(sub2UpdateTime != 1.0) {
				javaHelicsApiTests.helicsAssert("sub2UpdateTime != 1.0e9");
			}
			int[] sub5Value = {9};
			rv = helics.helicsSubscriptionGetBoolean(sub5, sub5Value);
			if(sub5Value[0] != 1) {
				javaHelicsApiTests.helicsAssert("sub5Value[0] != 1");
			}
			double[] sub2Real = {0.0};
			double[] sub2Imag = {0.0};
			rv = helics.helicsSubscriptionGetComplex(sub2, sub2Real, sub2Imag);
			if(sub2Real[0] != 5.6) {
				javaHelicsApiTests.helicsAssert("sub2Real[0] != 5.6");
			}
			if(sub2Imag[0] != -0.67) {
				javaHelicsApiTests.helicsAssert("sub2Imag[0] != -0.67");
			}
			double[] sub1Value = {0.0};
			rv = helics.helicsSubscriptionGetDouble(sub1, sub1Value);
			if(sub1Value[0] != 457.234) {
				javaHelicsApiTests.helicsAssert("sub1Value[0] != 457.234");
			}
			long[] sub4Value = {0};
			rv = helics.helicsSubscriptionGetInteger(sub4, sub4Value);
			if(sub4Value[0] != 1) {
				javaHelicsApiTests.helicsAssert("sub4Value[0] != 1");
			}
			byte[] sub7Point = new byte[256];
			int[] sub7PointLength = new int[1];
			double[] sub7DoubleValue = new double[1];
			rv = helics.helicsSubscriptionGetNamedPoint(sub7, sub7Point, sub7PointLength, sub7DoubleValue);
			String sub7PointString = new String(sub7Point);
			if(!sub7PointString.contains("Blah Blah")) {
				javaHelicsApiTests.helicsAssert("!sub7PointString.contains(\"Blah Blah\")");
			}
			if(sub7PointLength[0] != 9) {
				javaHelicsApiTests.helicsAssert("sub7PointLength[0] != 9");
			}
			if(sub7DoubleValue[0] != 20.0) {
				javaHelicsApiTests.helicsAssert("sub7DoubleValue[0] != 20.0");
			}
			byte[] sub3Value = new byte[256];
			int[] sub3Length = {256};
			rv = helics.helicsSubscriptionGetString(sub3, sub3Value, sub3Length);
			String sub3ValueString = new String(sub3Value);
			if(!sub3ValueString.contains("Mayhem")) {
				javaHelicsApiTests.helicsAssert("!sub3ValueString.contains(\"Mayhem\")");
			}
			/*string contains a null terminator*/
			if(sub3Length[0] != 7) {
				javaHelicsApiTests.helicsAssert("sub3Length[0] != 7");
			}
			int sub3ValueSize = helics.helicsSubscriptionGetValueSize(sub3);
			if(sub3ValueSize != 6) {
				javaHelicsApiTests.helicsAssert("sub3ValueSize != 6");
			}
			SWIGTYPE_p_double sub6Vector = null;
			int[] sub6ActualSize = new int[1];
			rv = helics.helicsSubscriptionGetVector(sub6, sub6Vector, 6, sub6ActualSize);
			rv = helics.helicsFederateFinalize(fed1);
			rv = helics.helicsFederateFinalize(fed2);
			helics.helicsFederateFree(fed1);
			helics.helicsFederateFinalize(fed1);
			helics.helicsFederateFree(fed2);
			helics.helicsFederateInfoFree(fedInfo2);
			rv = helics.helicsBrokerDisconnect(broker3);
			if(rv != helics_status.helics_ok) {
				javaHelicsApiTests.helicsAssert("rv != helics_status.helics_ok");
			}
			helics.helicsBrokerFree(broker3);
			//Clean Up Functions
			helics.helicsCleanupHelicsLibrary();
			helics.helicsCloseLibrary();
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			int totalNumberOfTests = javaHelicsApiTests.numberOfPassingTests + javaHelicsApiTests.numberOfFailedTests;
			double percentagePassed = (javaHelicsApiTests.numberOfPassingTests * 100) / (totalNumberOfTests);
			System.out.println(String.format("%d of %d tests passed.%n%.0f%% successfull.%n%d tests failed.", javaHelicsApiTests.numberOfPassingTests,totalNumberOfTests,percentagePassed,javaHelicsApiTests.numberOfFailedTests));
            if(javaHelicsApiTests.numberOfFailedTests > 0) {
                System.exit(1);
            }
		}
	}
}
