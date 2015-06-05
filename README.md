# closest-pair

A framework for [Apache Mesos](http://mesos.apache.org/) that given a file with a list of 2D points, calculates the closest distance between any two points in that set. Implements a divide-and-conquer algorithm which determines the result in 
O(nlogn) time instead of brute-force O(n^2). 

Based on [Rendler](https://github.com/mesosphere/RENDLER) implementation. 

Closest-Pair consists of two main components:
* `ClosestPairExecutor` extends `mesos.Executor`
* `ClosestPairScheduler` extends `mesos.Scheduler` 

## Quick Start with Vagrant

### Requirements

- [VirtualBox](http://www.virtualbox.org/) 4.1.18+
- [Vagrant](http://www.vagrantup.com/) 1.3+
- [git](http://git-scm.com/downloads) (command line tool)

### Start the `mesos-demo` VM

```bash
$ wget http://downloads.mesosphere.io/demo/mesos.box -O /tmp/mesos.box
$ vagrant box add --name mesos-demo /tmp/mesos.box
$ git clone https://github.com/chenlily/closest-pair
$ vagrant up
```

### Execution:

```bash
$ vagrant ssh
vagrant@mesos:~ $ cd hostfiles

# Update install dependencies
vagrant@mesos:cpp $ sudo apt-get update
vagrant@mesos:cpp $ sudo apt-get install libcurl4-openssl-dev libboost-regex1.55-dev \
          libprotobuf-dev libgoogle-glog-dev protobuf-compiler

# Build
vagrant@mesos:cpp $ make all

# Start the scheduler with the test file and the mesos master ip
vagrant@mesos:cpp $ ./closest-pair-scheduler testfile 127.0.1.1:5050
# <Ctrl+C> to stop...
```

### Shutting down the `mesos-demo` VM

```bash
# Exit out of the VM
vagrant@mesos:hostfiles $ exit
# Stop the VM
$ vagrant halt
# To delete all traces of the vagrant machine
$ vagrant destroy
```

### Test File Format 
Points can be either integers or floating point
```
x1 y1
x2 y2
x3 y3 
```

## Closest-Pair Architecture

### Rendler Scheduler

### Data Structures

A point is represented by a struct x and y coordinates. 

```c++
struct Point(
  double x,y; 
)
```

A split on an array into its left and right components is represented by a SplitInfo. It contains the left and right indices of its subarray, a parent ID, the minimum distances on its left and right halves, and also whether the split is a left or right child to its parent. If the split is the original/root array, the direction is set as NONE. 

```c++
struct SplitInfo (
  int left, right parent; 
  double lMinDist, rMinDist; 

  // enum Direction {LEFT, RIGHT, NONE}; 
  Direction dir;   
)
```
* `pointvector` vector of Points containing the original array of Points
* `splitQueue` list of vectors to be split in half 
* `parentMap` maps parentIDs to their splitInfo, also acts as a list of splitInfo whose minDistances still need to be calculated

#### Scheduler Behavior

The scheduler accepts a text file as command-line parameter to create a vector of points to calculate the minimum distance. 

* Reads in the file and initializes a point vector
* Split each vector into two halves until it is 2 or fewer elements. On each split, create a SplitInfo object representing the split and insert it into the parentMap 
* Until parentMap is empty, launch a combiner task to calculate the minDistance on splitInfo objects that minimum distances calculated on the left and right sides. Update splitInfo's parent through parentMap. 
* Print the final minDistance through stdout

### Combiner Executor

- Interprets incoming tasks' `task.data` field as an array of points, the subarray's parentID, minDistance on its left and right halves, and also whether the subarray is a left or right child to its parent
- Calculates the minDistance in that subarray of points
- Returns the subarray's minDistance, parentID, and relation to its parent to the scheduler in the data field of a TaskStatus 



