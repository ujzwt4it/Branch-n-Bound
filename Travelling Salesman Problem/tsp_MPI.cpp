#include <cstdio>
#include <stack>
#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <set>
#include <mpi.h>
using namespace std;

const int MAXL = 3000;
const int inf = 100000000;

struct TSP_Problem;

struct TSP_Solution {
	void *pts;
	int idx, cost;
	vector<int> taken;
	void printSol() 
	{
		printf("Minimum cost is %d.\n", cost);
	}
	int toStr(char *buffer) {
		int p = 0;
		p += sprintf(buffer + p, " %d %d ", idx, cost);
		for (int i = 0; i < idx; i++) {
			p += sprintf(buffer + p, " %d ", taken[i]);
		}
		return p;
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
		s1.idx = 0;
		s1.pts = this;
		s1.cost = inf;
		return s1;
	}
	static TSP_Solution decodeSolution(char *buffer, int &idx) {
		TSP_Solution s;
		int b, val;
		sscanf(buffer + idx, "%d%d%n", &(s.idx), &(s.cost), &b);
		idx += b;
		s.taken.resize(s.idx);
		for (int i = 0; i < s.idx; i++) {
			sscanf(buffer + idx, "%d%n", &val, &b);
			idx += b;
			s.taken[i] = val;
		}
		return s;
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

char buffer[MAXL];

void solve(TSP_Problem p) 
{
	MPI_Init(0, NULL);
	int pid, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &rank);
	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();
	MPI_Status st;
	stack<TSP_Solution> q;
	int best_cost;
	if (pid == 0) {
		TSP_Solution start = p.emptySolution(), best = p.worstSolution();
		bool is_idle[rank + 1];
		int num_idle = rank - 1;
		for (int i = 1; i < rank; i++) {
			is_idle[i] = true;
		}
		int idx = 0;
		idx += sprintf(buffer + idx, "%s ", "P ");
		idx += start.toStr(buffer + idx);
		idx += sprintf(buffer + idx, "%d ", best.cost);
		MPI_Send(buffer, strlen(buffer), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		is_idle[1] = false;
		num_idle--;
		while (num_idle + 1 < rank) {
			MPI_Recv(buffer, MAXL, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &st);
			char message[4];
			int idx = 0, b;
			sscanf(buffer + idx, "%s%n", message, &b);
			idx += b;
			if (strcmp(message, "GS") == 0) {
				int slaves_needed;
				int asked = st.MPI_SOURCE;
				sscanf(buffer + idx, "%d%n", &slaves_needed, &b);
				int slaves_given = min(slaves_needed, num_idle);
				idx = 0;
				idx += sprintf(buffer + idx, "%d ", slaves_given);
				idx += sprintf(buffer + idx, "%d ", best.cost);
				int i = 1;
				while (slaves_given) {
					if (!is_idle[i]) {
						i++;
					}
					else {
						idx += sprintf(buffer + idx, "%d ", i);
						slaves_given--;
						num_idle--;
						is_idle[i] = false;
					}
				}
				MPI_Send(buffer, strlen(buffer) + 1, MPI_CHAR, asked, 0, MPI_COMM_WORLD);
			}
			else if (strcmp(message, "I") == 0) {
				num_idle++;
				is_idle[st.MPI_SOURCE] = true;
			} 
			else if (strcmp(message, "D") == 0) {
				TSP_Solution candidate_sol = TSP_Problem::decodeSolution(buffer, idx);
				candidate_sol.cost += p.graph[candidate_sol.taken.back()][candidate_sol.taken[0]];
				if (candidate_sol.cost < best.cost) {
					int asked = st.MPI_SOURCE;
					best = candidate_sol;
				}
			}
		}
		for (int i = 1; i < rank; i++) {
			strcpy(buffer, "F ");
			MPI_Send(buffer, strlen(buffer) + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		}
		best.printSol();
	} 
	else {
		while (1) {
			MPI_Recv(buffer, MAXL, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &st);
			int b, idx = 0;
			char message[4];
			sscanf(buffer + idx, "%s%n", message, &b);
			idx += b;
			char req[MAXL];
			if (strcmp(message, "P") == 0) {
				TSP_Solution s = TSP_Problem::decodeSolution(buffer, idx);
				s.pts = &p;
				q.push(s);
				sscanf(buffer + idx, "%d%n", &best_cost, &b);
				idx += b;
				while (!q.empty()) {
					TSP_Solution sol = q.top();
					q.pop();
					if (sol.getBound(p) > best_cost) {
						continue;
					}
					if (sol.isFeasible()) {
						idx = 0;
						idx += sprintf(buffer + idx, "D ");
						idx += sol.toStr(buffer + idx);
						idx += sprintf(buffer + idx, " ");
						MPI_Send(buffer, strlen(buffer) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
						continue;
					}
					vector<TSP_Solution> v = p.expand(sol);
					for (int i = 0; i < v.size(); i++) {
						q.push(v[i]);
					}
					if (q.empty()) {
						break;
					}
					sprintf(req, "GS %d ", (int) q.size());
					MPI_Send(req, strlen(req), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
					req[0] = '\0';
					MPI_Recv(req, MAXL, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &st);
					int slaves_available, slave_number, p1 = 0;
					sscanf(req + p1, "%d%n", &slaves_available, &b);
					p1 += b;
					sscanf(req + p1, "%d%n", &best_cost, &b);
					p1 += b;
					for (int i = 0; i < slaves_available; i++) {
						sscanf(req + p1, "%d%n", &slave_number, &b);
						p1 += b;
						idx = 0;
						idx += sprintf(buffer + idx, "%s ", "P ");
						idx += q.top().toStr(buffer + idx);
						q.pop();
						idx += sprintf(buffer + idx, "%d ", best_cost);
						MPI_Send(buffer, strlen(buffer), MPI_CHAR, slave_number, 0, MPI_COMM_WORLD);
					}
				}
				sprintf(req, "I ");
				MPI_Send(req, strlen(req) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
			else if (strcmp(message, "F") == 0) {
				assert(st.MPI_SOURCE == 0);
				break;
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	double end = MPI_Wtime();
	MPI_Finalize();
	if (pid == 0) {
		printf("Time taken : %lf.\n", end - start);
	}
}

int main(int argc, char const *argv[]) 
{
	FILE * fp = fopen(argv[1], "r");
	TSP_Problem p;
	fscanf(fp, "%d", &p.n);
	p.min_edge = (int) 1e9;
	vector<int>temp(p.n);
	for (int i = 0; i < p.n; ++i) {
		for (int j = 0; j < p.n; ++j) {
			fscanf(fp, "%d", &temp[j]);
			p.min_edge = min(p.min_edge, temp[j]);
		}
		p.graph.push_back(temp);
	}
	p.initialise();
	solve(p);
	return 0;
}
