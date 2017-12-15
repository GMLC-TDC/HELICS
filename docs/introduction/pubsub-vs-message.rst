Pub/Sub vs Message
==================


+--------------------------------+-------------------------------+
|       Publish/Subscribe        |            Message            |
+================================+===============================+
| Fixed routes at initialization | Routes at transmission time   |
+--------------------------------+-------------------------------+
| 1 to N relationship            | All endpoints are routable    |
|                                | - Unless specified otherwise  |
+--------------------------------+-------------------------------+
| Values exist until updated     | Destination specified         |
+--------------------------------+-------------------------------+
| Default values                 | Rerouting through filters     |
+--------------------------------+-------------------------------+
| Associated units               | Data exists as singular blobs |
|                                | - No records kept             |
+--------------------------------+-------------------------------+
| Publisher is Fixed             | May define a message time     |
|                                | - Act as events               |
+--------------------------------+-------------------------------+
| No direct request mechanism    |                               |
+--------------------------------+-------------------------------+

