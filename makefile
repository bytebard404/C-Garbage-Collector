main: ObjectManager.o main.o
        clang++ -Wall ObjectManager.o main.o -o main -DNDEBUG

ObjectManager.o: ObjectManager.c
        clang++ -Wall -c ObjectManager.c -o ObjectManager.o -DNDEBUG

main.o: TestSuite.c
        clang++ -Wall -c TestSuite.c -o main.o -DNDEBUG

clean:
        rm -f ObjectManager.o main.o main