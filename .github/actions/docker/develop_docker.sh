#! /bin/sh

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:develop" || rv=1
docker push "helics/helics:develop" || rv=1

exit "${rv= 0}"
