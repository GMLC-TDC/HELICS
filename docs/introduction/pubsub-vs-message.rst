Value vs Message
==================

+--------------------------------------------------------------------+--------------------------------------------------------+
|`Publication <publications.html>`__/`Input <inputs.html>`__ Values  | `Endpoint <endpoints.html>`__                          |
+====================================================================+========================================================+
|Fixed routes at initialization                                      |Routes at transmission time                             |
+--------------------------------------------------------------------+--------------------------------------------------------+
|1 to N relationship (publications) N to 1 relationship for Inputs   |All endpoints are routable - Unless otherwise specified |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Values exist until updated                                          |Destination specified                                   |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Default values                                                      |Rerouting/modification through filters                  |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Associated units                                                    |Data exists as singular blobs - No records kept         |
+--------------------------------------------------------------------+--------------------------------------------------------+
|No direct request mechanism                                         |May define a message time - Act as events               |
+--------------------------------------------------------------------+--------------------------------------------------------+



Other Notes:

 - Endpoints can subscribe to publications to get a message for each data point
 - Both can be nameless to be non-routable from outside the defining federate
