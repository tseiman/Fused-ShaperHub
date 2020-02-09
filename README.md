# Mounting Octave, MOC

a proof of concept to mount Octave into local file system

## Dependencies

* cmake
* libjansson
* libcurl
* libfuse

e.g. on Ubuntu:

```
sudo apt-get install cmake libjansson-dev libcurl4-openssl-dev libfuse-dev
```

## Build

```
cmake -DCMAKE_BUILD_TYPE=Debug .
make

```

## Run
```
mkdir testdir
./bin/mount-octave -d  -f testdir/

```
Check content of testdir

