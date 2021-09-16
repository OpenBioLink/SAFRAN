Trial
=====

Trial properties
----------------

+----------------+----------------+----------------+----------------+
| Key            | Value type     | Description    | Default        |
+================+================+================+================+
| TRIAL          | [0|1]          | If set to 1,   | 0              |
|                |                | rules are only |                |
|                |                | applied to a   |                |
|                |                | representative |                |
|                |                | sample of all  |                |
|                |                | testtriples,   |                |
|                |                | sample size is |                |
|                |                | calculated     |                |
|                |                | according to   |                |
|                |                | CO             |                |
|                |                | NFIDENCE_LEVEL |                |
|                |                | and            |                |
|                |                | M              |                |
|                |                | ARGIN_OF_ERROR |                |
+----------------+----------------+----------------+----------------+
| PA             | Valid path     | Path to the    | "te            |
| TH_TEST_SAMPLE |                | testtriples of | st_sample.txt" |
|                |                | the sample     | (relative to   |
|                |                | (Used for      | exe)           |
|                |                | evaluation),   |                |
|                |                | can be         |                |
|                |                | absolute or    |                |
|                |                | relative to    |                |
|                |                | application    |                |
|                |                | (file is       |                |
|                |                | created, if it |                |
|                |                | already exists |                |
|                |                | it is          |                |
|                |                | overwritten)   |                |
+----------------+----------------+----------------+----------------+
| CO             | [8             | Confidence     | 95             |
| NFIDENCE_LEVEL | 0|85|90|95|99] | level of       |                |
|                |                | evaluation     |                |
|                |                | results        |                |
+----------------+----------------+----------------+----------------+
| M              | Integer        | Margin of      | 5              |
| ARGIN_OF_ERROR | (Percent)      | error +- of    |                |
|                |                | evaluation     |                |
|                |                | results        |                |
+----------------+----------------+----------------+----------------+