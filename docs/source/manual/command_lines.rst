Command lines
=============

* Windows

.. code-block:: bash

    SAFRAN.exe {action} {path_to_config}

* Linux

.. code-block:: bash

    ./SAFRAN {action} {path_to_config}

where ``action`` is one of

:applymax: Application of rules using the MaxPlus aggregation
:applynoisy: Application of rules using the Noisy-OR aggregation
:applynrnoisy: Application of rules using the Non-redundant Noisy-OR aggregation
:learnnrnoisy: Clustering of rules for the Non-redundant Noisy-OR aggregation
:calcjacc: Calculation of similarity matrices of rules for each relation (needed for clustering)

and ``path_to_config`` is a path to a file containing the configuration of an action as key-value pairs. Details to the configuration of a specific action can be seen on the `Actions page <./actions>`_. The properties file should have the following format:

::

   {KEY} = {VALUE}
   {KEY} = {VALUE}
   ...