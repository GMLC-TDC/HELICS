#! /bin/sh
# Require a valid Git ref as an argument
if [ "$#" -ne 1 ] || ! git rev-parse --verify "$1" >/dev/null; then
    echo "Usage: $0 GIT_REF TAG_NAME" >&2
    exit 1
fi

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:test-apps" || rv=1
docker push "helics/helics:test-apps" || rv=1

docker build . -f config/Docker/Dockerfile-HELICS-builder --tag "helics/helics:test-builder" || rv=1
docker push "helics/helics:test-builder" || rv=1

docker build . -f config/Docker/Dockerfile-HELICS --tag "helics/helics:test" || rv=1
docker push "helics/helics:test" || rv=1

exit $rv
