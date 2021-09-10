CC = g++

prog: Block.o Event.o Link.o Peer.o Simulator.o Transaction.o decl.h
	$(CC) -O2 prog.cpp *.o

Block.o: Block.cpp decl.h 
	$(CC) -O2 -c Block.cpp 
Event.o: Event.cpp decl.h
	$(CC) -O2 -c Event.cpp 
Link.o: Link.cpp decl.h
	$(CC) -O2 -c Link.cpp 
Peer.o: Peer.cpp decl.h
	$(CC) -O2 -c Peer.cpp 
Simulator.o: Simulator.cpp decl.h
	$(CC) -O2 -c Simulator.cpp 
Transaction.o: Transaction.cpp decl.h
	$(CC) -O2 -c Transaction.cpp

clean:
	rm *.o 