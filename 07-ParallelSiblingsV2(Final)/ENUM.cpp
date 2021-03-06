//Algoritmo em http://www.csie.nuk.edu.tw/~cychen/Lattices/Lattice%20Basis%20Reduction_%20Improved%20Practical%20Algorithms%20and%20Solving%20Subset%20Sum%20Problems.pdf - pagina 16

#include "ENUM.h"
#include "math.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

short nthreads;
short *u, **uT, **d, **delta, **v, **vectors;
double **cT, **y, cL;
LEnum list = NULL;

pthread_mutex_t u_Mutex, toPop_Mutex;


void printVec(short bound, short id);

void initEnum(short n_threads){
    short i, auxDim = dim+1;
    nthreads = n_threads;
    
    //Allocate memory
    u = (short*)_mm_malloc(dim*sizeof(NLEnum), 64);
    list = (LEnum)_mm_malloc(sizeof(NLEnum), 64);
    
    list->count = 0;
    list->head = list->tail = NULL;;
    
    //Fill u with the shortest vector
    for(short i=1; i<=dim; i++)
        u[i] = 0;
        
    
    cL = B[0];
    u[0]=1;
    
    //Prepare memory to each thread
    delta = (short**)_mm_malloc(n_threads*sizeof(short*), 64);
    uT = (short**)_mm_malloc(n_threads*sizeof(short*), 64);
    d = (short**)_mm_malloc(n_threads*sizeof(short*), 64);
    v = (short**)_mm_malloc(n_threads*sizeof(short*), 64);
    cT = (double**)_mm_malloc(n_threads*sizeof(double*), 64);
    y = (double**)_mm_malloc(n_threads*sizeof(double*), 64);
    
    for(i=0; i<n_threads; i++){
        delta[i] = (short*)_mm_malloc(auxDim*sizeof(short), 64);
        uT[i] = (short*)_mm_malloc(auxDim*sizeof(short), 64);
        d[i] = (short*)_mm_malloc(auxDim*sizeof(short), 64);
        v[i] = (short*)_mm_malloc(auxDim*sizeof(short), 64);
        cT[i] = (double*)_mm_malloc(auxDim*sizeof(double), 64);
        y[i] = (double*)_mm_malloc(auxDim*sizeof(double), 64);
    }
    
    //Prepare to Pthreads
    pthread_mutex_init(&u_Mutex, NULL);
    pthread_mutex_init(&toPop_Mutex, NULL);
}


void moveDown(short id, short t, short s){
    
    double aux = 0.0;
    short i;
    
    for (i = t + 1; i <= s; i++){
        aux += uT[id][i] * mu[i][t];
    }
    
    y[id][t] = aux;
    uT[id][t] = v[id][t] = short(round(-aux));
    delta[id][t] = 0;
    
    if (uT[id][t] > -aux)
        d[id][t] = -1;
    else
        d[id][t] = 1;
    
    //Prepare cT[t] to next iteration
    aux = y[id][t] + uT[id][t];
    cT[id][t] = cT[id][t + 1] + (aux * aux) * B[t];
}





void moveUP(short id, short t, short s){

    if(t < s){
        delta[id][t] = -delta[id][t];
    }
    if(delta[id][t]*d[id][t] >= 0){
        delta[id][t] += d[id][t];
    }
    uT[id][t] = v[id][t] + delta[id][t];
    
    //Prepare cT[t] to next iteration
    aux = y[id][t] + uT[id][t];
    cT[id][t] = cT[id][t + 1] + (aux * aux) * B[t];
}

int startSet(short id, Enum set){
	short i;
	short bound = set->bound+1;
    
    //Clean vectors from preciously executions
    for(i=0; i<=bound; i++)
        y[id][i] = 0.0;
    
    for(i=0; i<=bound; i++)
        delta[id][i] = 0;
    for(i=0; i<=bound; i++)
        d[id][i] = 1;
    for(i=0; i<=bound; i++)
        v[id][i] = 0;
    for(i=0; i<=bound; i++)
        uT[id][i] = 0;
    
    
    //Prepare to start by type
    if(set->type == 0){
        uT[id][0] = 1;
        for(i=1; i<=bound; i++)
            cT[id][i] = 0.0;
        
    } else if(set->type == 1){
        //Base Start
        cT[id][bound] = 0.0;
        cT[id][bound-1] = B[bound-1];
        uT[id][bound-1] = 1;
        
    } else
        if(set->type == 2 || set->type == 3 || set->type == 4){
            cT[id][bound] = 0.0;
            cT[id][bound-1] = B[bound-1];
            uT[id][bound-1] = 1;
            
            short t=bound-2;
            
            if(set->vec != NULL){

		set->vec[set->level-1] = set->sibling;

                short j=0;
                while(t>=(bound-set->level-1)){
                    moveDown(id, t, bound-1);
                    if(cT[id][t] > cL){
                        printf("%d:Lower cL |Type:%d, Sib:%d, Level:%d\n",bound, set->type,set->sibling, set->level);
                        return 1;
                    }                        
                    while(uT[id][t] != set->vec[j]){                        
                        moveUP(id, t, bound-1);
                        if(abs(uT[id][t]) > abs(set->vec[j])){
                            printf("%d:ERRO (Jump Sib) Type:%d, Sib:%d, Level:%d\n",bound, set->type,set->sibling, set->level);
                            return 1;
                        }
                    }                  
                    t--;
                    j++;
                }
            }else{            
                while(t>=(bound-set->level-1)){
                    moveDown(id, t, bound-1);
                    
                    if(cT[id][t] > cL){
                        printf("%d:Lower cL |Type:%d, Sib:%d, Level:%d\n",bound, set->type,set->sibling, set->level);
                        return 1;
                    }
                    
                    if(t == (bound-set->level-1)){
                        
                        while(uT[id][t] != set->sibling){
                            
                            moveUP(id, t, bound-1);
                            if(abs(uT[id][t]) > abs(set->sibling)){
                                printf("%d:ERRO (Jump Sib) Type:%d, Sib:%d, Level:%d\n",bound, set->type,set->sibling, set->level);
                                return 1;
                            }
                        }
                        
                    } else if(uT[id][t] != 0){
                        printf("%d:ERRO (Not '0') Type:%d, Sib:%d, Level:%d\n",bound, set->type,set->sibling, set->level);
                        printVec(dim, id);

                        return 1;
                    }
                    
                    t--;
                }
            }
        }
    return 0;
}


Enum newEnumElem(short bound, short sibling, short type, short level, short *vec){
    Enum st = (Enum)_mm_malloc(sizeof(NEnum),64);
    st->next = NULL;
    st->bound = bound-1;
    st->type = type;
    st->sibling = sibling;
    st->level = level;
    if(vec!=NULL){
        st->vec = (short*)_mm_malloc(4*sizeof(short),64);
        memcpy(&st->vec[0], vec, 4*sizeof(short));
    }else{
        st->vec = NULL;
    }
    return st;
}

void addTail(Enum newSet){
    if(list->count != 0){
        list->tail->next=newSet;
        list->tail = newSet;
        list->count++;
    }else{
        list->head = list->tail = newSet;
        list->count=1;
    }
}

Enum pop(){
    Enum aux;
    
    //Lock critical zone
    pthread_mutex_lock(&toPop_Mutex);
    
    if(!list->head){
        pthread_mutex_unlock(&toPop_Mutex);
        return NULL;
    }
        
    aux = list->head;
    list->head = list->head->next;
    list->count--;
    
    //Unlock critical zone
    pthread_mutex_unlock(&toPop_Mutex);
 
    return aux;
}


//ENUM accordingly C. P. Schnorr && M. Euchner
void EnumSET(Enum set, short id){
    
    double aux;
    short s, t;
    short i;
    short bound = set->bound;
    short toCopy = bound + 1;
    
    if(startSet(id, set))
        return;
  //  printVec(dim, id);
    printf("Original Vec\n");
    printVec(dim,id);
    //Start on leaf (like Schnorr)
    if(set->type==0){
        s = t = 0;
        aux = y[id][t] + uT[id][t];
        cT[id][t] = cT[id][t + 1] + (aux * aux) * B[t];

    //One Gamma
    } else if(set->type == 1){
        s = t = bound;

    //Starts on given sibling and go on
    } else if(set->type == 2){
        s = bound;
        bound = bound - set->level;
        t=bound;
        
    //Search just in one sibling
    } else if(set->type == 3){
        s = bound;
        bound = bound - set->level-1;
        moveDown(id, bound, s);
        t=bound;
    } else if(set->type == 4){
        s = bound;
        bool finished = false;
        short i;
        for(i=set->level-2; i>=0 && !finished; i--){
            if(set->vec[i]==2){
                bound++;
            }else{          
                finished = true;
            }                
        }
	if(set->vec[0]==2&&!finished){bound++;}
	bound -= set->level;
        t=bound;
    }
    printf("id: %d | bound = %d, type = %d, sibling = %d\n", id, set->bound, set->type, set->sibling);
    
    while(t <= bound){
        if (cT[id][t] < cL){
            if (t > 0){
                //moveDown
                t--;
                aux = 0.0;
                
                for (i = t + 1; i <= s; i++){
                    aux += uT[id][i] * mu[i][t];
                }
                
                y[id][t] = aux;
                uT[id][t] = v[id][t] = short(round(-aux));
                delta[id][t] = 0;
                
                if (uT[id][t] > -aux){
                    d[id][t] = -1;
                }
                else{
                    d[id][t] = 1;
                }
                
                //Prepare cT[t] to next iteration
                aux = y[id][t] + uT[id][t];
                cT[id][t] = cT[id][t + 1] + (aux * aux) * B[t];
                
            }
            else{
                //UpdateVector
                printf("%d:UPDATE Type:%d, Sib:%d, Level:%d -->%f / %f\n",s+1, set->type,set->sibling, set->level ,cT[id][0],cL);
                
                
                //Lock critical zone
                pthread_mutex_lock(&u_Mutex);
                    cL = cT[id][0];
                    memcpy(&u[0], uT[id], toCopy*sizeof(short));
                    for(i=toCopy; i<dim; i++)
                        u[i] = 0;
                
                //Unlock critical zone
                pthread_mutex_unlock(&u_Mutex);
            }
        }
        else{
            //moveUp
            t++;
            s = (s<t)?t:s; //Get max value
            
            if(t < s){
                delta[id][t] = -delta[id][t];
            }
            if(delta[id][t]*d[id][t] >= 0){
                delta[id][t] += d[id][t];
            }
            uT[id][t] = v[id][t] + delta[id][t];
            
            //Prepare cT[t] to next iteration
            aux = y[id][t] + uT[id][t];
            cT[id][t] = cT[id][t + 1] + (aux * aux) * B[t];
            
        }
    }
}

void* threadHander(void* vID){
    
    short id = *((short *) vID);
    Enum set = NULL;

    while (list->count>0) {
        set = pop();
        if(set){
            EnumSET(set, id);
        }
    }

    return NULL;
}

short createVectors(short veclen){
    short totvec = pow(4,veclen), i, j;
    bool comp=false;
    vectors = (short**)_mm_malloc(totvec*sizeof(short*),64);
    short* sigvec = (short*)_mm_malloc(veclen*sizeof(short),64);
    short* auxvec = (short*)_mm_malloc((veclen+1)*sizeof(short),64);
    
    for(i=0; i<=veclen; i++){
        sigvec[i] = -1;
        auxvec[i] = 0;
    }
    for(i=0; i<totvec; i++){
        comp=false;
        vectors[i] = (short*)_mm_malloc((veclen+1)*sizeof(short),64);
        memcpy(&vectors[i][0],auxvec,(veclen+1)*sizeof(short));
        for(j=veclen-1; j>=0 && !comp; j--){
            if(auxvec[j]!=2){comp=true;}
            if(sigvec[j]==-1){
                auxvec[j] = sigvec[j]*auxvec[j]+1;
            }else{
                if(auxvec[j]==1){
                    auxvec[j]=-1;
                }else{
                    auxvec[j]=0;
                }
            }
            sigvec[j] *= -1;
        }
    }
    free(auxvec);
    free(sigvec);
    
//    printf("printing vectors\n");
//    for(i=0; i<totvec; i++){
//        for(j=0; j<=veclen; j++){
//            printf("%d - %d\t",j,vectors[i][j]);
//        }
//        printf("\n");
//    }
    return totvec;
}

void freeVectors(short vecs){
    short i;
    for (i=0; i< vecs; i++){
        free(vectors[i]);
    }
    free(vectors);
}

//From vector '0 0 0 0 0'
void creatTasks(short bound, short level, short totvec){
    short i;
    //short depth = bound - level;
    Enum set = NULL;
    
    for(i=0; i<totvec; i++){
        set = newEnumElem(bound, 0, 3, level+1, vectors[i]);
        addTail(set);
        set = newEnumElem(bound, 1, 3, level+1, vectors[i]);
        addTail(set);
        set = newEnumElem(bound, -1, 3, level+1, vectors[i]);
        addTail(set);
        if(vectors[i][level-1]==2){
            set = newEnumElem(bound, 2, 4, level+1, vectors[i]);
            addTail(set);
        }else{
            set = newEnumElem(bound, 2, 2, level+1, vectors[i]);
            addTail(set);
        }
    }

//    for(i=1; i<=level; i++){
//
//        set = newEnumElem(bound, 1, 3, i, NULL);
//        addTail(set);
//        set = newEnumElem(bound, -1, 3, i, NULL);
//        addTail(set);
//        set = newEnumElem(bound, 2, 2, i, NULL);
//        addTail(set);
//    }
//    set = newEnumElem(bound, 0, 2, i, NULL);
//    addTail(set);
}

//OLD FUNCTION
//short* ENUM(){
    
//NEW FUNCTION WITH INPUTS
short* ENUM(short creatRange, short divRange, short MAX_DEPTH, short veclength){
    short i, j, n = 1;
    //short i, j, n = 1, creatRange = dim - nthreads, MAX_DEPTH = 0.4*dim, divRange = 0.6*dim, veclength;
    Enum set = NULL;
    short  totvec;
    
    totvec = createVectors(veclength);
    
    for (i = dim; i > creatRange && i>0; i--){
            creatTasks(i, veclength, totvec);
    }
    
    freeVectors(totvec);
    
    for(i=creatRange; i>divRange && i>0; i--){
        set = newEnumElem(i, 0, 3, 1, NULL);
        addTail(set);
	n++;
        set = newEnumElem(i, 1, 3, 1, NULL);
        addTail(set);
        n++;
        set = newEnumElem(i, -1, 3, 1, NULL);
        addTail(set);
        n++;
        set = newEnumElem(i, 2, 2, 1, NULL);
        addTail(set);
        n++;
    }
    
    
    //Prepare list with tasks
    for(; i>MAX_DEPTH && i>0; i--){
        set = newEnumElem(i, 1, 1, 1, NULL);
        addTail(set);
        n++;
    }
    if(i>0)set = newEnumElem(i, 1, 0, 1, NULL);
    addTail(set);
    
    printf("Total divisions: %d\n", list->count);
    
    pthread_t tHandles[nthreads];
    
    //Threads Start
    for (i = 0; i < nthreads; i++) {
        short *threadNum = (short*)malloc(sizeof (short));
        *threadNum = i;
        pthread_create(&tHandles[i], NULL, threadHander, (void *)threadNum);

    }
    
    //Threads Join
    for (i = 0; i < nthreads; i++)
        pthread_join(tHandles[i], NULL);
    
    return u;
}


void printVec(short bound, short id){
    short i;
    
    short dimension = bound;
    
    printf("uT:");
    for(i=0; i<dimension; i++)
        printf("%d ",uT[id][i]);
    printf("\n");
  /*
    printf("d:");
    for(i=0; i<dimension; i++)
        printf("%d ",d[id][i]);
    printf("\n");
    
    printf("delta:");
    for(i=0; i<dimension; i++)
        printf("%d ",delta[id][i]);
    printf("\n");
    
    printf("v:");
    for(i=0; i<dimension; i++)
        printf("%d ",v[id][i]);
    printf("\n");
   
    printf("cT:");
    for(i=0; i<dimension; i++)
        printf("%.2f ",cT[id][i]);
    printf("\n");
    
    printf("Y:");
    for(i=0; i<dimension; i++)
        printf("%.2f ",y[id][i]);
    printf("\n");
    printf("\n");
   */
}
