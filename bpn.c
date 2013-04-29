#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _MSC_VER
#include <windows.h>
#define SLEEP Sleep(1000)
#else
#define SLEEP sleep(1) 
#endif

typedef unsigned int uint;
typedef double n_var;

struct neuro {
	n_var s;
	n_var x;
	n_var b;
};

#define LAYER 3
#define MAX_NEURO_PER_LAYER 3
#define LEARNING_RATE 0.3

uint           lm[LAYER + 1] = {2, 3, 3, 1};
struct neuro  neu[MAX_NEURO_PER_LAYER][LAYER + 1];
n_var         w[MAX_NEURO_PER_LAYER][MAX_NEURO_PER_LAYER][LAYER];

n_var
sigmoid(n_var n)
{
	return 1.0 / (1.0 + exp(-n));
}

n_var 
n_rand()
{
	return ((n_var)rand())/((n_var)RAND_MAX);
}

void 
bpn_init()
{
	uint i, j, l;
	srand(467);

	for (l = 0; l < LAYER; l++)
		for (i = 0; i < lm[l + 1]; i++) {
			neu[i][l + 1].b = n_rand();
			for (j = 0; j < lm[l]; j++)
				w[i][j][l] = n_rand();
		}
}

void
bpn_layer_out(uint layer)
{
	uint i, j;
	for (j = 0; j < lm[layer + 1]; j ++) {
		neu[j][layer + 1].x = neu[j][layer + 1].b;
		for (i = 0; i < lm[layer]; i++)
			neu[j][layer + 1].x += neu[i][layer].x * w[j][i][layer];

		if (layer != LAYER - 1)
			neu[j][layer + 1].x = sigmoid(neu[j][layer + 1].x);
	}
}

void
bpn_layer_bp(uint layer)
{
	uint i, j;
	n_var sum;
	
	for (j = 0; j < lm[layer + 1]; j ++) {
		neu[j][layer + 1].b -= LEARNING_RATE * neu[j][layer + 1].s;
	}

	for (i = 0; i < lm[layer]; i++) {
		sum = 0;
		for (j = 0; j < lm[layer + 1]; j ++) {
			sum += neu[j][layer + 1].s * w[j][i][layer];
			w[j][i][layer]  -= 
				LEARNING_RATE * neu[j][layer + 1].s * neu[i][layer].x;
		}
		neu[i][layer].s = sum * neu[i][layer].x * (1.0 - neu[i][layer].x);
	}
}

n_var train_data[][3] = 
{
	{1, -1, 1},
	{-1, 1, 1},
	{-1, -1, -1},
	{1, 1, -1}
};

#define TRAIN_NUM (sizeof(train_data)/(3*sizeof(n_var)))

int
main()
{
	uint l, i = 0, it = 0;
	n_var y, delta;
	bpn_init();
	
	while (1) {
		neu[0][0].x = train_data[i][0];
		neu[1][0].x = train_data[i][1];
		y = train_data[i][2];
		i = (++i)%TRAIN_NUM;

		for (l = 0; l < LAYER; l++)
			bpn_layer_out(l);

		delta = neu[0][LAYER].x - y;
		neu[0][LAYER].s = 2.0 * delta;

		for (l = 0; l < LAYER; l++)
			bpn_layer_bp(LAYER - l - 1);
		
		printf("[%lf %lf] -> [%lf], y = %lf, delta = %lf \n", 
				neu[0][0].x, neu[1][0].x, neu[0][LAYER].x, y, delta);
		
		if (++it % 500 == 0) {
			printf("training %d ...\n", it);
			SLEEP;
		}
	}

	return 0;
}
