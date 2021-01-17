# Co-simulation Stages

Helics has several stages to the co-simulation. Creation, initialization, execution, and final state. The helicsFederateEnterExecutingMode is the transition between initialization and execution. In the init mode values can be exchanged prior to time beginning. Normally values published in init mode are available at time 0, but if the iteration is used they can be available inside the initialization mode. There are 3 available options for the iterate parameter.

`no_iteration` -- don't iterate

`force_iteration` -- guaranteed iteration, stay in iteration mode

`iterate_if_needed` -- If there is data available from other federates Helics will iterate, if no additional data is available it will move to execution mode and have granted time=0.

ou could manage the timing of the federation to use the `initialize` mode in HELICS. This would allow you to communicate initial conditions (`t=0`) information prior to the start of execution mode. It would also allow you to publish out the `t=1` information so that all federates would have access to it once they were granted `t=1`. Look into `helicsFederateEnterInitializingMode()`.