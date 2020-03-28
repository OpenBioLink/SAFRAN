# IRIFAB

IRIFAB (Improved rule inference for AnyBurl) is a application for inferencing the results of symbolic rules and create candidate rankings of them. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.

There are two versions of the rule applicator. 
+ The ***CPU*** version is designed for the excecution on one or multiple CPU threads. 
+ The ***CUDA*** version was designed for the excecution on CUDA-capable GPU's, but does currently ***NOT*** work

## Download binaries

[Windows x64](https://github.com/OpenBioLink/IRIFAB/blob/master/binaries/windows/IRIFAB_x64.exe)

[Windows x86](https://github.com/OpenBioLink/IRIFAB/blob/master/binaries/windows/IRIFAB_WIN32.exe)

[Linux](https://github.com/OpenBioLink/IRIFAB/blob/master/binaries/linux/IRIFAB)

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

## Performance
