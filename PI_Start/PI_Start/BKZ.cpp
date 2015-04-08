
#include "BKZ.h"

bool passvec(int* v, int index){
	int i;
	if (v[index] != 1){ return false; }
	for(i = 0; i < dim; i++){
		if (i != index && v[i] != 0) return false;
	}
	return true;
}

double** BKZ(double** bases, int beta, double delta){
	int k, h, i, l;
    int *v;
	double aux;

	int z = 0, j = 0;

	lll(bases, delta, dim-1);
	while (z < dim - 1){
		j = (j % (dim-1)) + 1;
		k = fmin(j + beta - 1, dim);
		h = fmin(k + 1, dim);
		
		v = ENUM(c, j - 1, k - 1);
		if (!passvec(v, j-1))
		{
			z = 0;
			//Transforma a matriz para a enviar
			//Insere nova base
			for (i = 0; i < dim; i++){
				//alterar vetor
				aux = 0.0;
				for (l = 0; l < dim; l++){
					aux += v[i] * bases[l][i];
				}
				bases[dim][i] = aux;
			}
			//Faz shift da base
			shiftVector(bases, j - 1, dim);


			//Chama LLL com a nova matriz, acaba em h pois vai ter mais 1 vetor a computar pela LLL
			lll(bases, delta, h);
		}else{
			z++;
			//chama LLL com a matriz actual
			lll(bases, delta, h-1);
		}
	}
	return bases;
}