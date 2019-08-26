Docker installation
===================

Requirements
------------
Docker version 19.03.1

Build
-----

  . docker build -t clang-test -f config/Docker/Dockerfile-MSan .


working with dockerhub
----------------------

  . docker images  # will show all images available on your machines

:
    REPOSITORY                              TAG                 IMAGE ID            CREATED             SIZE
    clang-test                              latest              a2b679e23225        2 hours ago         1.96GB

  . docker tag a2b679e23225  helics/clang-test   # this will tag the image ID for docker hub helics
  . docker push helics/clang-test:latest         # This will push the image to docker hub


Any user can pull you docker image using the following command:

  . docker pull helics/clang-test:latest


Helics docker can be found on the following web site.

  . https://cloud.docker.com/u/helics/repository/list

Remove a docker image
---------------------

  . docker image rm a2b679e23225 -f   # using -f force to remove the image id

Run a shell using a docker image as a container
-----------------------------------------------

  . docker run -it helics/clang-test /bin/bash

You can see what container is running with the `ps` command

  . docker ps


Working with docker container
-----------------------------

When you run a image, docker creates a container, as soon as you exit, the container is destroyed.   
You can detached from container (like the application screen) and reattach later.  

. to detached:    CTRL-P CTRL-D
. to reattached:   
  . docker ps
  . docker attached  <Container ID>


If you modified a container and you would like to save the modification, you can use the commit command.

  . docker commit -m "I modified this container"  620c7588882e helics-modified

NOTE:   The number here the Container Id found with `docker ps`

to display all container

  . docker ps -a

Remove all stopped containers
-----------------------------

. docker rm $(docker ps -a -q)


Reference
---------

All docker command can be found here:

  . https://docs.docker.com/engine/reference/commandline/cli/



