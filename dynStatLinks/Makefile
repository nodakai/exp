EXES = test test1

all : $(EXES)

.PHONY : clean

CPPFLAGS =
CXXFLAGS = -Wall -Wextra -g3 -O2
LDFLAGS = -L.

test : test.cpp libtestDll.so
	$(CXX) $< -o $@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -ltestDll

test1 : test.cpp libtestDll.a
	$(CXX) $< -o $@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -Wl,-Bstatic -ltestDll -Wl,-Bdynamic

testDll.o : testDll.cpp testDll.h
	$(CXX) $< -o $@ -c -fPIC $(CPPFLAGS) $(CXXFLAGS)

libtestDll.so : testDll.o
	$(CXX) $< -o $@ -shared $(LDFLAGS)

libtestDll.a : testDll.o
	$(AR) rcs $@ $^


clean :
	rm -f $(EXES) *.o *.a *.so
