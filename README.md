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

Performance test performed on an Intel i7-6500U CPU @ 2.50GHz, 2 Cores, 4 Logical Processors. (Averaged over 10 runs)

Each test was run with the following properties:

```
UNSEEN_NEGATIVE_EXAMPLES = 5
TOP_K_OUTPUT = 10
WORKER_THREADS = 3
```

The column "Configuration" shows the different configurations of the following properties:

```
FAST DISCRIMINATION_UNIQUE INTERMEDIATE_DISCRIMINATION DISCRIMINATION_BOUND
```

While 0 1 0 is closest to the original implementation and 1 0 1 is the fastest.

### FB15k

Dataset consisting of:

+ Train samples: 483,142
+ Test samples: 59,071
+ Validation samples: 50,000

Ruleset (alpha-50) size: 106,480

#### IRIFAB

| Configuration | Runtime file reading and preperation (ms) | Runtime rule application (ms) | hits@1 | hits@3 | hits@10 |
| ------------- | ----------------------------------------- | ----------------------------- | ------ | ------ | ------- |
| 0 0 0 1000    | 11,751                                    | 18,956                        | 0.7933 | 0.8292 | 0.8640  |
| 0 0 1 1000    | 11,852                                    | 18,536                        | 0.7934 | 0.8292 | 0.8639  |
| 0 1 0 1000    | 11,780                                    | 25,282                        | 0.7933 | 0.8292 | 0.8643  |
| 0 1 1 1000    | 11,778                                    | 24,700                        | 0.7933 | 0.8292 | 0.8641  |
| 1 0 0 1000    | 11,834                                    | 9,844                         | 0.7933 | 0.8292 | 0.8640  |
| 1 0 1 1000    | 11,796                                    | 9,714                         | 0.7934 | 0.8292 | 0.8639  |
| 1 1 0 1000    | 11,654                                    | 10,450                        | 0.7933 | 0.9292 | 0.8643  |
| 1 1 1 1000    | 11,714                                    | 10,370                        | 0.7933 | 0.8292 | 0.8641  |
| 0 1 0 4000    | 11,868                                    | 27,280                        | 0.8090 | 0.8445 | 0.8783  |
| 1 0 1 4000    | 11,763                                    | 10,444                        | 0.8090 | 0.8445 | 0.8782  |

#### Original AnyBURL Rule Application

| Configuration | Runtime file reading and preperation (ms) | Runtime rule application (ms) | hits@1 | hits@3 | hits@10 |
| ------------- | ----------------------------------------- | ----------------------------- | ------ | ------ | ------- |
| 0 1 0* 1000   | 7,170                                     | 148,877                       | 0.8094 | 0.8443 | 0.8785  |
| 0 1 0* 4000   | 7,053                                     | 203,534                       | 0.8094 | 0.8443 | 0.8785  |

*) You cannot actually set these  parameters (FAST DISCRIMINATION_UNIQUE INTERMEDIATE_DISCRIMINATION DISCRIMINATION_BOUND) in the AnyBURL implementation. Here 0 1 0 just represents a run with the original AnyBURL implementation and the specified DISCRIMINATION_BOUND set.

### OpenBioLink (Positive edges only)

Dataset consisting of:

+ Train samples: 4,193,905
+ Test samples: 180,310
+ Validation samples: 183,652

Ruleset (alpha-50) size: 4,845

| Configuation | Runtime file reading and preperation (ms) | Runtime rule application | hits@1 | hits@3 | hits@10 |
| ------------ | ----------------------------------------- | ------------------------ | ------ | ------ | ------- |
| 0 1 0 1000   | 43,585                                    | 276 mins (4.4 h)         | 0.1160 | 0.2107 | 0.3514  |
|              |                                           |                          |        |        |         |
|              |                                           |                          |        |        |         |

