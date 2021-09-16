Properties
============

The properties file should have the following format:

::

   {KEY} = {VALUE}
   {KEY} = {VALUE}
   ...

Common properties
------------------

Are used in every action.

+-----------------+-----------------+-----------------+-------------+
| Key             | Value type      | Description     | Default     |
+=================+=================+=================+=============+
| PATH_TRAINING   | Valid path      | Path to         | train.txt   |
|                 | (file)          | training file   |             |
|                 |                 | (absolute or    |             |
|                 |                 | relative)       |             |
+-----------------+-----------------+-----------------+-------------+
| PATH_TEST       | Valid path      | Path to test    | test.txt    |
|                 | (file)          | file (absolute  |             |
|                 |                 | or relative)    |             |
+-----------------+-----------------+-----------------+-------------+
| PATH_VALID      | Valid path      | Path to         | valid.txt   |
|                 | (file)          | validation file |             |
|                 |                 | (absolute or    |             |
|                 |                 | relative)       |             |
+-----------------+-----------------+-----------------+-------------+
| PATH_RULES      | Valid path      | Path to rule    | rules.txt   |
|                 | (file)          | set file        |             |
|                 |                 | (absolute or    |             |
|                 |                 | relative)       |             |
+-----------------+-----------------+-----------------+-------------+
| DISCR           | Integer         | Discriminates   | 4000        |
| IMINATION_BOUND |                 | rules which     |             |
|                 |                 | result sets     |             |
|                 |                 | have more       |             |
|                 |                 | elements than   |             |
|                 |                 | this bound.     |             |
|                 |                 | (Also used for  |             |
|                 |                 | limiting memory |             |
|                 |                 | consumption.) 0 |             |
|                 |                 | means no limit. |             |
+-----------------+-----------------+-----------------+-------------+
| UNSEEN_NE       | Integer         | The number of   | 5           |
| GATIVE_EXAMPLES |                 | negative        |             |
|                 |                 | examples for    |             |
|                 |                 | which we assume |             |
|                 |                 | that they       |             |
|                 |                 | exist, however, |             |
|                 |                 | we have not     |             |
|                 |                 | seen them.      |             |
|                 |                 | Rules with high |             |
|                 |                 | coverage are    |             |
|                 |                 | favoured the    |             |
|                 |                 | higher the      |             |
|                 |                 | chosen number.  |             |
+-----------------+-----------------+-----------------+-------------+
| REFLEXIV_TOKEN  | String          | Token used for  | me_myself_i |
|                 |                 | substitution of |             |
|                 |                 | reflexive       |             |
|                 |                 | rules. (Used if |             |
|                 |                 | ruleset was     |             |
|                 |                 | trained with    |             |
|                 |                 | R               |             |
|                 |                 | EWRITE_REFLEXIV |             |
|                 |                 | = TRUE)         |             |
+-----------------+-----------------+-----------------+-------------+
| TOP_K_OUTPUT    | Integer         | The top-k       | 10          |
|                 |                 | results that    |             |
|                 |                 | are after       |             |
|                 |                 | filtering kept  |             |
|                 |                 | in the results. |             |
+-----------------+-----------------+-----------------+-------------+
| WORKER_THREADS  | Integer         | Amount of       | -1          |
|                 |                 | threads that    |             |
|                 |                 | are used for    |             |
|                 |                 | computation.    |             |
|                 |                 | (-1 means all   |             |
|                 |                 | threads are     |             |
|                 |                 | used)           |             |
+-----------------+-----------------+-----------------+-------------+

calcjacc
---------------

Calculates the similarity matrices (Jaccard index) of all relations used
for aggregating with non-redundant noisy-or. The Jaccard index is
estimated using MinHash. Output: Binary files storing the Jaccard
indices between rules for each relation.

+--------------+--------------------+--------------------+---------+
| Key          | Value type         | Description        | Default |
+==============+====================+====================+=========+
|              |                    | General properties |         |
|              |                    | (see table above)  |         |
+--------------+--------------------+--------------------+---------+
| PATH_JACCARD | Valid path         | Path to the        | jaccard |
|              | (directory)        | directory used for |         |
|              |                    | storing the binary |         |
|              |                    | similarity matrix  |         |
|              |                    | files              |         |
+--------------+--------------------+--------------------+---------+
| RESOLUTION   | Integer            | Sets the accuracy  | 200     |
|              |                    | of the Jaccard     |         |
|              |                    | estimation. The    |         |
|              |                    | number of hash     |         |
|              |                    | functions used in  |         |
|              |                    | MinHash (f.e.      |         |
|              |                    | RESOLUTION = 200   |         |
|              |                    | --> 200 hash       |         |
|              |                    | functions --> Max  |         |
|              |                    | resolution of      |         |
|              |                    | Jaccard 1/200)     |         |
+--------------+--------------------+--------------------+---------+
| SEED         | Integer            | Seed for           | 0       |
|              |                    | generating hash    |         |
|              |                    | functions used in  |         |
|              |                    | MinHash            |         |
+--------------+--------------------+--------------------+---------+

learnnrnoisy
-------------------

Learns the optimal thresholds for clustering the rules on similarity.
There are two possible search strategies: grid search and random search.

**Requires calculation of similarity matrices (calcjacc).**

+--------------+----------------+----------------+----------------+
| Key          | Value type     | Description    | Default        |
+==============+================+================+================+
|              |                | General        |                |
|              |                | properties     |                |
|              |                | (see table     |                |
|              |                | above)         |                |
+--------------+----------------+----------------+----------------+
| PATH_JACCARD | Valid path     | Path to the    | jaccard        |
|              | (directory)    | directory      |                |
|              |                | containing the |                |
|              |                | binary         |                |
|              |                | similarity     |                |
|              |                | matrix files   |                |
+--------------+----------------+----------------+----------------+
| PATH_CLUSTER | Valid path     | Path to file   | cluster.txt    |
|              | (file)         | used for       |                |
|              |                | storing        |                |
|              |                | clustering     |                |
|              |                | results        |                |
+--------------+----------------+----------------+----------------+
| BUFFER_SIZE  | Integer        | Buffer size    | Maximum        |
|              |                | (in amount of  | unsigned long  |
|              |                | integers, 4    | long           |
|              |                | byte) used to  |                |
|              |                | limit memory   |                |
|              |                | consumption of |                |
|              |                | buffering      |                |
|              |                | previously     |                |
|              |                | inferred       |                |
|              |                | rules. Should  |                |
|              |                | only be set if |                |
|              |                | running out of |                |
|              |                | memory.        |                |
|              |                | (2500000000    |                |
|              |                | --> ~10 GB)    |                |
+--------------+----------------+----------------+----------------+
| STRATEGY     | [grid|random]  | Sets the       | grid           |
|              |                | search         |                |
|              |                | strategy to be |                |
|              |                | used for       |                |
|              |                | finding        |                |
|              |                | optimal        |                |
|              |                | clustering     |                |
+--------------+----------------+----------------+----------------+
| ITERATIONS   | Integer        | Amount of      | 10000          |
|              |                | iterations     |                |
|              |                | used in random |                |
|              |                | search         |                |
|              |                | strategy       |                |
+--------------+----------------+----------------+----------------+
| RESOLUTION   | Integer        | Determines     | 200            |
|              |                | smallest       |                |
|              |                | possible       |                |
|              |                | (1/RESOLUTION) |                |
|              |                | change of the  |                |
|              |                | threshold.     |                |
|              |                | (Amount of     |                |
|              |                | iterations     |                |
|              |                | used in grid   |                |
|              |                | search         |                |
|              |                | strategy,      |                |
|              |                | Limitation of  |                |
|              |                | search space   |                |
|              |                | in random      |                |
|              |                | search)        |                |
+--------------+----------------+----------------+----------------+
| SEED         | Integer        | Seed for the   | 0              |
|              |                | sampling of    |                |
|              |                | thresholds     |                |
|              |                | used in random |                |
|              |                | search         |                |
|              |                | strategy       |                |
+--------------+----------------+----------------+----------------+

applynrnoisy
-------------------

+--------------+----------------+----------------+----------------+
| Key          | Value type     | Description    | Default        |
+==============+================+================+================+
|              |                | General        |                |
|              |                | properties     |                |
|              |                | (see table     |                |
|              |                | above)         |                |
+--------------+----------------+----------------+----------------+
| PATH_CLUSTER | Valid path     | Path to file   | cluster.txt    |
|              | (file)         | containing     |                |
|              |                | clustering     |                |
|              |                | results        |                |
+--------------+----------------+----------------+----------------+
| PATH_OUTPUT  | Valid path     | Path to file   | p              |
|              | (file)         | used for       | redictions.txt |
|              |                | storing        |                |
|              |                | predictions    |                |
+--------------+----------------+----------------+----------------+


applynoisy \| applymax
-------------------------------------

+-------------+-----------------+-----------------+-----------------+
| Key         | Value type      | Description     | Default         |
+=============+=================+=================+=================+
|             |                 | General         |                 |
|             |                 | properties (see |                 |
|             |                 | table above)    |                 |
+-------------+-----------------+-----------------+-----------------+
| PATH_OUTPUT | Valid path      | Path to file    | predictions.txt |
|             | (file)          | used for        |                 |
|             |                 | storing         |                 |
|             |                 | predictions     |                 |
+-------------+-----------------+-----------------+-----------------+

