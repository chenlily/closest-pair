CXX = g++
CXXFLAGS = -g -O2 -pthread -std=c++11
LDFLAGS += -lmesos -lpthread -lprotobuf
CXXCOMPILE = $(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES)
CXXLINK = $(CXX) $(CXXFLAGS) -o $@ $(INCLUDES)


default: all
all: closest_pair_scheduler combiner_executor

HEADERS = closest_pair_helper.hpp

combiner_executor: combiner_executor.cpp $(HEADERS)
	$(CXXLINK) $< $(LDFLAGS)


%: %.cpp $(HEADERS)
	$(CXXLINK) $< $(LDFLAGS) -lboost_regex -lcurl


clean:
	(rm -f core combiner_executor closest_pair_scheduler)

#serial execution testing algorithm
sample: input.cpp
	$(CXX) input.cpp -g -std=c++11