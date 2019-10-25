FROM alpine:latest

RUN apk add --no-cache \
    bash \
    jq \
    ca-certificates \
    curl

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
