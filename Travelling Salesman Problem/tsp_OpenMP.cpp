#include <cstdio>
#include <stack>
#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <set>
#include <omp.h>
using namespace std;

int NUM_THREADS;
const int inf = 100000000;

struct TSP_Problem;

struct TSP_Solution {
	void *pts;
	int idx, cost;
	vector<int> taken;
	void printSol() {
		printf("Minimum cost is %d.\n", cost);
	}
	bool isFeasible();
	int getBound(TSP_Problem &p);
};

struct TSP_Problem {
	int n, min_edge;
	vector< vector<int> >graph;
	vector<int> adder;
	void initialise() {
		adder.resize(n);
		for(int i = 0; i < n; ++i) {
			int a = inf, b = inf;
			for(int j = 0; j < n; ++j) {
				if (j == i) {
					continue;
				}
				else if (graph[i][j] <= a) {
					b = a;
					a = graph[i][j];
				}
				else if (graph[i][j] < b) {
					b = graph[i][j];
				}
			}
			adder[i] = (a + b) / 2;
		}
	}
	TSP_Solution emptySolution() {
		TSP_Solution s1;
		s1.idx = 0;
		s1.pts = this;
		s1.cost = 0;
		return s1;
	}
	TSP_Solution worstSolution() {
		TSP_Solution s1;
		s1.pts = this;
		s1.cost = inf;
		s1.idx = 0;
		return s1;
	}
	vector<TSP_Solution> expand(TSP_Solution s);
};

bool TSP_Solution::isFeasible() {
	return (idx == (((TSP_Problem *)pts)->n));
}

int TSP_Solution::getBound(TSP_Problem &p) {
	int bound = cost;
	set<int> done;
	for(int i = 0; i < taken.size(); ++i) {   
		done.insert(taken[i]);
	}
	for(int i = 0; i < ((TSP_Problem *)pts)->n; ++i) {
		if (done.find(i) == done.end()) {
			bound += p.min_edge;
		}
	}
	return bound;
}

vector<TSP_Solution> TSP_Problem::expand(TSP_Solution s) {
	vector<TSP_Solution> ret;
	TSP_Solution s1, other;
	int previous;
	set<int> done;
	for(int i = 0; i < s.taken.size(); ++i) {   
		done.insert(s.taken[i]);
	}
	if (s.taken.size() == 0) {
		for(int i = 0; i < ((TSP_Problem *)s.pts)->n; ++i) {
			other = s;
			other.idx = 1;
			other.taken.push_back(i);
			ret.push_back(other);
		}
		return ret;
	}
	for(int i = 0; i < ((TSP_Problem *)s.pts)->n; ++i) {
		if(done.find(i) == done.end()) {
			s1 = s;
			s1.idx++;
			s1.taken.push_back(i);
			previous = s1.taken[s1.taken.size()-2];
			s1.cost += graph[previous][i];
			ret.push_back(s1);
		}
	}
	return ret;
}

void solve(TSP_Problem p) {
	omp_set_num_threads(NUM_THREADS);
	int sz;
	stack<TSP_Solution> ST;
	ST.push(p.emptySolution());
	TSP_Solution best = p.worstSolution();
	double start = omp_get_wtime();
	while(!ST.empty()) {
		if(ST.size() >= NUM_THREADS) {
			sz = NUM_THREADS;
		}
		else {
			sz = ST.size();
		}
		TSP_Solution data[sz];
		for(int i = 0; i < sz; ++i) {
			data[i] = ST.top();
			ST.pop();
		}
		#pragma omp parallel for 
		for(int i = 0; i < sz; ++i) {
			if(data[i].isFeasible()) {
				data[i].cost += p.graph[data[i].taken.back()][data[i].taken[0]];
				#pragma omp critical
				if (data[i].cost < best.cost) {
					best.cost = data[i].cost;
				}
			}
			else if (data[i].getBound(p) < best.cost) { 
				vector<TSP_Solution> ret = p.expand(data[i]);
				if (data[i].getBound(p) < best.cost) {
					#pragma omp critical
					for(int l = 0; l < ret.size(); ++l) {
						ST.push(ret[l]);
					}
				}
			}
		}
	}
	double end = omp_get_wtime();
	best.printSol();
	printf("Time taken : %lf.\n", end - start);
}

int main(int argc, char const *argv[]) {
 	NUM_THREADS = atoi(argv[1]);
 	TSP_Problem p;
	scanf("%d", &p.n);
	p.min_edge = inf;
	vector<int> temp(p.n);
	for(int i = 0; i < p.n; ++i) {
		for(int j = 0; j < p.n; ++j) {
			scanf("%d", &temp[j]);
			p.min_edge = min(p.min_edge, temp[j]);
		}
		p.graph.push_back(temp);
	}
	p.initialise();
	solve(p);
	return 0;
}
