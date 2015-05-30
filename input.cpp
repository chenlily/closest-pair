#include <iostream> 
#include <vector>
#include <fstream>
#include <cmath>  
#include <queue>
#include <limits> 
#include <map> 
#include <utility>
#include <algorithm> 
#include <cassert>

using namespace std; 

const int NOT_SET = -1; 

enum Direction{
	LEFT, 
	RIGHT, 
	NONE
}; 

struct Point{
	double x,y;

	Point(double _x, double _y) 
		: x(_x), y(_y) {}
};

struct SplitInfo{
	int left, right, parent; 
	double lMinDist = NOT_SET, rMinDist = NOT_SET;
	//enum Direction = {LEFT, RIGHT, NONE};
	Direction dir;

	void printSplitInfo(){
			cout << "left = " << left << endl; 
			cout << "right = " << right << endl; 
			cout << "parent = " << parent << endl; 
			cout << "dir = " << dir << endl; 
			cout << "----" << endl << endl; 
	}
	/**
	void setSplitInfo(int _left, int _right, int _parent, Direction _dir){
		left = _left; 
		right = _right; 
		parent = _parent; 
	} **/

	SplitInfo(int _left, int _right, int _parent, Direction _dir)
		:left(_left), right(_right), parent(_parent), 
		lMinDist(double(NOT_SET)), rMinDist(double(NOT_SET)), 
		dir(_dir) { 
			//printSplitInfo();
		}  

	//copy constructor 
	/**SplitInfo(const SplitInfo& other){
		left = other.left; 
		right = other.right; 
		parent = other.parent; 
		lMinDist = other.lMinDist; 
		rMinDist = other.rMinDist; 
	}
	//override equals sign
	SplitInfo& operator=(const SplitInfo& other){
		left = other.left; 
		right = other.right; 
		parent = other.parent; 
		lMinDist = other.lMinDist; 
		rMinDist = other.rMinDist; 
		return *this; 
	} **/
};

static vector<Point> pointVector; 
static queue<SplitInfo> splitQueue; 
static map<int, SplitInfo> parentMap; 
static int nextParentId = 0; 

double calcDist(Point p1, Point p2){
	return sqrt( ((p1.x - p2.x) * (p1.x - p2.x)) +
				((p1.y - p2.y) * (p1.y - p2.y)) ); 
}


//assumes points in pointVector are sorted by x coordinate 
void splitter(){
	while(!splitQueue.empty()){
		SplitInfo splitVec = splitQueue.front();
		splitQueue.pop();

		//base case 
		if (splitVec.right - splitVec.left < 2){
			if (splitVec.right == splitVec.left){
				splitVec.lMinDist = splitVec.rMinDist = numeric_limits<double>::max(); 
			} else if (splitVec.right - splitVec.left == 1){
				splitVec.lMinDist = splitVec.rMinDist = 
					calcDist(pointVector[splitVec.left], pointVector[splitVec.right]); 
			} else {
				cerr << "error in splitting indices, right lower than left" << endl;
				return;
			}
		} else {
			//split to two vectors and push back into the queue 
			int mid = (splitVec.right - splitVec.left) / 2; 
			splitQueue.push(SplitInfo(splitVec.left, splitVec.left + mid, nextParentId, LEFT)); 
			splitQueue.push(SplitInfo(splitVec.left + mid + 1 , splitVec.right, nextParentId, RIGHT));
		}
		//print info at nextParent Id 
		//cout << "parent info: " << nextParentId << endl; 
		//splitVec.printSplitInfo(); 
		parentMap.insert(make_pair(nextParentId++, splitVec));
		//parentMap[nextParentId] = splitVec;
	}
}

struct less_than_X
{
	inline bool operator() (const Point& p1, const Point& p2){
		return p1.x < p2.x;
	}
};

struct less_than_Y
{
	inline bool operator() (const Point& p1, const Point& p2){
		return p1.y < p2.y; 
	}
};

double minDist(SplitInfo splitVec){
	double upperBound = min(splitVec.lMinDist, splitVec.rMinDist); 

	vector<Point> candidates; 
	int mid = (splitVec.left + splitVec.right) / 2;
	for(int i = splitVec.left; i < splitVec.right + 1; i++){
		if(abs(pointVector[i].x - pointVector[mid].x) < upperBound){
			candidates.push_back(pointVector[i]);
		}
	}

	sort(candidates.begin(), candidates.end(), less_than_Y()); 
	double candMin = upperBound; 
	for (int i = 0; i < candidates.size(); i++){
		for (int j = i + 1; j < candidates.size() && candidates[j].y - candidates[i].y < candMin; j++){
			if (calcDist(candidates[i], candidates[j]) < candMin)
				candMin = calcDist(candidates[i], candidates[j]);
		}
	}
	return min(upperBound, candMin); 
}

double combiner(){
	while (parentMap.size() > 1){
		//if left and right are not filled out, push to end
		auto itr = parentMap.begin();
		while(itr != parentMap.end()){
			if(itr->second.lMinDist != NOT_SET && itr->second.rMinDist != NOT_SET
				&& itr->second.parent != 0){
				//calculate minDist
				double min = minDist(itr->second); 

				//update parent
				int parentNum = itr->second.parent; 
				if(itr->second.dir == LEFT){
					parentMap.at(parentNum).lMinDist = min; 
				} else if (itr->second.dir == RIGHT){
					parentMap.at(parentNum).rMinDist = min; 
				}
				
				parentMap.erase(itr++);
			} else {
				++itr; 
			}
		} 
	}

	//last node should be the parent
	auto itr = parentMap.begin(); 
	assert(itr->second.lMinDist != NOT_SET && itr->second.rMinDist != NOT_SET);

	return minDist(itr->second); 
} 


int main(int argc, char** argv)
{
	//get vector points from command line 
	string filename; 
	while(argc != 2){
		cout << "give only a single filename as input" << endl; 
	}
	filename = argv[1]; 

	double x,y; 

	ifstream in; 
	in.open(filename); 
	while(in >> x >> y){ 
		cout << x << y << endl; 
		pointVector.push_back(Point(x, y)); 

	}

	//debugging, viewing output 
	
	for(int i = 0; i != pointVector.size(); i++){
		cout << pointVector[i].x << " " << pointVector[i].y << endl; 
	} 

	//remember to sort nodes by x coordinate 
	sort(pointVector.begin(), pointVector.end(), less_than_X());

	//put first node into split queue
	SplitInfo start(0, pointVector.size() - 1, nextParentId++, NONE);
	splitQueue.push(start);

	splitter();

	//print parent map
	
	for(auto it = parentMap.begin(); it != parentMap.end(); it++){
		cout << "parent id #: " << it->first << endl; 
		it->second.printSplitInfo();
		cout << endl;
	}
	

	//TODO: test combiner 
	double minDistance = combiner(); 
	cout << minDistance << endl; 

	return 0; 
}