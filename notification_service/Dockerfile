FROM golang:1.18

WORKDIR /notification_service

# pre-copy/cache go.mod for pre-downloading dependencies and only redownloading them in subsequent builds if they change
COPY ./src/go.mod ./
COPY ./src/go.sum ./
RUN go mod download && go mod verify

COPY ./src/ ./
RUN go build -o /notification_server


EXPOSE 50051


CMD ["/notification_server"]
