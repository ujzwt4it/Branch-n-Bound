#include <cstdio>
#include <stack>
#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <set>
using namespace std;

const int N = 20;
const int inf = 100000000;

struct Assignment_Solution {
	void *pts;
	int cost, idx;
	int mapping[N];
	void printSol() {
		printf("Minimum cost of Assignment is %d achieved by mapping : \n", cost);
		for(int i = 0; i < idx; ++i) {
			printf("%d does job %d\n", i + 1, mapping[i] + 1);
		}
	}
	bool isFeasible();
	int getBound();
};

struct Assignment_Problem {
	int n; 
	int cost[N][N];
	Assignment_Solution emptySolution() {
		Assignment_Solution s1;
		s1.idx = 0;
		s1.pts = this;
		s1.cost = 0;
		for(int i = 0; i < N; ++i) {
			s1.mapping[i]=-1;
		}
		return s1;
	}
	Assignment_Solution worstSolution() {
		Assignment_Solution s1;
		s1.idx = 0;
		s1.pts = this;
		s1.cost = inf;
		for(int i = 0; i < N; ++i) {
			s1.mapping[i] = -1;
		}
		return s1;
	}
	vector<Assignment_Solution> expand(Assignment_Solution s);
};

bool Assignment_Solution::isFeasible() {
	return (idx == (((Assignment_Problem *)pts)->n));
}

int Assignment_Solution::getBound() {
	int bound = cost;
	set<int> done;
	for(int i = 0; i < idx; ++i) {
		done.insert(mapping[i]);
	}
	for(int i = idx; i < ((Assignment_Problem *)pts)->n; ++i) {
		int mn = inf;
		for(int j = 0; j < ((Assignment_Problem *)pts)->n; ++j) {
			if(done.find(j) == done.end()) {
				mn = min(mn, ((Assignment_Problem *)pts)->cost[i][j]);
			}
		}
		bound += mn;
	}
	return bound;
}

vector<Assignment_Solution> Assignment_Problem::expand(Assignment_Solution s) {
	Assignment_Solution s1 = s;
	set<int> done;
	for(int i = 0; i < s.idx; ++i) {
		done.insert(s.mapping[i]);
	}
	s1.idx++;
	vector<Assignment_Solution> ret;
	for(int i = 0; i < n; ++i) {
		if(done.find(i) == done.end()) {
			s1.cost += cost[s1.idx-1][i];
			s1.mapping[s1.idx-1] = i;
			ret.push_back(s1);
			s1.cost -= cost[s1.idx-1][i];
		}
	}
	return ret;
}

void solve(Assignment_Problem p) {
	int sz;
	stack<Assignment_Solution> ST;
	ST.push(p.emptySolution());
	Assignment_Solution best = p.worstSolution();
	double start = clock();
	while(!ST.empty()) {
		Assignment_Solution data = ST.top();
		ST.pop();
		if(data.isFeasible()) {
			if (data.cost < best.cost) {
				best = data;
			}
		}
		else if (data.getBound() < best.cost) { 
			vector<Assignment_Solution> ret = p.expand(data);
			for(int l = 0; l < ret.size(); ++l) {
				ST.push(ret[l]);
			}
		}
	}
	double end = clock();
	best.printSol();
	printf("Time taken : %lf.\n", 1.0*(end - start)/CLOCKS_PER_SEC);
}

int main() {
	Assignment_Problem p;
	scanf("%d", &p.n);
	for(int i = 0; i < p.n; ++i) {
		for(int j = 0; j < p.n; ++j) {
			scanf("%d", &p.cost[i][j]);
		}
	}
	solve(p);
	return 0;
}
