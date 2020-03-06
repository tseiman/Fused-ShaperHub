# Mounting Octave, MOC

a proof of concept to mount Octave into local file system

## Dependencies

* cmake
* libjansson
* libcurl
* libfuse

e.g. on Ubuntu (tested with 18.04 LTS):

```
sudo apt-get install cmake libjansson-dev libcurl4-openssl-dev libfuse-dev
```

## Build

```
git clone https://github.com/tseiman/MountOctave.git
cd MountOctave
cmake -DCMAKE_BUILD_TYPE=Debug -DIS_MOC=ON .
make

```

## Run
```
mkdir testdir
./bin/mount-octave -f testdir/

```
Check content of testdir

