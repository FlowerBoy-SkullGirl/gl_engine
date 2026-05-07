INCLUDES := -lGL -lglfw -lGLEW -lm -lXrandr -lXi -lXxf86vm -g  

BINARY_PATH := ./bin
SOURCE_PATH := ./src
HEADER_PATH := ./src/headers
GLAD_PATH := ./glad/src/gl.c
GLAD_INCLUDE := ./glad/include
TEST_PATH := ./test
# Filter out main.cpp from source files for tests
TEST_SOURCE := $(filter-out $(SOURCE_PATH)/main.cpp, $(wildcard $(SOURCE_PATH)/*.cpp))
TEST_INCLUDE := -lmcheck

square:
	g++ -o $(BINARY_PATH)/square $(SOURCE_PATH)/*.cpp  $(GLAD_PATH) $(INCLUDES) -I $(GLAD_INCLUDE)

test_suite:
	g++ -o $(TEST_PATH)/full_suite_test $(TEST_SOURCE) $(TEST_PATH)/*.cpp $(GLAD_PATH) $(INCLUDES) $(TEST_INCLUDE) -I $(GLAD_INCLUDE)

# Valgrind logs will trace the memory leak to the function where the memory was allocated, so grep'ing the name of the function 
# should allow us to tell if we are at fault for any memory violations in the logs
# If/else statement prints results instead of returning the fail/succeed return status from grep
run_tests:
	valgrind --tool=memcheck --leak-check=yes --log-file=$(TEST_PATH)/test-logs $(TEST_PATH)/full_suite_test ; if grep -e "mesh" -e "shape" -e "init_buffer" $(TEST_PATH)/test-logs; \
		then echo "Found memory leak from gl_engine"; else echo "Logs do not contain memory leak from gl_engine"; fi
