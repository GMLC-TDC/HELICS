#! /bin/sh
# Require a valid Git ref as an argument
if [ "$#" -ne 1 ] || ! git rev-parse --verify "$1" >/dev/null; then
    echo "Usage: $0 GIT_REF" >&2
    exit 1
fi

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:develop" || rv=1
docker push "helics/helics:develop" || rv=1

exit $rv
