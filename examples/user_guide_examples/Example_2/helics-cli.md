### Setting Up

1. make sure that helics cli executable is in your installed helics /bin folder, and that the appropriate lib is in the /lib folder 

   example:
   
   i. downloaded helics-cli from https://github.com/GMLC-TDC/helics-cli
   
   ii. put 'helics' in /Users/[username]/Software/HELICS/bin
   
   iii. edit PATH in .bashrc to point to directory of binaries
   
   ```
   export PATH=/Users/[username]/Software/HELICS/bin:$PATH
   ```
   
   and DYLD\_LIBRARY\_PATH to point to directory of libraries
   
   ```
   export DYLD_LIBRARY_PATH=/Users/[username]/helics_install/lib:$DYLD_LIBRARY_PATH
   ```
   
   
2. executing a cosim can be as simple as:
	
	helics run --path=runner.json
	
	'helics' is the helics-cli executable
	'run' tells helics-cli to start
	'--path' tells helics-cli where to find the federates.
	
3. Let's take a look inside runner.json:

```
	{
  "federates": [
    {
      "directory": ".",
      "exec": "python -u EV0.py 1",
      "host": "localhost",
      "name": "EV_federate"
    },
    {
      "directory": ".",
      "exec": "python -u EVController0.py 1",
      "host": "localhost",
      "name": "EVController_federate"
    }

  ],
  "name": "EV_toy"
}
```

This json file tells helics-cli a few things:

- There are two types of federates: one named EV0.py, the other named EVController0.py

- Both federates are located in the current directory
 
- There are only one of each federate

- The name of the federation is EV_toy





   