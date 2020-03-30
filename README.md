# IRIFAB

IRIFAB (Improved rule inference for AnyBurl) is a application for inferencing the results of symbolic rules and create candidate rankings of them. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.

There are two versions of the rule applicator. 
+ The ***CPU*** version is designed for the excecution on one or multiple CPU threads. 
+ The ***CUDA*** version was designed for the excecution on CUDA-capable GPU's, but does currently ***NOT*** work

## Download binaries

[Windows x64](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/windows/IRIFAB.exe)

[Linux](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/linux/IRIFAB)

[Example config-apply.properties](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/config-apply.properties)

## Build from source

1. Have cmake installed
2. Create folder **build** in {PATH_TO_PROJECT}/CPU/
3. Switch to {PATH_TO_PROJECT}/CPU/build
4. Run `cmake ../`

If building using Visual Studio
1. Open IRIFAB.sln
2. Select **Release** as build type
4. Click on **Build** → **Build Solution**

Else
1. Run `make` from **build** folder

## Manual

To run IRIFAB a properties file called apply-config.properties is required as an startup argument.

#### Windows

`IRIFAB.exe apply-config.properties`

#### Linux

`./IRIFAB apply-config.properties`

#### Properties file

The properties file should have the following format 
```
{KEY} = {VALUE}
{KEY} = {VALUE}
...
```


| Key                           | Value type     | Description                                                  | Default |
| ----------------------------- | -------------- | ------------------------------------------------------------ | ------- |
| PATH_TRAINING                 | Valid path     | Path to the training triples                                 |         |
| PATH_TEST                     | Valid path     | Path to the test triples                                     |         |
| PATH_VALID                    | Valid path     | Path to the validation triples                               |         |
| PATH_RULES                    | Valid path     | Path to the rule set                                         |         |
| PATH_OUTPUT                   | Valid path     | Path to the output file                                      |         |
| DISCRIMINATION_BOUND          | Integer        | Returns only results for head or tail computation if the results set has less elements than this bound. The idea is that any results set which has more elements is anyhow not useful for a top-k ranking.  Should be set to a value thats higher than the k of the requested top-k (however, the higher the value the more runtime is required) | 1000    |
| UNSEEN_NEGATIVE_EXAMPLES      | Integer        | The number of negative examples for which we assume that they exist, however, we have not seen them. Rules with high coverage are favored the higher the chosen number. | 5       |
| TOP_K_OUTPUT                  | Integer        | The top-k results that are after filtering kept in the results. | 10      |
| WORKER_THREADS                | Integer        | Amount of threads that are started to compute the ranked results. | 3       |
| THRESHOLD_CORRECT_PREDICTIONS | Integer        | The threshold for the number of correct prediction created with the refined rule. | 2       |
| THRESHOLD_CONFIDENCE          | Floating point | The threshold for the confidence of the refined rule         | 0.001   |
| FAST                          | [0\|1]         | If 0 the original rule engine is used (for each testtriple -> for each rule)<br />If 1 a faster version is used which caches intermediate results and requires more memory | 0       |
| DISCRIMINATION_UNIQUE         | [0\|1]         | If 1 the unique results are calculated before discrimination.  (Original 1, More performant: 0) | 1       |
| INTERMEDIATE_DISCRIMINATION   | [0\|1]         | If 1 not only the final results are discriminated, but also intermediate results, see DISCRIMINATION_BOUND | 0       |

## Performance
