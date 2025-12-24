# k-way_Min-cut_Partitioning

## How to Compile
In `HW2_k-way_Min-cut_Partitioning/src/`, enter the following command:
```
$ make
```

An executable file `hw2` will be generated in `HW2_k-way_Min-cut_Partitioning/bin/`.

If you want to remove it, please enter the following command:
```
$ make clean
```


## How to Run
In `HW2_k-way_Min-cut_Partitioning/bin/`, enter the following command:
```
$ ./hw2 <input file> <output file> <number of partitions>
```
E.g., in `HW2_k-way_Min-cut_Partitioning/bin/`, enter the following command:
```
$ ./hw2 ../testcase/public1.txt ../output/public1.2way.out 2
```

## How to Test(Public Testcases)
In `HW2_k-way_Min-cut_Partitioning/src/`, enter the following command:
```
$ make test_public1
$ make test_public2
```
If you want test all testcases:
```
$ make test
```
