# closest-pair

A framework for [Apache Mesos](http://mesos.apache.org/) that given a file with a set of points, calculates the closest distance between any two points in that set. 

Based on [Rendler](https://github.com/mesosphere/RENDLER) implementation. 

Closest-Pair consists of two main components:
* 'ClosestPairExecutor' extends 'mesos.Executor'
* 'ClosestPairScheduler' extends 'mesos.Scheduler' 

## Quick Start with Vagrant
=========

### Requirements

- [VirtualBox](http://www.virtualbox.org/) 4.1.18+
- [Vagrant](http://www.vagrantup.com/) 1.3+
- [git](http://git-scm.com/downloads) (command line tool)

### Start the `mesos-demo` VM

```bash
$ wget http://downloads.mesosphere.io/demo/mesos.box -O /tmp/mesos.box
$ vagrant box add --name mesos-demo /tmp/mesos.box
$ git clone https://github.com/mesosphere/RENDLER.git
$ cd RENDLER
$ vagrant up
```

### Execution:

```bash
$ vagrant ssh
vagrant@mesos:~ $ cd hostfiles/cpp

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