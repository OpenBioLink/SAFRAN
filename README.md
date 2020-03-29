# IRIFAB

IRIFAB (Improved rule inference for AnyBurl) is a application for inferencing the results of symbolic rules and create candidate rankings of them. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.

There are two versions of the rule applicator. 
+ The ***CPU*** version is designed for the excecution on one or multiple CPU threads. 
+ The ***CUDA*** version was designed for the excecution on CUDA-capable GPU's, but does currently ***NOT*** work

## Download binaries

[Windows x64](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/windows/IRIFAB_x64.exe)

[Windows x86](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/windows/IRIFAB_WIN32.exe)

[Linux](https://github.com/OpenBioLink/IRIFAB/raw/master/binaries/linux/IRIFAB)

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

### Properties

| Parameter                     | Value type     | Description                        |
| ----------------------------- | -------------- | ---------------------------------- |
| PATH_TRAINING                 | Valid path     |                                    |
| PATH_TEST                     | Valid path     |                                    |
| PATH_VALID                    | Valid path     |                                    |
| PATH_RULES                    | Valid path     |                                    |
| PATH_OUTPUT                   | Valid path     |                                    |
| UNSEEN_NEGATIVE_EXAMPLES      | Integer        |                                    |
| TOP_K_OUTPUT                  | Integer        |                                    |
| WORKER_THREADS                | Integer        | Amount of threads that are started |
| THRESHOLD_CORRECT_PREDICTIONS | Integer        |                                    |
| THRESHOLD_CONFIDENCE          | Floating point |                                    |
| FAST                          | [0\|1]         |                                    |
| DISCRIMINATION_UNIQUE         | [0\|1]         |                                    |
| INTERMEDIATE_DISCRIMINATION   | [0\|1]         |                                    |

## Performance
