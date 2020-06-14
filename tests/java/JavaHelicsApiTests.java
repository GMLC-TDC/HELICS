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
            // General HELICS Functions
            double helicsTimeZero = helics.getHelics_time_zero();
            if (helicsTimeZero != 0.0) {
                javaHelicsApiTests.helicsAssert("helicsTimeZero != 0.0.");
            }
            double helicsTimeEpsilon = helics.getHelics_time_epsilon();
            if (helicsTimeEpsilon != 1.0e-9) {
                javaHelicsApiTests.helicsAssert("helicsTimeEpsilon != 1.0E-9");
            }
            int helicsTrue = helics.getHelics_true();
            if (helicsTrue != 1) {
                javaHelicsApiTests.helicsAssert("helicsTrue != 1");
            }
            int helicsFalse = helics.getHelics_false();
            if (helicsFalse != 0) {
                javaHelicsApiTests.helicsAssert("helicsFalse != 0");
            }
            String helicsVersion = helics.helicsGetVersion();
            if (helicsVersion == null) {
                javaHelicsApiTests.helicsAssert("helicsVersion == null");
            }
            int helicsCoreTypeIsAvailabe = helics.helicsIsCoreTypeAvailable("zmq");
            if (helicsCoreTypeIsAvailabe != 1) {
                javaHelicsApiTests.helicsAssert("helicsCoreTypeIsAvailabe != 1");
            }
            // Broker API Functions
            SWIGTYPE_p_void broker1 = helics.helicsCreateBroker("zmq", "broker1", "--federates 3 --loglevel 1");
            if (broker1 == null) {
                javaHelicsApiTests.helicsAssert("broker1 == null");
            }
            SWIGTYPE_p_void broker2 = helics.helicsBrokerClone(broker1);
            if (broker2 == null) {
                javaHelicsApiTests.helicsAssert("broker2 == null");
            }
            byte[] broker1Address = new byte[256];
            String broker1AddressString = helics.helicsBrokerGetAddress(broker1);
            if (!broker1AddressString.contains("tcp://127.0.0.1:23404")) {
                javaHelicsApiTests.helicsAssert("!broker1AddressString.equals(\"tcp://127.0.0.1:23404\")");
            }

            String broker1IdentifierString = helics.helicsBrokerGetIdentifier(broker1);

            if (!broker1IdentifierString.contains("broker1")) {
                javaHelicsApiTests.helicsAssert("!broker1IdentifierString.equals(\"broker1\")");
            }
            int broker1IsConnected = helics.helicsBrokerIsConnected(broker1);
            if (broker1IsConnected != 1) {
                javaHelicsApiTests.helicsAssert("broker1IsConnected != 1");
            }
            helics.helicsBrokerDisconnect(broker1);
            broker1IsConnected = helics.helicsBrokerIsConnected(broker1);
            if (broker1IsConnected != 0) {
                javaHelicsApiTests.helicsAssert("broker1IsConnected != 0");
            }
            helics.helicsBrokerDisconnect(broker2);
            helics.helicsBrokerFree(broker1);
            helics.helicsBrokerFree(broker2);
            helics.helicsCloseLibrary();
            // Core API Functions
            SWIGTYPE_p_void core1 = helics.helicsCreateCore("test", "core1", "--autobroker");
            if (core1 == null) {
                javaHelicsApiTests.helicsAssert("core1 == null");
            }
            SWIGTYPE_p_void core2 = helics.helicsCoreClone(core1);
            if (core2 == null) {
                javaHelicsApiTests.helicsAssert("core2 == null");
            }
            byte[] core1Identifier = new byte[10];
            String core1IdentifierString = helics.helicsCoreGetIdentifier(core1);

            if (!core1IdentifierString.contains("core1")) {
                javaHelicsApiTests.helicsAssert("!core1IdentifierString.equals(\"core1\")");
            }

            int core1IsConnected = helics.helicsCoreIsConnected(core1);
            if (core1IsConnected != 0) {
                javaHelicsApiTests.helicsAssert("core1IsConnected != 0");
            }
            SWIGTYPE_p_void sourceFilter1 = helics.helicsCoreRegisterFilter(core1,
                    helics_filter_type.helics_filter_type_delay, "core1SourceFilter");
            if (sourceFilter1 == null) {
                javaHelicsApiTests.helicsAssert("sourceFilter1 == null");
            }
            helics.helicsFilterAddSourceTarget(sourceFilter1, "ep1");
            SWIGTYPE_p_void destinationFilter1 = helics.helicsCoreRegisterFilter(core1,
                    helics_filter_type.helics_filter_type_delay, "core1DestinationFilter");
            if (destinationFilter1 == null) {
                javaHelicsApiTests.helicsAssert("destinationFilter1 == null");
            }
            helics.helicsFilterAddDestinationTarget(destinationFilter1, "ep2");
            SWIGTYPE_p_void cloningFilter1 = helics.helicsCoreRegisterCloningFilter(core1, "ep3");
            if (cloningFilter1 == null) {
                javaHelicsApiTests.helicsAssert("cloningFilter1 == null");
            }
            helics.helicsFilterRemoveDeliveryEndpoint(cloningFilter1, "ep3");

            helics.helicsCoreSetReadyToInit(core1);
            helics.helicsCoreDisconnect(core1);
            helics.helicsCoreDisconnect(core2);
            helics.helicsCoreFree(core1);
            helics.helicsCoreFree(core2);
            helics.helicsCloseLibrary();
            // Federate Info API Functions
            SWIGTYPE_p_void fedInfo1 = helics.helicsCreateFederateInfo();
            if (fedInfo1 == null) {
                javaHelicsApiTests.helicsAssert("fedInfo1 == null");
            }
            helics.helicsFederateInfoSetCoreInitString(fedInfo1, "-f 1");
            helics.helicsFederateInfoSetCoreName(fedInfo1, "core3");
            helics.helicsFederateInfoSetCoreType(fedInfo1, 3);
            helics.helicsFederateInfoSetCoreTypeFromString(fedInfo1, "zmq");
            helics.helicsFederateInfoSetFlagOption(fedInfo1, 1, helics.getHelics_true());
            helics.helicsFederateInfoSetTimeProperty(fedInfo1,
                    helics_properties.helics_property_time_input_delay.swigValue(), 1.0);
            helics.helicsFederateInfoSetIntegerProperty(fedInfo1,
                    helics_properties.helics_property_int_log_level.swigValue(), 1);
            helics.helicsFederateInfoSetIntegerProperty(fedInfo1,
                    helics_properties.helics_property_int_max_iterations.swigValue(), 100);
            helics.helicsFederateInfoSetTimeProperty(fedInfo1,
                    helics_properties.helics_property_time_output_delay.swigValue(), 1.0);
            helics.helicsFederateInfoSetTimeProperty(fedInfo1,
                    helics_properties.helics_property_time_period.swigValue(), 1.0);
            helics.helicsFederateInfoSetTimeProperty(fedInfo1,
                    helics_properties.helics_property_time_delta.swigValue(), 1.0);
            helics.helicsFederateInfoSetTimeProperty(fedInfo1,
                    helics_properties.helics_property_time_offset.swigValue(), 0.1);
            helics.helicsFederateInfoFree(fedInfo1);
            // Federate API Functions
            SWIGTYPE_p_void broker3 = helics.helicsCreateBroker("zmq", "broker3", "--federates 1 --loglevel 1");
            SWIGTYPE_p_void fedInfo2 = helics.helicsCreateFederateInfo();
            String coreInitString = "--federates 1";
            helics.helicsFederateInfoSetCoreInitString(fedInfo2, coreInitString);
            helics.helicsFederateInfoSetCoreTypeFromString(fedInfo2, "zmq");
            helics.helicsFederateInfoSetIntegerProperty(fedInfo2,
                    helics_properties.helics_property_int_log_level.swigValue(), 1);
            helics.helicsFederateInfoSetTimeProperty(fedInfo2,
                    helics_properties.helics_property_time_delta.swigValue(), 1.0);
            SWIGTYPE_p_void fed1 = helics.helicsCreateCombinationFederate("fed1", fedInfo2);
            if (fed1 == null) {
                javaHelicsApiTests.helicsAssert("fed1 == null");
            }
            SWIGTYPE_p_void fed2 = helics.helicsFederateClone(fed1);
            if (fed2 == null) {
                javaHelicsApiTests.helicsAssert("fed2 == null");
            }
            SWIGTYPE_p_void fed3 = helics.helicsGetFederateByName("fed1");
            if (fed3 == null) {
                javaHelicsApiTests.helicsAssert("fed3 == null");
            }
            //set uninterruptible flag
            helics.helicsFederateSetFlagOption(fed2, 1, helics.getHelics_false());

         //   helics.helicsFederateSetTimeProperty(fed2,
          //          helics_properties.helics_property_time_input_delay.swigValue(), 1.0);
            helics.helicsFederateSetIntegerProperty(fed1,
                    helics_properties.helics_property_int_log_level.swigValue(), 1);
            helics.helicsFederateSetIntegerProperty(fed2,
                    helics_properties.helics_property_int_max_iterations.swigValue(), 100);
           // helics.helicsFederateSetTimeProperty(fed2,
            //        helics_properties.helics_property_time_output_delay.swigValue(), 1.0);
            helics.helicsFederateSetTimeProperty(fed2, helics_properties.helics_property_time_period.swigValue(),
                    0.0);
            helics.helicsFederateSetTimeProperty(fed2,
                    helics_properties.helics_property_time_delta.swigValue(), 1.0);

            SWIGTYPE_p_void fed1CloningFilter = helics.helicsFederateRegisterCloningFilter(fed1, "fed1/Ep1");
            if (fed1CloningFilter == null) {
                javaHelicsApiTests.helicsAssert("fed1CloningFilter == null");
            }
            SWIGTYPE_p_void fed1DestinationFilter = helics.helicsFederateRegisterFilter(fed1,
                    helics_filter_type.helics_filter_type_delay, "fed1DestinationFilter");
            if (fed1DestinationFilter == null) {
                javaHelicsApiTests.helicsAssert("fed1DestinationFilter == null");
            }
            helics.helicsFilterAddDestinationTarget(fed1DestinationFilter, "ep2");

            SWIGTYPE_p_void ep1 = helics.helicsFederateRegisterEndpoint(fed1, "Ep1", "string");
            if (ep1 == null) {
                javaHelicsApiTests.helicsAssert("ep1 == null");
            }
            SWIGTYPE_p_void ep2 = helics.helicsFederateRegisterGlobalEndpoint(fed1, "Ep2", "string");
            if (ep2 == null) {
                javaHelicsApiTests.helicsAssert("ep2 == null");
            }
            SWIGTYPE_p_void pub1 = helics.helicsFederateRegisterGlobalPublication(fed1, "pub1",
                    helics_data_type.helics_data_type_double, null);
            if (pub1 == null) {
                javaHelicsApiTests.helicsAssert("pub1 == null");
            }
            SWIGTYPE_p_void pub2 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub2", "complex", null);
            if (pub2 == null) {
                javaHelicsApiTests.helicsAssert("pub2 == null");
            }

            SWIGTYPE_p_void sub1 = helics.helicsFederateRegisterSubscription(fed1, "pub1", null);
            if (sub1 == null) {
                javaHelicsApiTests.helicsAssert("sub1 == null");
            }
            SWIGTYPE_p_void sub2 = helics.helicsFederateRegisterSubscription(fed1, "pub2", null);
            if (sub2 == null) {
                javaHelicsApiTests.helicsAssert("sub2 == null");
            }
            helics.helicsInputAddTarget(sub2, "Ep2");
            SWIGTYPE_p_void pub3 = helics.helicsFederateRegisterPublication(fed1, "pub3",
                    helics_data_type.helics_data_type_string, null);
            if (pub3 == null) {
                javaHelicsApiTests.helicsAssert("pub3 == null");
            }

            String pub1KeyString = helics.helicsPublicationGetKey(pub1);
            String pub1TypeString = helics.helicsPublicationGetType(pub1);
            String pub1UnitsString = helics.helicsPublicationGetUnits(pub1);
            String sub1KeyString = helics.helicsSubscriptionGetKey(sub1);
            String sub1UnitsString = helics.helicsInputGetUnits(sub1);
            if (!pub1KeyString.contains("pub1")) {
                javaHelicsApiTests.helicsAssert("!pub1KeyString.equals(\"pub1\")");
            }
            if (!pub1TypeString.contains("double")) {
                javaHelicsApiTests.helicsAssert("!pub1KeyString.equals(\"double\")");
            }
            if (!pub1UnitsString.contains("")) {
                javaHelicsApiTests.helicsAssert("!pub1UnitsString.equals(\"\")");
            }
            if (!sub1KeyString.contains("pub1")) {
                javaHelicsApiTests.helicsAssert("!sub1KeyString.equals(\"pub1\")");
            }
            if (!sub1UnitsString.contains("")) {
                javaHelicsApiTests.helicsAssert("!sub1UnitsString.equals(\"\")");
            }
            SWIGTYPE_p_void fed1SourceFilter = helics.helicsFederateRegisterFilter(fed1,
                    helics_filter_type.helics_filter_type_delay, "fed1SourceFilter");
            if (fed1SourceFilter == null) {
                javaHelicsApiTests.helicsAssert("fed1SourceFilter == null");
            }
            helics.helicsFilterAddSourceTarget(fed1SourceFilter, "Ep2");
            helics.helicsFilterAddDestinationTarget(fed1SourceFilter, "fed1/Ep1");
            helics.helicsFilterRemoveTarget(fed1SourceFilter, "fed1/Ep1");
            helics.helicsFilterAddSourceTarget(fed1SourceFilter, "Ep2");
            helics.helicsFilterRemoveTarget(fed1SourceFilter, "Ep2");
            String fed1SourceFilterNameString = helics.helicsFilterGetName(fed1SourceFilter);
            if (!fed1SourceFilterNameString.contains("fed1SourceFilter")) {
                javaHelicsApiTests.helicsAssert("!fed1SourceFilterNameString.contains(\"fed1SourceFilter\")");
            }
            // Need to test them with helicsFederateRegisterTypeInput() and helics.helicsInputAddTarget()
            // SWIGTYPE_p_void sub3 = helics.helicsFederateRegisterTypeInput(fed1,
            // "fed1/pub3", "string", null);
            SWIGTYPE_p_void sub3 = helics.helicsFederateRegisterSubscription(fed1, "fed1/pub3", null);
            if (sub3 == null) {
                javaHelicsApiTests.helicsAssert("sub3 == null");
            }
            SWIGTYPE_p_void pub4 = helics.helicsFederateRegisterTypePublication(fed1, "pub4", "int", null);
            if (pub4 == null) {
                javaHelicsApiTests.helicsAssert("pub4 == null");
            }
            // SWIGTYPE_p_void sub4 = helics.helicsFederateRegisterTypeInput(fed1,
            // "fed1/pub4", "int", null);
            SWIGTYPE_p_void sub4 = helics.helicsFederateRegisterSubscription(fed1, "fed1/pub4", null);
            if (sub4 == null) {
                javaHelicsApiTests.helicsAssert("sub4 == null");
            }
            SWIGTYPE_p_void pub5 = helics.helicsFederateRegisterGlobalTypePublication(fed1, "pub5", "boolean", null);
            if (pub5 == null) {
                javaHelicsApiTests.helicsAssert("pub5 == null");
            }
            // SWIGTYPE_p_void sub5 = helics.helicsFederateRegisterTypeInput(fed1, "pub5",
            // "boolean", null);
            SWIGTYPE_p_void sub5 = helics.helicsFederateRegisterSubscription(fed1, "pub5", null);
            if (sub5 == null) {
                javaHelicsApiTests.helicsAssert("sub5 == null");
            }
            SWIGTYPE_p_void pub6 = helics.helicsFederateRegisterGlobalPublication(fed1, "pub6",
                    helics_data_type.helics_data_type_vector, null);
            if (pub6 == null) {
                javaHelicsApiTests.helicsAssert("pub6 == null");
            }
            // SWIGTYPE_p_void sub6 = helics.helicsFederateRegisterInput(fed1, "pub6",
            // helicsConstants.HELICS_DATA_TYPE_VECTOR, null);
            SWIGTYPE_p_void sub6 = helics.helicsFederateRegisterSubscription(fed1, "pub6", null);
            if (sub6 == null) {
                javaHelicsApiTests.helicsAssert("sub6 == null");
            }
            SWIGTYPE_p_void pub7 = helics.helicsFederateRegisterGlobalPublication(fed1, "pub7",
                    helics_data_type.helics_data_type_named_point, null);
            if (pub7 == null) {
                javaHelicsApiTests.helicsAssert("pub7 == null");
            }
            // SWIGTYPE_p_void sub7 = helics.helicsFederateRegisterInput(fed1, "pub7",
            // helicsConstants.HELICS_DATA_TYPE_NAMEDPOINT, null);
            SWIGTYPE_p_void sub7 = helics.helicsFederateRegisterSubscription(fed1, "pub7", null);
            if (sub7 == null) {
                javaHelicsApiTests.helicsAssert("sub7 == null");
            }
            helics.helicsInputSetDefaultBoolean(sub5, helics.getHelics_false());
            helics.helicsInputSetDefaultComplex(sub2, -9.9, 2.5);
            helics.helicsInputSetDefaultDouble(sub1, 3.4);
            helics.helicsInputSetDefaultInteger(sub4, 6);
            helics.helicsInputSetDefaultNamedPoint(sub7, "hollow", 20.0);
            helics.helicsInputSetDefaultString(sub3, "default");
            double[] sub6Default = { 3.4, 90.9, 4.5 };
            helics.helicsInputSetDefaultVector(sub6, sub6Default, 3);
            helics.helicsEndpointSubscribe(ep2, "fed1/pub3");
            helics.helicsFederateEnterInitializingModeAsync(fed1);
            int rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
            if (rs == 0) {
                Thread.sleep(500);
                rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
                if (rs == 0) {
                    Thread.sleep(500);
                    rs = helics.helicsFederateIsAsyncOperationCompleted(fed1);
                    if (rs == 0) {
                        /* this operation should have completed by now */
                        javaHelicsApiTests.helicsAssert("rs == 0 rs==" + Integer.toString(rs));
                    }
                }
            }
            helics.helicsFederateEnterInitializingModeComplete(fed1);
            helics.helicsFederateEnterExecutingModeAsync(fed1);
            helics.helicsFederateEnterExecutingModeComplete(fed1);
            helics_message mesg1 = new helics_message();
            mesg1.setData("Hello");
            mesg1.setDest("Ep2");
            mesg1.setLength(5);
            mesg1.setOriginal_dest("Ep2");
            mesg1.setOriginal_source("fed1/Ep1");
            mesg1.setSource("fed1/Ep1");
            mesg1.setTime(0.0);
            helics.helicsEndpointSendMessage(ep1, mesg1);
            mesg1.setData("There");
            helics.helicsEndpointSendMessage(ep1, mesg1);
            helics.helicsEndpointSetDefaultDestination(ep2, "fed1/Ep1");

            String ep1NameString = helics.helicsEndpointGetName(ep1);
            String ep1TypeString = helics.helicsEndpointGetType(ep1);

            if(!ep1NameString.contains("fed1/Ep1")) {
                javaHelicsApiTests.helicsAssert("!ep1NameString.contains(\"fed1/Ep1\")");
            }

            if(!ep1TypeString.contains("string")) {
                javaHelicsApiTests.helicsAssert("!ep1TypeString.contains(\"string\")");
            }

            SWIGTYPE_p_void coreFed1 = helics.helicsFederateGetCoreObject(fed1);
            if (coreFed1 == null) {
                javaHelicsApiTests.helicsAssert("coreFed1 == null");
            }

            double fed1Time = helics.helicsFederateGetCurrentTime(fed1);
            if (fed1Time != 0.0) {
                javaHelicsApiTests.helicsAssert("fed1Time[0] != 0.0");
            }
            int fed1EndpointCount = helics.helicsFederateGetEndpointCount(fed1);
            if (fed1EndpointCount != 2) {
                javaHelicsApiTests.helicsAssert("fed1EndpointCount != 2");
            }

            String fed1NameString = helics.helicsFederateGetName(fed1);
            if (!fed1NameString.contains("fed1")) {
                javaHelicsApiTests.helicsAssert("!fed1NameString.contains(\"fed1\")");
            }

            helics_federate_state fed1State = helics.helicsFederateGetState(fed1);
            if (fed1State.swigValue() != 2) {
                javaHelicsApiTests.helicsAssert("fed1State != 2");
            }
            int fed1PubCount = helics.helicsFederateGetPublicationCount(fed1);
            if (fed1PubCount != 7) {
                javaHelicsApiTests.helicsAssert("fed1PubCount != 7");
            }
            int fed1SubCount = helics.helicsFederateGetInputCount(fed1);

            if (fed1SubCount != 7) {
                javaHelicsApiTests.helicsAssert("fed1SubCount != 7");
            }
            helics.helicsPublicationPublishBoolean(pub5, helics.getHelics_true());
            helics.helicsPublicationPublishComplex(pub2, 5.6, -0.67);
            helics.helicsPublicationPublishDouble(pub1, 457.234);
            helics.helicsPublicationPublishInteger(pub4, 1);
            helics.helicsPublicationPublishNamedPoint(pub7, "Blah Blah", 20.0);
            helics.helicsPublicationPublishString(pub3, "Mayhem");
            double[] pub6Vector = { 4.5, 56.5 };
            helics.helicsPublicationPublishVector(pub6, pub6Vector, 2);
            Thread.sleep(500);
            helics.helicsFederateRequestTimeAsync(fed1, 1.0);

            double returnTime = helics.helicsFederateRequestTimeComplete(fed1);
            if (returnTime != 1.0) {
                javaHelicsApiTests.helicsAssert("returnTime[0] != 1.0");
            }
            int ep2MsgCount = helics.helicsEndpointPendingMessages(ep2);
            if (ep2MsgCount != 2) {
                javaHelicsApiTests.helicsAssert("ep2MsgCount != 2");
            }
            int ep2HasMsg = helics.helicsEndpointHasMessage(ep2);
            if (ep2HasMsg != 1) {
                javaHelicsApiTests.helicsAssert("ep2HasMsg != 1");
            }
            helics_message msg2 = helics.helicsEndpointGetMessage(ep2);
            double msg2Time = msg2.getTime();
            if (msg2Time != 0.0) {
               System.out.println(msg2Time);
                javaHelicsApiTests.helicsAssert("msg2Time != 1.0");
            }
            String msg2Data = msg2.getData();
            if (!"Hello".equals(msg2Data)) {
             System.out.println(msg2Data);
                javaHelicsApiTests.helicsAssert("!msg2Data.equals(\"Hello\")");
            }
            long msg2Length = msg2.getLength();
            if (msg2Length != 5) {
                javaHelicsApiTests.helicsAssert("msg2Length != 5");
            }
            String msg2OriginalSource = msg2.getOriginal_source();
            if (!"fed1/Ep1".equals(msg2OriginalSource)) {
                javaHelicsApiTests.helicsAssert("!msg2OriginalSource.equals(\"fed1/Ep1\")");
            }
            String msg2Source = msg2.getSource();
            if (!"fed1/Ep1".equals(msg2Source)) {
                javaHelicsApiTests.helicsAssert("!msg2Source.equals(\"fed1/Ep1\")");
            }
            String msg2Destination = msg2.getDest();
            if (!"Ep2".equals(msg2Destination)) {
                System.out.println(msg2Destination);
                javaHelicsApiTests.helicsAssert("!msg2Destination.equals(\"Ep2\")");
            }
            String msg2OriginalDestination = msg2.getOriginal_dest();
            if (!"".equals(msg2OriginalDestination)) {
                javaHelicsApiTests.helicsAssert("!msg2OriginalDestination.equals(\"\")");
            }
            int fed1MsgCount = helics.helicsFederatePendingMessages(fed1);
            if (fed1MsgCount != 1) {
                javaHelicsApiTests.helicsAssert("fed1MsgCount != 1");
            }
            int fed1HasMsg = helics.helicsFederateHasMessage(fed1);
            if (fed1HasMsg != 1) {
                javaHelicsApiTests.helicsAssert("fed1HasMsg != 1");
            }
            helics_message msg3 = helics.helicsFederateGetMessage(fed1);
            double msg3Time = msg3.getTime();
            if (msg3Time != 0.0) {
                javaHelicsApiTests.helicsAssert("msg3Time != 0.0");
            }
            String msg3Data = msg3.getData();
            if (!"There".equals(msg3Data)) {
                javaHelicsApiTests.helicsAssert("!msg3Data.equals(\"There\")");
            }
            long msg3Length = msg3.getLength();
            if (msg3Length != 5) {
                javaHelicsApiTests.helicsAssert("msg3Length != 5");
            }

            String msg3OriginalSource = msg3.getOriginal_source();
            if (!"fed1/Ep1".equals(msg3OriginalSource)) {
                javaHelicsApiTests.helicsAssert("!msg3OriginalSource.equals(\"fed1/Ep1\")");
            }
            String msg3Source = msg3.getSource();
            if (!"fed1/Ep1".equals(msg3Source)) {
                javaHelicsApiTests.helicsAssert("!msg3Source.equals(\"fed1/Ep1\")");
            }
            String msg3Destination = msg3.getDest();
            if (!"Ep2".equals(msg3Destination)) {
                javaHelicsApiTests.helicsAssert("!msg3Destination.equals(\"Ep2\")");
            }
            String msg3OriginalDestination = msg3.getOriginal_dest();
            if (!"".equals(msg3OriginalDestination)) {
                javaHelicsApiTests.helicsAssert("!msg3OriginalDestination.equals(\"\")");
            }

//            String sub1TypeString = "";
//            sub1TypeString = helics.helicsInputGetType(sub1);
            // Commented for now till C APIs for HELICS 2.0 are completely tested.
//            if (!sub1TypeString.contains("double")) {
//                javaHelicsApiTests.helicsAssert("!sub1TypeString.equals(\"double\")");
//            }
            int sub1Updated = helics.helicsInputIsUpdated(sub1);
            if (sub1Updated != 1) {
                javaHelicsApiTests.helicsAssert("sub1Updated != 1");
            }
            double sub2UpdateTime = helics.helicsInputLastUpdateTime(sub2);

            if (sub2UpdateTime != 1.0) {
                javaHelicsApiTests.helicsAssert("sub2UpdateTime != 1.0");
            }

            double[] sub2Real = { 0.0 };
            double[] sub2Imag = { 0.0 };
            helics.helicsInputGetComplex(sub2, sub2Real, sub2Imag);
            if (sub2Real[0] != 5.6) {
                javaHelicsApiTests.helicsAssert("sub2Real != 5.6");
            }
            if (sub2Imag[0] != -0.67) {
                javaHelicsApiTests.helicsAssert("sub2Imag != -0.67");
            }

            double sub1Value = helics.helicsInputGetDouble(sub1);
            if (sub1Value != 457.234) {
                javaHelicsApiTests.helicsAssert("sub1Value != 457.234");
            }
            long sub4Value = helics.helicsInputGetInteger(sub4);
            if (sub4Value != 1) {
                System.out.println(sub4Value);
                javaHelicsApiTests.helicsAssert("sub4Value != 1");
            }
            byte[] sub7Point = new byte[256];
            int[] sub7PointLength = new int[1];
            double[] sub7DoubleValue = new double[1];
            helics.helicsInputGetNamedPoint(sub7, sub7Point, sub7PointLength, sub7DoubleValue);
            String sub7PointString = new String(sub7Point);

            if (!sub7PointString.contains("Blah Blah")) {
                javaHelicsApiTests.helicsAssert("!sub7PointString.contains(\"Blah Blah\")");
            }
            if (sub7PointLength[0] != 10) {
                javaHelicsApiTests.helicsAssert("sub7PointLength[0] != 10");
            }
            if (sub7DoubleValue[0] != 20.0) {
                javaHelicsApiTests.helicsAssert("sub7DoubleValue[0] != 20.0");
            }
            int sub5Value = helics.helicsInputGetBoolean(sub5);
            if (sub5Value != 1) {
                javaHelicsApiTests.helicsAssert("sub5Value[0] != 1");
            }
            byte[] sub3Value = new byte[256];
            int[] sub3Length = { 256 };
            helics.helicsInputGetString(sub3, sub3Value, sub3Length);
            String sub3ValueString = new String(sub3Value);
            if (!sub3ValueString.contains("Mayhem")) {
                javaHelicsApiTests.helicsAssert("!sub3ValueString.contains(\"Mayhem\")");
            }

            /* string contains a null terminator */
            if (sub3Length[0] != 7) {
                javaHelicsApiTests.helicsAssert("sub3Length[0] != 7");
            }
            int sub3ValueSize = helics.helicsInputGetRawValueSize(sub3);
            if (sub3ValueSize != 6) {
                javaHelicsApiTests.helicsAssert("sub3ValueSize != 6");
            }
            SWIGTYPE_p_double sub6Vector = null;
            int[] sub6ActualSize = new int[1];
            helics.helicsInputGetVector(sub6, sub6Vector, 6, sub6ActualSize);

            helics.helicsFederateFinalize(fed1);
            helics.helicsFederateFinalize(fed2);
            helics.helicsFederateFree(fed1);
            helics.helicsFederateFinalize(fed2);
            helics.helicsFederateFree(fed2);
            helics.helicsFederateInfoFree(fedInfo2);
            helics.helicsBrokerDisconnect(broker3);

            helics.helicsBrokerFree(broker3);
            // Clean Up Functions
            helics.helicsCleanupLibrary();
            helics.helicsCloseLibrary();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            int totalNumberOfTests = javaHelicsApiTests.numberOfPassingTests + javaHelicsApiTests.numberOfFailedTests;
            double percentagePassed = (javaHelicsApiTests.numberOfPassingTests * 100) / (totalNumberOfTests);
            System.out.println(String.format("%d of %d tests passed.%n%.0f%% successful.%n%d tests failed.",
                    javaHelicsApiTests.numberOfPassingTests, totalNumberOfTests, percentagePassed,
                    javaHelicsApiTests.numberOfFailedTests));
            if (javaHelicsApiTests.numberOfFailedTests > 0) {
                System.exit(1);
            }
        }
    }
}
