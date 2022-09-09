/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/helics98.hpp"

#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    // Load Gas Model - DemoAlt_disruption - Compressor Outage            


    // Get HELICS version
    Console.WriteLine($"Gas: HELICS version ={h.helicsGetVersion()}");

    // Create Federate Info object that describes the federate properties
    Console.WriteLine("Gas: Creating Federate Info");
    var fedinfo = h.helicsCreateFederateInfo();

    // Set core type from string
    Console.WriteLine("Gas: Setting Federate Core Type");
    h.helicsFederateInfoSetCoreName(fedinfo, "Gas Federate Core");
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "tcp");

    // Federate init string
    Console.WriteLine("Gas: Setting Federate Info Init String");
    string fedinitstring = "--federates=1";
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

    // Create value federate
    Console.WriteLine("Gas: Creating Value Federate");
    var vfed = h.helicsCreateValueFederate("Gas Federate", fedinfo);
    Console.WriteLine("Gas: Value federate created");

    // Register Publication and Subscription for coupling points
    SWIGTYPE_p_void GasPubPth = h.helicsFederateRegisterGlobalTypePublication(vfed, "GasThermalPower", "double", "");
    SWIGTYPE_p_void GasPubPthMax = h.helicsFederateRegisterGlobalTypePublication(vfed, "GasThermalPowerMin", "double", "");
    SWIGTYPE_p_void SubToElectric = h.helicsFederateRegisterSubscription(vfed, "ElectricPower", "");

    // Set one second message interval
    double period = 1;
    Console.WriteLine("Electric: Setting Federate Timing");
    h.helicsFederateSetTimeProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_TIME_PERIOD, period);

    // check to make sure setting the time property worked
    double period_set = h.helicsFederateGetTimeProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_TIME_PERIOD);
    Console.WriteLine($"Time period: {period_set}");

    // set max iteration at 20
    h.helicsFederateSetIntegerProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_INT_MAX_ITERATIONS, 20);
    int iter_max = h.helicsFederateGetIntegerProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_INT_MAX_ITERATIONS);
    Console.WriteLine($"Max iterations: {iter_max}");

    // enter execution mode
    h.helicsFederateEnterExecutingMode(vfed);
    Console.WriteLine("Gas: Entering execution mode");

    // Synthetic data
    double[] Pthermal = { 200, 200, 200, 200, 30, 200, 200, 200, 200, 200, 200, 200 };
    double[] PthermalOld = new double[Pthermal.Length];
    double[] PthermalNew = new double[Pthermal.Length];
    Array.Copy(Pthermal, PthermalOld,Pthermal.Length);
    Array.Copy(Pthermal, PthermalNew, Pthermal.Length);

    double PthermalMax = 1000;
    double PthermalMin = 100;
    // set number of HELICS time steps based on scenario
    double total_time = Pthermal.Length;
    Console.WriteLine($"Number of time steps in scenario: {total_time}");

    double granted_time = 0;
    double requested_time;
    //var iter_flag = HelicsIterationRequest.HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED;
    var iter_flag = HelicsIterationRequest.HELICS_ITERATION_REQUEST_FORCE_ITERATION;

    short Iter = 0;
    List<TimeStepInfo> timestepinfo = new List<TimeStepInfo>();
    List<TimeStepInfo> notconverged = new List<TimeStepInfo>();
    TimeStepInfo currenttimestep = new TimeStepInfo() { timestep = 0, itersteps = 0 };
    TimeStepInfo CurrentDiverged = new TimeStepInfo() { timestep = 0, itersteps = 0 };

    List<double> GasLastVal = new List<double>();

    int TimeStep;
    bool IsRepeating;
    bool HasViolations;
    int helics_iter_status;

    // Switch to release mode to enable console output to file
#if !DEBUG
    // redirect console output to log file
    FileStream ostrm;
    StreamWriter writer;
    TextWriter oldOut = Console.Out;
    ostrm = new FileStream(outputfolder + "Log_gas_federate.txt", FileMode.OpenOrCreate, FileAccess.Write);
    writer = new StreamWriter(ostrm);
    Console.SetOut(writer);
#endif
    // this function is called each time the SAInt solver state changes
    for (TimeStep = 0; TimeStep < total_time; TimeStep++)
    {
        // non-iterative time request here to block until both federates are done iterating
        Console.WriteLine($"Requested time {TimeStep}");
        //Console.WriteLine($"Requested time {e.TimeStep}");

        Iter = 0; // Iteration number

        // HELICS time granted 
        granted_time = h.helicsFederateRequestTime(vfed, TimeStep);
        Console.WriteLine($"Granted time: {granted_time}");

        IsRepeating = true;
        HasViolations = true;

        // Publishing initial available thermal power of zero MW and zero pressure difference
        if (TimeStep == 0)
        {
            //granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            MappingFactory.PublishGasThermalPower(granted_time - 1, Iter, PthermalMax, GasPubPth, 1, GasPubPthMax);
        }
        // Set time step info
        currenttimestep = new TimeStepInfo() { timestep = TimeStep, itersteps = 0 };
        timestepinfo.Add(currenttimestep);

        while (IsRepeating && Iter < iter_max)
        {
            // stop iterating if max iterations have been reached
            IsRepeating = (Iter < iter_max);
            Iter += 1;
            currenttimestep.itersteps += 1;

            // iterative HELICS time request                        
            Console.WriteLine($"Requested time: {TimeStep}, iteration: {Iter}");

            // get requested thermal power from connected gas plants, determine if there are violations                        
            HasViolations = false;

            // iterative HELICS time request
            // get publication from electric federate and avoid negative Pthermal due to the negative cubic function (to be checked in the electric federate)
            granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status); 
            double val = h.helicsInputGetDouble(SubToElectric);
            Console.WriteLine(String.Format("Gas-Received: Time {0} \t iter {1} \t Pthe = {2:0.0000} [MW]", TimeStep, Iter, val));

            GasLastVal.Add(val);

            if (Math.Abs(Pthermal[TimeStep] - val) > 0.001)
            {
                HasViolations = true;

                if (val < PthermalMin)
                {
                    Pthermal[TimeStep] = PthermalMin;
                }
                else if (val > PthermalMax)
                {
                    Pthermal[TimeStep] = PthermalMax;
                }

                else
                { 
                    Pthermal[TimeStep] = val; 
                }

                Console.WriteLine(String.Format("Gas-Event: Time {0} \t iter {1} \t Pthe = {2:0.0000} [MW]", TimeStep, Iter, Pthermal[TimeStep]));
            }
            else
            {
                if (GasLastVal.Count > 1)
                {
                    if (Math.Abs(GasLastVal[GasLastVal.Count - 2] - GasLastVal[GasLastVal.Count - 1]) > 0.001)
                    {
                        HasViolations = true;
                    }
                }
                else
                {
                    HasViolations = true;
                }
            }



            // HELICS time granted 
            granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            Console.WriteLine($"Granted time: {granted_time - 1}, iteration status: {helics_iter_status}");
            // using an offset of 1 on the granted_time here because HELICS starts at t=1 and SAInt starts at t=0 
            MappingFactory.PublishGasThermalPower(granted_time - 1, Iter, Pthermal[TimeStep], GasPubPth, (PthermalMax - val), GasPubPthMax);

            PthermalNew[TimeStep] = Pthermal[TimeStep];

            if (Iter >= iter_max && HasViolations)
            {
                CurrentDiverged = new TimeStepInfo() { timestep = TimeStep, itersteps = Iter };
                notconverged.Add(CurrentDiverged);
            }

            IsRepeating = HasViolations;

        }
    }

    // request time for end of time + 1: serves as a blocking call until all federates are complete
    requested_time = total_time;
    //Console.WriteLine($"Requested time: {requested_time}");

    Console.WriteLine($"Requested time step: {requested_time}");
    h.helicsFederateRequestTime(vfed, requested_time);

#if !DEBUG
    // close out log file
    Console.SetOut(oldOut);
    writer.Close();
    ostrm.Close();
#endif

    // finalize federate
    h.helicsFederateFinalize(vfed);
    Console.WriteLine("Gas: Federate finalized");
    h.helicsFederateFree(vfed);
    // If all federates are disconnected from the broker, then close libraries
    h.helicsCloseLibrary();

    using (FileStream fs = new FileStream(outputfolder + "TimeStepInfo_gas_federate.txt", FileMode.OpenOrCreate, FileAccess.Write))
    {
        using (StreamWriter sw2 = new StreamWriter(fs))
        {
            sw2.WriteLine("TimeStep \t IterStep");
            foreach (TimeStepInfo x in timestepinfo)
            {
                sw2.WriteLine(String.Format("{0}\t{1}", x.timestep, x.itersteps));
            }
        }

    }
    using (FileStream fs = new FileStream(outputfolder + "NotConverged_gas_federate.txt", FileMode.OpenOrCreate, FileAccess.Write))
    {
        using (StreamWriter sw2 = new StreamWriter(fs))
        {
            sw2.WriteLine("TimeStep \t IterStep");
            foreach (TimeStepInfo x in notconverged)
            {
                sw2.WriteLine(String.Format("{0}\t{1}", x.timestep, x.itersteps));
            }
        }
    }

    using (FileStream fs = new FileStream(outputfolder + "PthermalAndPthermalNew.txt", FileMode.OpenOrCreate, FileAccess.Write))
    {
        using (StreamWriter sw2 = new StreamWriter(fs))
        {
            sw2.WriteLine("TimeStep \t PthermalOld[MW]\t PthermalNew[MW] \tPthermalMin[MW]\t PthermalMax[MW]");
            for (int t=0;t <total_time; t++)
            {
                sw2.WriteLine(String.Format("{0}\t\t{1:0.00}\t\t{2:0.00}\t\t{3:0.00}\t\t{4:0.00}", t, PthermalOld[t],PthermalNew[t],PthermalMin,PthermalMax));
            }
        }
    }

    // Diverging time steps
    if (notconverged.Count == 0)
        Console.WriteLine("\n Gas: There is no diverging time step.");
    else
    {
        Console.WriteLine("Gas: the solution diverged at the following time steps:");
        foreach (TimeStepInfo x in notconverged)
        {
            Console.WriteLine($"Time-step {x.timestep}");
        }
        Console.WriteLine($"\n Gas: The total number of diverging time steps = { notconverged.Count }");
    }

  
    return 0;
}
