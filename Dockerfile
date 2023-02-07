
FROM alpine:latest AS builder
WORKDIR /app
COPY . .
RUN apk --no-cache add gcc musl-dev libc-dev linux-headers
RUN gcc server.c server-utils.c errors.c player.c global.c game.c stopwatch.c file-access.c time-utils.c -lm -o server.o -pthread -lrt
RUN gcc client.c errors.c -o client.o

FROM alpine:latest AS runtime
WORKDIR /app
COPY --from=builder /app/*.o /app
CMD while true; do sleep 1; done
