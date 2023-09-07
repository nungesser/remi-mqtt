BIN=remi-mqtt
PAHO_LIB=-lpaho-mqtt3c

# object file
OBJ=$(BIN).o

all: $(BIN)

$(BIN):	$(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(PAHO_LIB)

clean:
	rm -rf $(OBJ) $(BIN)

DOCKER_IMG_NAME=debian-mqtt

docker-build:
	docker rmi -f $(DOCKER_IMG_NAME) && docker build -t $(DOCKER_IMG_NAME) .
