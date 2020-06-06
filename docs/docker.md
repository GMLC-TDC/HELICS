# Docker installation

## Requirements

Docker version 19

## Getting a docker from the hub

To search a docker from any repository you can use this command

```bash
docker search helics
```

|            NAME            |                                                       DESCRIPTION                                                        |
| :------------------------: | :----------------------------------------------------------------------------------------------------------------------: |
|       helics/octave        |                                               container for testing octave                                               |
|      helics/buildenv       | containers for helping with the CI test of helics, including building on different compilers in different configurations |
|     helics/clang-tsan      |                                       container for running clang thread sanitizer                                       |
| helics/buildenv:sanitizers |                                          container for running clang sanitizers                                          |
|       helics/helics        |                                       container with installed HELICS executables                                        |

the `helics/helics` repository contains a number of tags corresponding to different versions of helics with the all the apps and executables present for each different version.

```bash
docker pull helics/helics:develop
```

## Build a new docker image

```bash
docker build -t clang-test -f config/Docker/Dockerfile-HELICS-apps .
```

The HELICS and Sanitizers Dockerfiles will accept a MAKE_PARALLEL build argument that can be used to set how many threads make uses. On machines with low memory such as those used by CI services, setting this too high can result in out of memory compiler errors.

```bash
docker build -t clang-test -f config/Docker/Dockerfile-HELICS-apps --build-arg MAKE_PARALLEL=12 .
```

In addition to this, the HELICS-apps Dockerfile for the HELICS apps currently accepts an ENABLE_GITHUB argument (defaults to false) that when set to true will replace the copied current source directory with a copy of the HELICS source code checked out from GitHub. Due Docker not allowing conditional copy commands, it is recommended to run the docker build from a relatively empty working directory. It will also take a GIT_BRANCH argument (defaults to develop) that can be used to control which GitHub branch or tagged version gets checked out.

```bash
docker build -t helics-apps-test -f config/Docker/Dockerfile-HELICS-apps --build-arg ENABLE_GITHUB=true --build-arg GIT_BRANCH=v2.4.0 .
```

## Working with dockerhub

```bash
docker images  # will show all images available on your machines
```

|    REPOSITORY    |  TAG   |   IMAGE ID   |   CREATED   |  SIZE  |
| :--------------: | :----: | :----------: | :---------: | :----: |
| helics-apps-test | latest | a2b679e23225 | 2 hours ago | 1.96GB |

```bash
docker tag a2b679e23225  helics/helics:latest  # this will tag the image ID for docker repository helics/helics
docker push helics/helics:latest         # This will push the image to docker hub repository
```

Any user can pull you docker image using the following command:

```bash
docker pull helics/helics:latest
```

Helics docker can be found on the following web site.

[https://cloud.docker.com/u/helics/repository/list](https://cloud.docker.com/u/helics/repository/list)

## Remove a docker image

```bash
docker image rmi a2b679e23225 -f   # using -f force to remove the image id
```

## Run a interactive shell using a docker image as a container

```bash
docker run -it helics/helics /bin/bash
```

You can see what container is running with the `ps` command

```bash
docker ps
```

| CONTAINER ID |     IMAGE     |   COMMAND   |    CREATED    |    STATUS    | PORTS |      NAMES       |
| :----------: | :-----------: | :---------: | :-----------: | :----------: | :---: | :--------------: |
| 98d7005cba00 | helics/helics | "/bin/bash" | 2 seconds ago | Up 2 seconds |   -   | wizardly_gagarin |

## Working with docker container

When you run a image, docker creates a container, as soon as you exit, the container is destroyed.
You can detached from container (like the application screen) and reattach later.

- to detached: `CTRL-P CTRL-D`

- to reattached the container found in the table above:

```bash
docker ps
docker attach  <Container ID> or <Container Name>
docker attach wizardly_gagarin
...
or
...
docker attach 98d7005cba00
```

If you modified a container and you would like to save the modification, you can use the commit command.

```bash
docker commit -m "I modified this container"  620c7588882e helics-modified
```

> **NOTE:** The number is the Container Id found with `docker ps`

to display all container

```bash
docker ps -a
```

## Remove all stopped containers

```bash
docker rm $(docker ps -a -q)
```

## Reference

All docker command can be found here:

[https://docs.docker.com/engine/reference/commandline/cli/](https://docs.docker.com/engine/reference/commandline/cli/)
