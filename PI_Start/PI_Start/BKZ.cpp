//N = Dimensao, por enquanto definida globalmente para simplificar
#define N  2;


bool passvec(double v[]){
	int i;
	if (v[0]!=1){return false;}
	for(i = 1; i < N; i++){
		if (v[i] != 0) return false;
	}
	return true;
}

double** BKZ(double *bases[], double *u[], double *c[], int beta, double d){
	int z = 0, j = 0, k, h, alt1, alt2, i, l;
	double v[N];
	double aux[][];
	LLL(&bases, d);
	while (z < N - 1){
		j = (j * mod(N-1)) + 1; 
		k = min(j + beta - 1, N);
		h = min(k + 1, n);
		//cria nova matriz de ortogonalizacao para enviar para o ENUM
		//v = ENUM();
		if (!passvec(v))
		{
			z = 0;
			/*Transforma a matriz para a enviar*/
			alt1 = k - j;
			for(alt2 = h; alt>j-1; alt2--){
				for(i = 0; i < N; i++){
					bases[alt2+alt1][i] = bases[alt2];
				}
			}
			for(alt2 = j; alt2 <= k ; alt2++){
				for(i = 0; i < N; i++){
					bases[alt2][i] *= v[alt2];
				}
			}
			/*Chama LLL com a nova matriz*/
			LLL(&bases, d);
		}else{
			z++;
			/*chama LLL com a matriz actual*/
			LLL(&bases, d);
		}
	}
	return bases;
}