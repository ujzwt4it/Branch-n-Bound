#include <bits/stdc++.h>
using namespace std;

typedef long long LL;

const int lim_n = 50;

void generate_matrix(int n, int m, LL x, LL y) {
	LL mod = (y - x + 1);
	LL displace = x;
	LL matrix[n][m];
	for(int i = 0; i < n; ++i) {
		matrix[i][i] = 0;
		for(int j = 0; j < n; ++j) {
			LL number = ((LL)rand() * rand()) % mod + displace;
			assert(number >= x && number <= y);
			printf("%lld", number);
			if (j < m) {
				printf(" ");
			}
		}
		printf("\n");
	}
}

int main() {
	freopen("assignment2.txt", "w", stdout);
	srand(unsigned(time(0)));
	int n = lim_n;
	printf("%d\n", n);
	LL range_low = 1;
	LL range_high = 1000;
	generate_matrix(n, n, range_low, range_high);
	return 0;
}