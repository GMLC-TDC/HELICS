# Other notes for developers

- If making a new federate class in the application API which inherits from any of the existing ones (except Federate itself) you need to be aware of the virtual inheritance in ValueFederate and MessageFederate and make sure to call the constructor of Federate; failing to do so will produce non-functional code.
