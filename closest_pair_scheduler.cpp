#include <libgen.h>
#include <stdlib.h>
#include <limits>
#include <signal.h>
#include <assert.h> 
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm> 
#include <unordered_map> 
#include <mesos/resources.hpp>
#include <mesos/scheduler.hpp>
#include <stout/stringify.hpp>
#include "closest_pair_helper.hpp"
#include <mesos/type_utils.hpp>

using namespace mesos;

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;
using std::numeric_limits;
using std::make_pair;
using std::ifstream;

using mesos::Resources;

const float CPUS_PER_TASK = 0.2;
const int32_t MEM_PER_TASK = 32;

static vector<Point> pointVector; 
static queue<SplitInfo> splitQueue; 
static unordered_map<int, SplitInfo> parentMap; 
static int nextParentId = 0;
static double minDistance = numeric_limits<double>::max();

MesosSchedulerDriver* schedulerDriver;

static string createTaskData(const vector<Point> & pv, const SplitInfo & split);
static void shutdown();
static void SIGINTHandler();

class ClosestPairScheduler : public Scheduler
{
 public:
  ClosestPairScheduler(const ExecutorInfo& _combiner)
    :combiner(_combiner),  
    tasksLaunched(0), 
    tasksFinished(0) {
      SplitInfo start(0, pointVector.size() - 1, nextParentId++, NONE);
      splitQueue.push(start);
    }

  virtual ~ClosestPairScheduler() {}

  virtual void registered(
      SchedulerDriver*,
      const FrameworkID&, 
      const MasterInfo&) {
      cout << "Scheduler Registered!" << endl;
  }

  virtual void reregistered(SchedulerDriver*, const MasterInfo& masterInfo) {}

  virtual void disconnected(SchedulerDriver* driver) {}

  virtual void resourceOffers(SchedulerDriver* driver,
      const vector<Offer>& offers) {
    cout << "getting resource offers" << endl; 
    //for (size_t i = 0; i < offers.size(); i++) {
      //const Offer& offer = offers[i];
    foreach(const Offer& offer, offers){
      cout << "Received offer " << offer.id() << " with " 
        << offer.resources() << endl; 
      
      static const Resources TASK_RESOURCES = Resources::parse(
          "cpus:" + stringify<float>(CPUS_PER_TASK) +
          ";mem:" + stringify<size_t>(MEM_PER_TASK)).get();

      size_t maxTasks = 0;      

      double cpus = 0; 
      double mem = 0; 
      //size_t maxTasks = 0; 

      for (int i = 0; i < offer.resources_size(); i++) {
        const Resource& resource = offer.resources(i);
        if (resource.name() == "cpus" &&
            resource.type() == Value::SCALAR) {
          cpus = resource.scalar().value();
        } else if (resource.name() == "mem" &&
                   resource.type() == Value::SCALAR) {
          mem = resource.scalar().value();
        }
      }
      while (cpus >= CPUS_PER_TASK && mem >= MEM_PER_TASK){
        maxTasks++; 
        cpus -= CPUS_PER_TASK; 
        mem -= MEM_PER_TASK; 
      }

      cout << "MaxTasks " << maxTasks << endl; 
      
      // Launch tasks.
      vector<TaskInfo> tasks;
      if (parentMap.empty()) {
        cout << "no more parent things to run " << endl; 
      }
      for (size_t i = 0; i < maxTasks && parentMap.size() > 0; i++) {
        cout << "parent queue size " << parentMap.size()  << endl; 
        if (parentMap.size() == 1) {
          cout << "single parent " << parentMap.begin()->second.parent << endl;
          cout << "element " << parentMap.begin()->second.lMinDist << " "
          << parentMap.begin()->second.rMinDist << endl; 
        }
        //cout << "reaches here " << endl; 
        auto itr = parentMap.begin();
        while (itr != parentMap.end()) {
          if (itr->second.lMinDist != NOT_SET && itr->second.rMinDist != NOT_SET
            /*&& itr->second.parent == 0*/) {

            string taskData = createTaskData(pointVector, itr->second); 

            string combineId = itr->second.convertToString(); 
            TaskInfo task;

            task.set_name("Combiner " + combineId);
            task.mutable_task_id()->set_value(combineId);
            task.mutable_slave_id()->MergeFrom(offer.slave_id());
            task.mutable_executor()->MergeFrom(combiner);            
            task.mutable_resources()->MergeFrom(TASK_RESOURCES);
            task.set_data(taskData);                
            tasks.push_back(task);
            tasksLaunched++;
            parentMap.erase(itr++); 
            cout << "Combiner " << combineId << endl;
            continue; 
          } else {
            ++itr; 
          }
        }
      }
      //cout << "before launchTasks" << endl;
      driver->launchTasks(offer.id(), tasks); 
    }
  }


  virtual void offerRescinded(SchedulerDriver* driver,
      const OfferID& offerId) {}


  virtual void statusUpdate(SchedulerDriver* driver, const TaskStatus& status) {
    
    cout << "receiving status updates" << endl; 
    if (status.state() == TASK_FINISHED) {
      cout << "Task " << status.task_id().value() << " finished" << endl;

      vector<double> infoVec = stringToVector(status.data());
      double dist = infoVec[0];
      int parent = int(infoVec[1]); 

      Direction dir = static_cast<Direction>(int(infoVec[2])); 

      if(dist < minDistance) {
        minDistance = dist;
        cout << "updating minDist to " << minDistance << endl; 
      }

      if(dir == LEFT) 
          parentMap.at(parent).lMinDist = dist; 
      else if(dir == RIGHT)
          parentMap.at(parent).rMinDist = dist; 

      tasksFinished++;
    } else if (status.state() == TASK_LOST) {
      cout << "Task " << status.task_id().value() << " " << status.message() << endl;  
    }

    if (tasksFinished == tasksLaunched && 
        splitQueue.empty() && 
        parentMap.empty()) {
      // Wait to receive any pending framework messages
      // If some framework messages are lost, it may hang indefinitely.

      shutdown();
      driver->stop();
    } 
  }
 
  virtual void frameworkMessage(SchedulerDriver* driver,
      const ExecutorID& executorId,
      const SlaveID& slaveId,
      const string& data) {}

  virtual void slaveLost(SchedulerDriver* driver, const SlaveID& sid) {}

  virtual void executorLost(SchedulerDriver* driver,
      const ExecutorID& executorID,
      const SlaveID& slaveID,
      int status) {}

  virtual void error(SchedulerDriver* driver, const string& message) {
    cout << message << endl;
  }

 private:
  const ExecutorInfo combiner;
  size_t tasksLaunched;
  size_t tasksFinished;
};


static string createTaskData(const vector<Point> & pv, const SplitInfo & split)
{
  cout << "creating task data " << endl; 
  vector<double> doubleVec; 
  int size = split.right - split.left + 1; 
  //doubleVec.push_back(double(size)); 

  for (int i = 0; i < size; i++)
  {
    auto itr = pv.begin() + split.left + i;
    doubleVec.push_back(itr->x); 
    doubleVec.push_back(itr->y); 
  }

  doubleVec.push_back(double(split.parent)); 
  doubleVec.push_back(split.lMinDist); 
  doubleVec.push_back(split.rMinDist); 
  doubleVec.push_back(double(split.dir)); 

  return vectorToString(doubleVec); 
}

static void shutdown() {
  printf("closestPair is shutting down\n");
}

static void SIGINTHandler(int signum)
{
  if (schedulerDriver != NULL) {
    shutdown();
    schedulerDriver->stop();
  }
  delete schedulerDriver;
  exit(0);
}

int main(int argc, char** argv) {
  string filename, master; 
  if (argc != 3){
    cout << "Usage: " << argv[0] << " <filename> <master>" << endl; 
  } 

  filename = argv[1]; 
  master = argv[2]; 
  cout << "filename: " << filename << endl; 
  cout << "master:  " << master << endl; 
  double x,y; 

  ifstream in; 
  in.open(filename); 
  while(in >> x >> y){ 
    cout << x << y << endl; 
    pointVector.push_back(Point(x, y)); 

  }

  sort(pointVector.begin(), pointVector.end(), 
    [](Point p1, Point p2){ return p1.x < p2.x; });

  string path = realpath(dirname(argv[0]), NULL);
  string combinerURI = path + "/combiner_executor";
  cout << combinerURI << endl;

  ExecutorInfo combiner;
  combiner.mutable_executor_id()->set_value("Combiner");
  combiner.mutable_command()->set_value(combinerURI);
  combiner.set_name("Combine Executor (C++)");
  combiner.set_source("cpp");

  ClosestPairScheduler scheduler(combiner);

  FrameworkInfo framework;
  framework.set_user(""); // Have Mesos fill in the current user.
  framework.set_name("Closest Pair Scheduler (C++)");
  //framework.set_role(role);
  framework.set_principal("closest-pair-scheduler-cpp");


  while(!splitQueue.empty()) {
    SplitInfo splitVec = splitQueue.front();
    splitQueue.pop();

    if (splitVec.right - splitVec.left < 2) {
      if (splitVec.right == splitVec.left) {
        splitVec.lMinDist = splitVec.rMinDist = 
        numeric_limits<double>::max(); 
        cout << "gets to base case in splitting" << endl; 
      } else if (splitVec.right - splitVec.left == 1) {
        splitVec.lMinDist = splitVec.rMinDist = 
        calcDist(pointVector[splitVec.left], pointVector[splitVec.right]); 
        cout << "gets to base case in splitting" << endl; 
      } else {
        cerr << "error in splitting indices" << endl;
        return 0; 
      }
    } else {
      int mid = (splitVec.right - splitVec.left) / 2; 
      splitQueue.push(SplitInfo(
          splitVec.left, 
          splitVec.left + mid, 
          nextParentId, 
          LEFT)); 
        splitQueue.push(SplitInfo(
          splitVec.left + mid + 1 , 
          splitVec.right, 
          nextParentId, 
          RIGHT));
      }
      parentMap.insert(make_pair(nextParentId++, splitVec));
  }

  cout << "finished splitting " << endl; 

  // Set up the signal handler for SIGINT for clean shutdown.
  struct sigaction action;
  action.sa_handler = SIGINTHandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  cout << "creating Driver" << endl;

  schedulerDriver = new MesosSchedulerDriver(&scheduler, framework, master);

  int status = schedulerDriver->run() == DRIVER_STOPPED ? 0 : 1;
  cout << "Final Minimum Distance between two points: " << minDistance << endl;  


  // Ensure that the driver process terminates.
  schedulerDriver->stop();

  shutdown();
  delete schedulerDriver;
  return status;
}
