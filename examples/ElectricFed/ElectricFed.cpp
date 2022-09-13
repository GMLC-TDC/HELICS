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
    // Artificial delay
    int delay = 0;
    int AfterTimeStep = 0;


    // Get HELICS version
    std::cout << "Electric: HELICS version ="<<helicscpp::version()<<std::endl;

    // Create Federate Info object that describes the federate properties
    std::cout << "Electric: Creating Federate Info"<<std::endl;
    helicscpp::FederateInfo fedinfo = helicscpp::FederateInfo{};

    // Set core type from string
    std::cout << "Electric: Setting Federate Core Type";
    h.helicsFederateInfoSetCoreName(fedinfo, "Electric Federate Core");
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "tcp");

    // Federate init string
    std::cout << "Electric: Setting Federate Info Init String"<<std::endl;
    string fedinitstring = "--federates=1";
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

    // Create value federate
    std::cout << "Electric: Creating Value Federate"<<std::endl;
    var vfed = h.helicsCreateValueFederate("Electric Federate", fedinfo);
    std::cout << "Electric: Value federate created"<<std::endl;

    // Register Publication and Subscription for coupling points
    SWIGTYPE_p_void ElectricPub = h.helicsFederateRegisterGlobalTypePublication(vfed, "ElectricPower", "double", "");
    SWIGTYPE_p_void SubToGas = h.helicsFederateRegisterSubscription(vfed, "GasThermalPower", "");
    SWIGTYPE_p_void SubToGasMin = h.helicsFederateRegisterSubscription(vfed, "GasThermalPowerMin", "");

    // Set one second message interval
    double period = 1;
    std::cout << "Electric: Setting Federate Timing"<<std::endl;
    h.helicsFederateSetTimeProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_TIME_PERIOD, period);

    // check to make sure setting the time property worked
    double period_set = h.helicsFederateGetTimeProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_TIME_PERIOD);
    std::cout << "Time period: "<<period_set<<std::endl;

    // set max iteration at 20
    h.helicsFederateSetIntegerProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_INT_MAX_ITERATIONS, 20);
    int iter_max = h.helicsFederateGetIntegerProperty(vfed, (int)HelicsProperties.HELICS_PROPERTY_INT_MAX_ITERATIONS);
    std::cout << "Max iterations: "<<iter_max<<std::endl;

    // start execution mode
    h.helicsFederateEnterExecutingMode(vfed);
    std::cout << "Electric: Entering execution mode");

    // Synthetic data
    double[] P = { 70, 50, 20, 80, 30, 100, 90, 65, 75, 70, 60, 50 };
    double[] POld = new double[P.Length];
    double[] PNew = new double[P.Length];
    Array.Copy(P, POld, P.Length);
    Array.Copy(P, PNew, P.Length);
    // set number of HELICS time steps based on scenario
    double total_time = P.Length;
    std::cout << "Number of time steps in scenario: "<<total_time <<std::endl;

    double granted_time = 0;
    double requested_time;
    //var iter_flag = HelicsIterationRequest.HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED;
    var iter_flag = HelicsIterationRequest.HELICS_ITERATION_REQUEST_FORCE_ITERATION;

    // variables to control iterations
    short Iter = 0;
    List<TimeStepInfo> timestepinfo = new List<TimeStepInfo>();
    List<TimeStepInfo> notconverged = new List<TimeStepInfo>();
    TimeStepInfo CurrentDiverged = new TimeStepInfo();
    TimeStepInfo currenttimestep = new TimeStepInfo() { timestep = 0, itersteps = 0 };

    List<double> ElecLastVal = new List<double>();

    int TimeStep;
    bool IsRepeating;
    bool HasViolations;
    int helics_iter_status;


    // Main function to be executed on the input data
    for (TimeStep = 0; TimeStep < total_time; TimeStep++)
    {
        // non-iterative time request here to block until both federates are done iterating the last time step
        std::cout << $"Requested time "<<TimeStep <<std::endl;

        // HELICS time granted 
        granted_time = h.helicsFederateRequestTime(vfed, TimeStep);
        std::cout << $"Granted time: "<<granted_time-1 <<std::endl;

        IsRepeating = true;
        HasViolations = true;

        Iter = 0; // Iteration number

        // Initial publication of thermal power request equivalent to PGMAX for time = 0 and iter = 0;
        if (TimeStep == 0)
        {
            //granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            MappingFactory.PublishElectricPower(granted_time - 1, Iter, P[TimeStep], ElectricPub);
        }

        // Set time step info
        currenttimestep = new TimeStepInfo() { timestep = TimeStep, itersteps = 0 };
        timestepinfo.Add(currenttimestep);

        while (IsRepeating && Iter<iter_max)
        {

            // Artificial delay
            if (TimeStep > AfterTimeStep)
            {
                Thread.Sleep(delay);
            }

            // stop iterating if max iterations have been reached                    
            Iter += 1;
            currenttimestep.itersteps += 1;

            granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            double GasMin = h.helicsInputGetDouble(SubToGasMin);
            std::cout << String.Format("Electric-Received: Time {0} \t iter {1} \t PthgMargin = {2:0.0000} [MW]", TimeStep, Iter, GasMin));

            granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            // get available thermal power at nodes, determine if there are violations
            HasViolations = MappingFactory.SubscribeToGasThermalPower(granted_time - 1, Iter, P[TimeStep], SubToGas, ElecLastVal); 

            //get currently required thermal power                 
            //double HR = 5 + 0.5 * P[TimeStep] - 0 * P[TimeStep] * P[TimeStep];
            //double ThermalPower = HR / 3.6 * P[TimeStep]; //Thermal power in [MW]; // eta_th=3.6/HR[MJ/kWh]

            //ElecLastVal.Add(valPth);

            if (HasViolations)
            {
                if (GasMin < 0 && P[TimeStep]>10)
                { 
                    P[TimeStep] -=5; 
                }                       
            }

            PNew[TimeStep] = P[TimeStep];

            if (Iter == iter_max && HasViolations)
            {
                CurrentDiverged = new TimeStepInfo() { timestep = TimeStep, itersteps = Iter };
                notconverged.Add(CurrentDiverged);
            }

            // iterative HELICS time request                    
            std::cout << "Requested time: "<<TimeStep<<" , iteration: "<<Iter<<std::endl;
            // HELICS time granted
            granted_time = h.helicsFederateRequestTimeIterative(vfed, TimeStep, iter_flag, out helics_iter_status);
            std::cout << $"Granted time: {granted_time-1},  Iteration status: {helics_iter_status}" <<std::endl;

            // Using an offset of 1 on the granted_time here because HELICS starts at t=1 and SAInt starts at t=0                        
            MappingFactory.PublishElectricPower(granted_time - 1, Iter, P[TimeStep], ElectricPub);

            IsRepeating = HasViolations;
        }
    }

    // request time for end of time + 1: serves as a blocking call until all federates are complete
    requested_time = total_time;
    std::cout << "Requested time: " << requested_time <<std::endl;
    h.helicsFederateRequestTime(vfed, requested_time);



    // finalize federate
    h.helicsFederateFinalize(vfed);
    std::cout << "Electric: Federate finalized" <<std::endl;
    h.helicsFederateFree(vfed);
    // If all federates are disconnected from the broker, then close libraries
    h.helicsCloseLibrary();




    // Diverging time steps
    if (notconverged.Count == 0)
        std::cout << "\n Electric: There is no diverging time step.");
    else
    {
        std::cout << "Electric: the solution diverged at the following time steps:");
        foreach (TimeStepInfo x in notconverged)
        {
            std::cout << "Time-step " << x.timestep <<std::endl;
        }
        std::cout << "\n Electric: The total number of diverging time steps = { notconverged.Count }");
    }

    return 0;
}
