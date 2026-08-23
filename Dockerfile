FROM alpine:latest

WORKDIR /app

RUN apk add --no-cache build-base cmake

COPY . .

RUN cmake -B build -S . && cmake --build build
