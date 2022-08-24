
## Usage

```
docker pull crazyzlj/taudem_ext:alpine-openmpi-gdal-latest
cd TauDEM_ext
mkdir -p data/results
#
# TauDEM cmd: docker run -v $(pwd)/data:/data crazyzlj/taudem_ext:alpine-openmpi-gdal-latest [-np <nproc>] <TauDEMToolName> <Arguments>
# where, nproc is the number of MPI processes, the default is the number of processors assigned to the Docker, the "-np <nproc>" is optional.
#
# For example,
# 1. pit remove
docker run -v $(pwd)/data:/data crazyzlj/taudem_ext:alpine-openmpi-gdal-latest pitremove -z /data/logan.tif -fel /data/results/loganfel.tif
# or specify -np 2
docker run -v $(pwd)/data:/data crazyzlj/taudem_ext:alpine-openmpi-gdal-latest -np 2 pitremove -z /data/logan.tif -fel /data/results/loganfel.tif
# 2. d8 flow direction
docker run -v $(pwd)/data:/data crazyzlj/taudem_ext:alpine-openmpi-gdal-latest d8flowdir -fel /data/results/loganfel.tif -p /data/results/logand8.tif
```

## Test for developers
The <tag> can be any tag you want, e.g., `crazyzlj/taudem_ext`.

```
docker build -t <tag> -f docker/Dockerfile .
```

## Release docker images

The <tag> can be any tag you want, e.g., `crazyzlj/taudem_ext`.

The <ver> can be `latest` or any others, e.g., `apline-openmpi-gdal-latest`.
```
cd TauDEM_exe
docker buildx create --use
docker buildx build --platform linux/amd64,linux/arm64 --push -t <tag>:<ver> -f docker/Dockerfile .
# e.g.,
docker buildx build --platform linux/amd64,linux/arm64 --push -t crazyzlj/taudem_ext:apline-openmpi-gdal-latest -f docker/Dockerfile .
```
