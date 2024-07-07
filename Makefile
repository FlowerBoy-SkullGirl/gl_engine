INCLUDES := -lGL -lglfw -lGLEW -lm -lXrandr -lXi -lXxf86vm -g 

BINARY_PATH := ./bin
SOURCE_PATH := ./src
HEADER_PATH := ./src/headers

square:
	g++ -o $(BINARY_PATH)/square $(SOURCE_PATH)/*.cpp $(INCLUDES)
