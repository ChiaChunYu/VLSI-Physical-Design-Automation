# Detailed Placement

## How to Compile 
In `src/`, enter the following command:
```
$ make
```
An executable file "hw3" will be generated in "bin/".

If you want to remove it, please enter the following command:
```
$ make clean
```

## How to Run
In `bin/`, enter the following command:
```
$ ./hw3 <input LEF> <input DEF> <output DEF>
```
E.g., in "HW3/bin/", enter the following command:
```
$ ./hw3 ../testcase/public1.lef ../testcase/public1.def ../output/public1.def
```

## How to Test(Public Testcases)
In `src/`, enter the following command:
```
$ make test_public1
$ make test_public2
$ make test_public3
$ make test_public4
```
If you want test all testcases:
```
$ make test
```