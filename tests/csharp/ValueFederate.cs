using System;

namespace HelicsValueFederate
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine(gmlc.helics.helicsGetVersion());

            // Create broker
            var brkr = gmlc.helics.helicsCreateBroker("zmq", "", "-f 1 --name=mainbroker");
            if (gmlc.helics.helicsBrokerIsConnected(brkr) == 1)
            {
                Console.WriteLine("Broker connection success!");
            }

            // Set up federate info
            var fedinfo = gmlc.helics.helicsCreateFederateInfo();
            gmlc.helics.helicsFederateInfoSetCoreName(fedinfo, "TestA Core");
            gmlc.helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq");
            gmlc.helics.helicsFederateInfoSetCoreInitString(fedinfo, "--broker=mainbroker --federates=1");
            gmlc.helics.helicsFederateInfoSetTimeProperty(fedinfo, (int)gmlc.helics_properties.helics_property_time_delta, 0.01);
            gmlc.helics.helicsFederateInfoSetIntegerProperty(fedinfo, (int)gmlc.helics_properties.helics_property_int_log_level, 1);

            // Create a value federate
            var vFed = gmlc.helics.helicsCreateValueFederate("TestA Federate", fedinfo);

            var state = gmlc.helics.helicsFederateGetState(vFed);
            if (state == gmlc.helics_federate_state.helics_state_startup)
            {
                Console.WriteLine("PASS (state == startup)");
            }
            else
            {
                Console.WriteLine("FAIL (state == startup)");
            }

            // Create a publication and subscription for the value federate
            var pub = gmlc.helics.helicsFederateRegisterGlobalPublication(vFed, "pub1", gmlc.helics_data_type.helics_data_type_double, "");
            var sub = gmlc.helics.helicsFederateRegisterSubscription(vFed, "pub1", "");
            gmlc.helics.helicsInputSetDefaultDouble(sub, 1.0);

            gmlc.helics.helicsFederateEnterExecutingMode(vFed);

            state = gmlc.helics.helicsFederateGetState(vFed);
            if (state == gmlc.helics_federate_state.helics_state_execution)
            {
                Console.WriteLine("PASS (state == executing)");
            }
            else
            {
                Console.WriteLine("FAIL (state == executing)");
            }


            // Test a series of published values and time advancement
            gmlc.helics.helicsPublicationPublishDouble(pub, 2.0);

            var value = gmlc.helics.helicsInputGetDouble(sub);
            if (value == 1.0)
            {
                Console.WriteLine("PASS (value == 1.0)");
            }
            else
            {
                Console.WriteLine("FAIL (value == 1.0)");
            }

            var grantedtime = gmlc.helics.helicsFederateRequestTime(vFed, 1.0);
            Console.WriteLine("Granted time " + grantedtime);

            value = gmlc.helics.helicsInputGetDouble(sub);
            if (value == 2.0)
            {
                Console.WriteLine("PASS (value == 2.0)");
            }
            else
            {
                Console.WriteLine("FAIL (value == 2.0)");
            }

            gmlc.helics.helicsPublicationPublishDouble(pub, 3.0);

            grantedtime = gmlc.helics.helicsFederateRequestTime(vFed, 2.0);
            Console.WriteLine("Granted time " + grantedtime);

            value = gmlc.helics.helicsInputGetDouble(sub);
            if (value == 3.0)
            {
                Console.WriteLine("PASS (value == 3.0)");
            }
            else
            {
                Console.WriteLine("FAIL (value == 3.0)");
            }

            // Finalize the federate
            gmlc.helics.helicsFederateFinalize(vFed);

            state = gmlc.helics.helicsFederateGetState(vFed);
            if (state == gmlc.helics_federate_state.helics_state_finalize)
            {
                Console.WriteLine("PASS (state == finalized)");
            }
            else
            {
                Console.WriteLine("FAIL (state == finalized)");
            }

            // Wait for the broker to disconnect
            while (gmlc.helics.helicsBrokerIsConnected(brkr) == 1)
            {
                System.Threading.Thread.Sleep(1);
            }

            // Clean-up
            gmlc.helics.helicsFederateInfoFree(fedinfo);
            gmlc.helics.helicsFederateFree(vFed);
            gmlc.helics.helicsCloseLibrary();
        }
    }
}
