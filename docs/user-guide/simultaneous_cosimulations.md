# Simultaneous Co-simulations
Sometimes it is necessary or desirable to be able to execute multiple simultaneous simulations on a single computer system.  Either for increased parallelism or from multiple users or as part of a larger coordinated execution for sensitivity analysis or uncertainty quantification.  HELICS includes a number of different options for managing this and making it easier.  

## General Notes
HELICS starts with some default port numbers for network communication, so only a single broker(per core type) with default options is allowed to be running on a single computer at a given time.  
