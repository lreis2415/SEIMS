
## Usage

TODO

## Test for developers
The <tag> can be `apline`, `apline-with-gdal`, `apline-with-mongodb`, and `apline-with-gdal-mongodb`.
```
docker build -t crazyzlj/ccgl:<tag>-test -f docker/test/<tag>/Dockerfile .

# or add `--progress=plain` to print all output from shell

docker build --progress=plain -t crazyzlj/ccgl:<tag>-test -f docker/test/<tag>/Dockerfile .
```

### CCGL with the support of MongoDB

See the [official tutorial](https://www.mongodb.com/compatibility/docker).

+ Select a proper tag of the official mongo image in [supported-tags-and-respective-dockerfile-links](https://github.com/docker-library/docs/blob/master/mongo/README.md#supported-tags-and-respective-dockerfile-links), e.g., `4.4.14-focal`.
+ Start a mongo server instance: `docker run --name some-mongo -d -p <client_port>:27017 mongo:tag`,
where `some-mongo` is the name you want to assign to your container and 
`tag` is the tag specifying the MongoDB version you want,
`client_port` is the port you want to exposed to your localhost, e.g., `27017` or any other legal number.
For example, 
  + `docker network create docker-mongo-network`
  + `docker run -d -p 27018:27017 --name mongodb-docker mongo:4.4.14-focal`
  + `docker ps` will show something like:
    ```
    CONTAINER ID   IMAGE                           COMMAND                  CREATED         STATUS         PORTS                                 NAMES
    6cef37edee6a   mongo:4.4.14-focal              "docker-entrypoint.s…"   5 seconds ago   Up 4 seconds   0.0.0.0:27018->27017/tcp   mongodb-docker
    ```
  + Run `docker logs mongodb-docker` to check the logs which provide a wealth of useful information.
  + Now, you can connect to the MongoDB running in the container from *the host* host via `localhost:27018`.
  
+ Mount data directory of host using `-v </path/to/any>:/data/db`, e.g., 
`docker run -d -v /Users/ljzhu/Documents/data/docker_mongodata:/data/db -p 27017:27017 --name mongodb-docker mongo:4.4.14-focal`

+ There are couple of ways to do [Connect to MongoDB from another Docker container](https://github.com/docker-library/docs/blob/master/mongo/README.md#connect-to-mongodb-from-another-docker-container). 
Also refers to one userful answer in [stackflow](https://stackoverflow.com/a/43962099).
  + (Currently tested) Use mongodb container ip address: `docker inspect -f '{{.NetworkSettings.IPAddress}}' mongo-docker`.
    The command will output the IP of the mongodb container which can be used in another application's container.
    For example, `./unittestd -host 172.17.0.3 -port 27017`

    The IP of the mongodb container can be passed as an argument when build another docker image,
    also see [here](https://stackoverflow.com/a/34254700):

    `docker build --progress=plain -t crazyzlj/ccgl:alpine-with-mongodb-test --build-arg host=172.17.0.2 --build-arg port=27017 -f docker/test/alpine-with-mongodb/Dockerfile .`
  + ...

## Release docker images

The <tag> can be `apline`, `apline-with-gdal`, `apline-with-mongodb`, and `apline-with-gdal-mongodb`.
The <ver> can be `latest` or any others, e.g., `v1.0`.
```
cd CCGL
docker buildx create --use
docker buildx build --platform linux/amd64,linux/arm64 --push -t crazyzlj/ccgl:<tag>-<ver> -f docker/<tag>/Dockerfile .
```
