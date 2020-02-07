# Mounting Octave, MOC

a proof of concept to mount Octave into local file system

## Dependencies

* libjansson
* libcurl
* libfuse

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

