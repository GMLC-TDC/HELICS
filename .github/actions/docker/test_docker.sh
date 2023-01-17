#! /bin/sh

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:test-apps" || rv=1
docker push "helics/helics:test-apps" || rv=1

docker build . -f config/Docker/Dockerfile-HELICS-builder --tag "helics/helics:test-builder" || rv=1
docker push "helics/helics:test-builder" || rv=1

docker build . -f config/Docker/Dockerfile-HELICS --tag "helics/helics:test" || rv=1
docker push "helics/helics:test" || rv=1

exit "${rv= 0}"
