
SOURCE_FILES := event_iterator.cpp loader.cpp
CFLAGS :=

mutator:
	$(CXX) $(CFLAGS) -o mutator mutator.cpp $(SOURCE_FILES)
test:
	$(CXX) $(CFLAGS) -o test test.cpp $(SOURCE_FILES)
all: mutator test
clean:
	rm -rf test mutator
