FROM alpine:latest

RUN apk add --no-cache \
    bash \
    jq \
    git \
    ca-certificates \
    curl

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
