FROM gcc:12.3.0-bookworm

# Environment variables are handled in the docker-compose file. Otherwise, uncomment these:
#ENV BROKER_URI=tcp://192.168.6.59:1883 \
#    DEVICE_ID=IRsensor \
#    DEVICE_KEY=password

RUN apt update && apt install -y libpaho-mqtt-dev && rm -rf /var/lib/apt/lists/*
RUN mkdir /opt/remi-mqtt
COPY ./remi-mqtt.c /opt/remi-mqtt/
COPY ./Makefile    /opt/remi-mqtt/

WORKDIR /opt/remi-mqtt/
RUN make

CMD ["./remi-mqtt"]
