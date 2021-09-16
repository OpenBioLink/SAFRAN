Action *applynoisy*
-------------------

Application of rules using noisy-or aggregation approach.

.. math::
    score(e) = 1 - \prod_{i=1}^{k} (1 - conf(r_i))

where :math:`r_1, \dots, r_k` are rules that predict entity :math:`e`.

Configuration file
^^^^^^^^^^^^^^^^^^

Input : 
   * PATH_TRAINING : Valid path (file)
        Path to training file (absolute or relative), **default: train.txt**
   * PATH_TEST : Valid path (file)
        Path to test file (absolute or relative), **default: test.txt**
   * PATH_VALID : Valid path (file)
        Path to validation file (absolute or relative), **default: valid.txt**
   * PATH_RULES : Valid path (file)
        Path to rules file (absolute or relative), **default: rules.txt**

Properties :
   * WORKER_THREADS : int
        Number of threads that are used for computation. (-1 means all threads are used), **default: -1**
   * DISCRIMINATION_BOUND : int
        Discriminates (omits) rules which predict more elements than this, 0 means no limit., **default: 4000**
   * UNSEEN_NEGATIVE_EXAMPLES : int
        The number of negative examples for which we assume that they exist, however, we have not seen them. Rules with high coverage are favoured the higher the chosen number, **default: 5**
   * REFLEXIV_TOKEN : string
        Token used for substitution of reflexive rules. (Used if AnyBURL ruleset was trained with REWRITE_REFLEXIV = TRUE), **default: me_myself_i**
   * TOP_K_OUTPUT : int
        The top-k results that are after filtering kept in the results, **default: 10**

Output : 
    * PATH_OUTPUT : Valid path (file)
        Path to file used for storing predictions, **default: predictions.txt**