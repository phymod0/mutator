
SOURCE_FILES := event_iterator.cpp
CFLAGS :=

test:
	$(CXX) $(CFLAGS) -o test test.cpp $(SOURCE_FILES)
clean:
	rm -rf test
