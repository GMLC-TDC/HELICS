#! /bin/sh

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:latest" || rv=1
docker push "helics/helics:latest" || rv=1

docker build . -f config/Docker/Dockerfile-HELICS-builder --tag "helics/helics:builder" || rv=1
docker push "helics/helics:builder" || rv=1

exit "${rv= 0}"
