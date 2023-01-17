#! /bin/sh
# Require a tag name for the docker build image
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 TAG_NAME" >&2
    exit 1
fi

docker build . -f config/Docker/Dockerfile-HELICS-apps --tag "helics/helics:$1" || rv=1
docker push "helics/helics:$1" || rv=1

exit "${rv= 0}"
