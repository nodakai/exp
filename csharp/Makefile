progs = Test01.exe

.PHONY : all clean

all : $(progs)

Test01.exe : Test01.cs
	dmcs $< -out:$@

clean : rm *~
