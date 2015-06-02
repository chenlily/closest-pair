#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm> 
#include <assert.h>
#include <limits>
#include <stout/stringify.hpp>
#include <stout/numify.hpp>
#include <stout/lambda.hpp>
#include <mesos/executor.hpp>
#include "closest_pair_helper.hpp"

using namespace mesos;

using std::cout;
using std::endl;
using std::string;
using std::vector; 
using std::min; 
using std::queue;
using std::numeric_limits; 

double minDist(
    double lMinDist, 
    double rMinDist, 
    const vector<Point> & pointVec) { 
  
  cout << "lMinDist " << lMinDist << endl; 
  cout << "rMinDist" << rMinDist << endl; 
  cout << "pointVector: "; 
  for(int i = 0; i != pointVec.size(); i++) {
    cout << pointVec[i].x << " " << pointVec[i].y <<  ";"; 
  } 

  if (pointVec.size() == 1)
    return numeric_limits<double>::max(); 
  else if (pointVec.size() == 2)
    return calcDist(pointVec[0], pointVec[1]); 

  double upperBound = min(lMinDist, rMinDist); 

  vector<Point> candidates; 
  int mid = pointVec.size() / 2;
  for (int i = 0; i < pointVec.size(); i++) {
    if (abs(pointVec[i].x - pointVec[mid].x) < upperBound)
        candidates.push_back(pointVec[i]);
  }

  sort(candidates.begin(), candidates.end(), 
    [](Point p1, Point p2) { return p1.y < p2.y; });

  double candMin = upperBound; 
  for (int i = 0; i < candidates.size(); i++) {
    for (int j = i + 1; j < candidates.size() 
      && candidates[j].y - candidates[i].y < candMin; j++) {
      if (calcDist(candidates[i], candidates[j]) < candMin)
        candMin = calcDist(candidates[i], candidates[j]);
    }
  }

  cout << "returns from minDist" << endl; 

  return min(upperBound, candMin); 
}

string minDistMessage(double minDist, int parent, Direction dir)
{
  vector<double> doubleVec; 
  doubleVec.push_back(minDist); 
  doubleVec.push_back(double(parent));
  doubleVec.push_back(double(dir)); 

  return vectorToString(doubleVec); 
}

static void runTask(ExecutorDriver* driver, const TaskInfo& task)
{
  cout << "Running task " << task.task_id().value() << endl;
  vector<double> dataVec = stringToVector(task.data()); 
  
  vector<Point> pv; 
  int pvSize = (dataVec.size() - 4) / 2; 
  //rebuild point data from task.data 
  for (int i = 0; i < pvSize; i++) {
    Point p(dataVec[2 * i], dataVec[(2 * i) + 1]); 
    pv.push_back(p); 
  }

  int parent = int(dataVec[dataVec.size() - 4]);
  double lMinDist = int(dataVec[dataVec.size() - 3]);
  double rMinDist = int(dataVec[dataVec.size() - 2]);
  Direction dir = static_cast<Direction>(int(dataVec[dataVec.size() - 1])); 


  //calculate min distance   
  cout << "run task info --- " << endl; 
  cout << "Parent " << parent << endl; 
  double min = minDist(lMinDist, rMinDist, pv); 
  cout << "returns from minDist with min: " << min << endl; 
  cout << "enters sendFrameworkMessage" << endl; 
  //build message to send back with task status 
  string msg = minDistMessage(min, parent, dir);
  


  //cout << "returns from minDistMessage";
  //driver->sendFrameworkMessage(msg); 

  //cout << "returns from sendFrameworkMessage " << endl; 
  cout << "message data sending back " << msg << endl; 
  TaskStatus status;
  status.mutable_task_id()->MergeFrom(task.task_id());
  status.set_state(TASK_FINISHED);
  status.set_data(msg); 
  driver->sendStatusUpdate(status);
  cout << "sent status update " << endl; 
}

void* start(void* arg) {
  lambda::function<void(void)>* thunk = (lambda::function<void(void)>*) arg;
  (*thunk)();
  delete thunk;
  return NULL;
}

class ClosestPairExecutor : public Executor
{
 public: 
  virtual ~ClosestPairExecutor() {}
  virtual void registered (ExecutorDriver* driver,
      const ExecutorInfo& executorInfo,
      const FrameworkInfo& frameworkInfo,
      const SlaveInfo& slaveInfo) {
    cout << "Registered Closest Pair Executor on "; 
    cout << slaveInfo.hostname() << endl;
  }

  virtual void reregistered(ExecutorDriver* driver,
      const SlaveInfo& slaveInfo) {
    cout << "Re-registered Executor on "; 
    cout << slaveInfo.hostname() << endl;
  }

  virtual void disconnected(ExecutorDriver* driver) {}


  virtual void launchTask(ExecutorDriver* driver, const TaskInfo& task) {
    cout << "Starting task " << task.task_id().value() << endl;

    /*lambda::function<void(void)>* thunk =
      new lambda::function<void(void)>(lambda::bind(&runTask, driver, task));

    pthread_t pthread;
    if (pthread_create(&pthread, NULL, &start, thunk) != 0) {
      TaskStatus status;
      status.mutable_task_id()->MergeFrom(task.task_id());
      status.set_state(TASK_FAILED);

      driver->sendStatusUpdate(status);
    } else {
      pthread_detach(pthread);

      TaskStatus status;
      status.mutable_task_id()->MergeFrom(task.task_id());
      status.set_state(TASK_RUNNING);

      driver->sendStatusUpdate(status);
    }*/

    runTask(driver, task); 
  }

  virtual void killTask(ExecutorDriver* driver, const TaskID& taskId) {}
  virtual void frameworkMessage(ExecutorDriver* driver, const string& data) {}
  virtual void shutdown(ExecutorDriver* driver) {}
  virtual void error(ExecutorDriver* driver, const string& message) {}  
};

int main(int argc, char** argv)
{
  ClosestPairExecutor executor;
  MesosExecutorDriver driver(&executor);
  return driver.run() == DRIVER_STOPPED ? 0 : 1;
}