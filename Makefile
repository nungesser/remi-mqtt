BIN=remi-mqtt
PAHO_LIB=-lpaho-mqtt3c

# object file
OBJS=$(BIN).o paho.o

all: $(BIN)

$(BIN):	$(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(PAHO_LIB)

clean:
	rm -rf $(OBJS) $(BIN)

DOCKER_IMG_NAME=debian-mqtt

docker-build:
	docker rmi -f $(DOCKER_IMG_NAME) && docker build -t $(DOCKER_IMG_NAME) .
