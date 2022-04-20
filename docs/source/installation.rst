Installation
============

You can find download links to prebuilt executables (recommended) for your operating system below. If you want the latest development version you can build SAFRAN from source, which is described below.

Download
--------

Prebuild binaries can be found here (only x64):

* `Linux <https://github.com/OpenBioLink/SAFRAN/releases/download/v1.0.0/Linux.7z>`_
* `Windows (MSVC) <https://github.com/OpenBioLink/SAFRAN/releases/download/v1.0.0/Windows-MSVC.7z>`_
* `Windows (MinGW) <https://github.com/OpenBioLink/SAFRAN/releases/download/v1.0.0/Windows-MinGW.7z>`_
* `MacOS <https://github.com/OpenBioLink/SAFRAN/releases/download/v1.0.0/macOS.7z>`_


Build from source
-----------------

1. Clone the SAFRAN repository
2. Download and extract boost 1.76.0 to project root directory

   1. Windows: `https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip <https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip>`_
   2. Unix: `https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz <https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz>`_

3. Have cmake installed (> 9.6.0)
4. Create and change to directory **build**
5. Run ``cmake ../``
6. Run ``make`` (Unix) or ``cmake --build .`` (Windows)