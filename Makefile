INCLUDES := -lGL -lglfw -lGLEW -lm -lXrandr -lXi -lXxf86vm -g  

BINARY_PATH := ./bin
SOURCE_PATH := ./src
HEADER_PATH := ./src/headers
GLAD_PATH := ./glad/src/gl.c
GLAD_INCLUDE := ./glad/include

square:
	g++ -o $(BINARY_PATH)/square $(SOURCE_PATH)/*.cpp  $(GLAD_PATH) $(INCLUDES) -I $(GLAD_INCLUDE)
