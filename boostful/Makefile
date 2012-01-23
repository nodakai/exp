all : boostful.txt

.PHONY : all clean

BOOST_ROOT= /opt/boost/1.48.0
BOOST_INCLUDE= $(BOOST_ROOT)/include
BOOST_LIB= $(BOOST_ROOT)/lib
CPPFLAGS= -isystem $(BOOST_INCLUDE)
CXXFLAGS= -Wall -Wextra
LDFLAGS= -L $(BOOST_LIB) -lboost_program_options

% : %.cpp
	$(CXX) $< -o $@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)

boostful.txt : %.txt : %
	LD_LIBRARY_PATH=$(BOOST_LIB):$$LD_LIBRARY_PATH ./$< > $@

clean :
	rm -f boostful.txt boostful
