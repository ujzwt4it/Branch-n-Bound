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

const int N = 20;
const int MAXL = 3000;
const int inf = 100000000;

struct Assignment_Solution {
	void *pts;
	int cost, idx;
	int mapping[N];
	int getCost() {
		return cost;
	}
	int toStr(char *buffer) {
		int p = 0;
		p += sprintf(buffer + p, " %d %d ", idx, cost);
		for (int i = 0; i < idx; i++) {
			p += sprintf(buffer + p, " %d ", mapping[i]);
		}
		return p;
	}
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
	static Assignment_Solution decodeSolution(char *buffer, int &idx) {
		Assignment_Solution s;
		int b, val;
		sscanf(buffer + idx, "%d%d%n", &(s.idx), &(s.cost), &b);
		idx += b;
		for(int i = 0; i < s.idx; ++i) {
			sscanf(buffer + idx, "%d%n", &val, &b);
			idx += b;
			s.mapping[i] = val;
		}
		return s;
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

char buffer[MAXL];

void solve(Assignment_Problem p) {
	MPI_Init(0, NULL);
	int pid, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &rank);
	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();
	MPI_Status st;
	stack<Assignment_Solution> q;
	int best_cost;
	if (pid == 0) {
		Assignment_Solution start = p.emptySolution(), best = p.worstSolution();
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
				Assignment_Solution candidate_sol = Assignment_Problem::decodeSolution(buffer, idx);
				if (candidate_sol.cost < best.cost) {
					best = candidate_sol;
					best_cost = candidate_sol.cost;
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
				Assignment_Solution s = Assignment_Problem::decodeSolution(buffer, idx);
				s.pts = &p;
				q.push(s);
				sscanf(buffer + idx, "%d%n", &best_cost, &b);
				idx += b;
				while (!q.empty()) {
					Assignment_Solution sol = q.top();
					q.pop();
					if (sol.getBound() > best_cost) {
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
					vector<Assignment_Solution> v = p.expand(sol);
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

int main(int argc, char const *argv[]) {
 	FILE * fp = fopen(argv[1], "r");
	Assignment_Problem p;
	fscanf(fp, "%d", &p.n);
	for(int i = 0; i < p.n; ++i) {
		for(int j = 0; j < p.n; ++j) {
			fscanf(fp, "%d", &p.cost[i][j]);
		}	
	}
	solve(p);
	return 0;
}