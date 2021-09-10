#!/bin/bash
make
for n in 5 10 20 50 100
do
    for z in 0.0 0.5 1.0
    do 
        # ./a.out ${n} ${z} 0.0001 0.001 50 570436
        time ./a.out ${n} ${z} 0.0001 0.001 50 570436
        time ./a.out ${n} ${z} 0.1 1 50 570436
        time ./a.out ${n} ${z} 0.5 10 50 570436
        time ./a.out ${n} ${z} 2 10 50 570436
        time ./a.out ${n} ${z} 5 20 50 570436
        time ./a.out ${n} ${z} 10 50 50 570436
        time ./a.out ${n} ${z} 15 100 50 570436
        time ./a.out ${n} ${z} 20 600 50 570436
        time ./a.out ${n} ${z} 6 600 50 570436
        time ./a.out ${n} ${z} 0.0001 0.001 100 570436
        time ./a.out ${n} ${z} 0.1 1 100 570436
        time ./a.out ${n} ${z} 0.5 10 100 570436
        time ./a.out ${n} ${z} 2 10 100 570436
        time ./a.out ${n} ${z} 5 20 100 570436
        time ./a.out ${n} ${z} 10 50 100 570436
        time ./a.out ${n} ${z} 15 100 100 570436
        time ./a.out ${n} ${z} 20 600 100 570436
    done
done