#ifndef __CLOSEST_PAIR_HELPER_HPP__
#define __CLOSEST_PAIR_HELPER_HPP__   

#include <iostream>
#include <string>
#include <stout/strings.hpp>

using std::endl; 
using std::cout; 
using std::string;
using std::vector;
using std::queue;

namespace mesos {
  string vectorToString(vector<double>& vec) {
    string result;
    result.append(stringify<size_t>(vec.size()));
    result.push_back('\0');
    for (size_t i = 0; i < vec.size(); i++) {
      result.append(std::to_string(vec[i]));
      result.push_back('\0');
    }
    return result;
  }

  vector<double> stringToVector(const string& str) {
    const char *data = str.c_str();
    string lenStr = data;
    size_t len = numify<size_t>(data).get();
    data += lenStr.length() + 1;

    vector<double> result;
    for (size_t i = 0; i < len; i ++) {
      string s = data;
      data += s.length() + 1;
      result.push_back(stod(s));
    }
    return result;
  }

};

const int32_t NOT_SET = -1; 

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


double calcDist(Point p1, Point p2){
	return sqrt( ((p1.x - p2.x) * (p1.x - p2.x)) +
				((p1.y - p2.y) * (p1.y - p2.y)) ); 
}


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

	SplitInfo(int _left, int _right, int _parent, Direction _dir)
		:left(_left), right(_right), parent(_parent), 
		lMinDist(double(NOT_SET)), rMinDist(double(NOT_SET)), 
		dir(_dir) { 
			//printSplitInfo();
		}  

	//may have to fix this later
	string convertToString()
	{
		vector<double> vec; 
		vec.push_back((double)left); 
		vec.push_back((double)right); 
		vec.push_back((double)parent);
		vec.push_back((double)lMinDist);
		vec.push_back((double)rMinDist);
		vec.push_back((double)dir); 
		 
		return stringify(vec); 
	}
};


#endif // __CLOSEST_PAIR_HELPER_HPP__