#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <cstdio>
#include "hw2_output.h"

using namespace std;

typedef int** Matrix;

typedef struct {
    Matrix A, B, C, D, J, L, R;
    int N, M, K;
    sem_t* semJ;
    sem_t* semL;
    pthread_t* threads;
    int* counter;
    pthread_mutex_t* mutex;
} MatrixData;

//Function to read input
void readInput(MatrixData& data){
    int n, m, k;
    scanf("%d %d", &n, &m);
    data.N = n;
    data.M = m;

    // Allocating memory for the matrices and semaphores
    data.A = (Matrix)malloc(n * sizeof(int*));
    data.B = (Matrix)malloc(n * sizeof(int*));
    data.J = (Matrix)malloc(n * sizeof(int*));
    data.semJ = (sem_t*)malloc(n * sizeof(sem_t));
    for(int i = 0; i < n; ++i){
        data.A[i] = (int*)malloc(m * sizeof(int));
        data.B[i] = (int*)malloc(m * sizeof(int));
        data.J[i] = (int*)malloc(m * sizeof(int));
        sem_init(&data.semJ[i], 0, 0);
        for(int j = 0; j < m; ++j) {
            scanf("%d", &data.A[i][j]);
        }
    }
    scanf("%d %d", &n, &m);
    for(int i = 0; i < n; ++i){
        for(int j = 0; j < m; ++j) {
            scanf("%d", &data.B[i][j]);
        }
    }

    scanf("%d %d", &m, &k);
    data.K = k;
    data.C = (Matrix)malloc(m * sizeof(int*));
    data.D = (Matrix)malloc(m * sizeof(int*));
    data.L = (Matrix)malloc(m * sizeof(int*));
    data.semL = (sem_t*)malloc(n * sizeof(sem_t));

    for (int i = 0; i < n; ++i) {
        sem_init(&data.semL[i], 0, 0);
    }

    for(int i = 0; i < m; ++i){
        data.C[i] = (int*)malloc(k * sizeof(int));
        data.D[i] = (int*)malloc(k * sizeof(int));
        data.L[i] = (int*)malloc(k * sizeof(int));

        for(int j = 0; j < k; ++j) {
            scanf("%d", &data.C[i][j]);
        }
    }
    scanf("%d %d", &m, &k);
    for(int i = 0; i < m; ++i){
        for(int j = 0; j < k; ++j) {
            scanf("%d", &data.D[i][j]);
        }
    }

    data.counter = (int*)malloc((k) * sizeof(int));
    data.mutex = (pthread_mutex_t*)malloc((k) * sizeof(pthread_mutex_t));

    for(int i = 0; i < k; ++i){
        data.counter[i] = 0;
        pthread_mutex_init(&data.mutex[i], NULL);
    }

    // Allocating memory for the final result
    data.R = (Matrix)malloc(n * sizeof(int*));
    for(int i = 0; i < n; ++i){
        data.R[i] = (int*)malloc(k * sizeof(int));
    }
}

// Function to add two rows of matrices
void* addRowsJ(void* arg) {
    MatrixData* data = (MatrixData*)arg;
    pthread_t self = pthread_self();

    for (int i = 0; i < data->N; ++i) {
        if (pthread_equal(self, data->threads[i])) {
            for (int j = 0; j < data->M; ++j) {
                data->J[i][j] = data->A[i][j] + data->B[i][j];
                hw2_write_output(0,i+1,j+1,data->J[i][j]);

            }
            sem_post(&data->semJ[i]);
            break;
        }
    }

    return NULL;

}

// Function to add two rows of matrices
void* addRowsL(void* arg) {

    MatrixData* data = (MatrixData*)arg;
    pthread_t self = pthread_self();

    for (int i = data->N; i <= data->N + data->M; ++i) {
        if (pthread_equal(self, data->threads[i])) {
            int row = i - data->N;
            for (int j = 0; j < data->K; ++j) {
                data->L[row][j] = data->C[row][j] + data->D[row][j];
                hw2_write_output(1,row+1,j+1,data->L[row][j]);
                pthread_mutex_lock(&data->mutex[j]);
                data->counter[j]++;
                if(data->counter[j] == data->M){
                    for(int k = 0; k < data->N; ++k){
                        sem_post(&data->semL[k]);
                    }
                }
                pthread_mutex_unlock(&data->mutex[j]);
            }
            break;
        }
    }

    return NULL;
}

// Function to multiply a row and a column of matrices
void* multiplyRowAndColumn(void* arg) {

    MatrixData* data = (MatrixData*)arg;
    pthread_t self = pthread_self();

    for (int i = 0; i < data->N; ++i) {
        if (pthread_equal(self, data->threads[i + data->N + data->M])) {
            sem_wait(&data->semJ[i]);
            for (int j = 0; j < data->K; ++j) {
                sem_wait(&data->semL[i]);
                data->R[i][j] = 0;
                for (int l = 0; l < data->M; ++l) {
                    data->R[i][j] += data->J[i][l] * data->L[l][j];
                }
                hw2_write_output(2,i+1,j+1,data->R[i][j]);
            }
            break;
        }
    }

    return NULL;
}

void printMatrix(Matrix matrix, int n, int m){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main() {

    hw2_init_output();
    MatrixData data;
    readInput(data);

    // Creating the threads
    data.threads = (pthread_t*)malloc((data.N + data.M + data.N) * sizeof(pthread_t));

    //Row adding threads for J
    for (int i = 0; i < data.N; ++i) {
        pthread_create(&data.threads[i], NULL, addRowsJ, &data);
    }

    //Row adding threads for L
    for (int i = 0; i < data.M; ++i) {
        pthread_create(&data.threads[i + data.N], NULL, addRowsL, &data);
    }

    //Row multiplying threads
    for (int i = 0; i < data.N ; ++i) {
        pthread_create(&data.threads[i + data.N + data.M], NULL, multiplyRowAndColumn, &data);
    }

    // Waiting for the threads to finish
    for (int i = 0; i < data.N + data.M + data.N; ++i) {
        pthread_join(data.threads[i], NULL);
    }
    printMatrix(data.R, data.N, data.K);


    // Destroying the semaphores
    for (int i = 0; i < data.N; ++i) {
        sem_destroy(&data.semJ[i]);
    }
    for (int i = 0; i < data.N; ++i) {
        sem_destroy(&data.semL[i]);
    }

    // Free the resources
    free(data.threads);
    free(data.A);
    free(data.B);
    free(data.C);
    free(data.D);
    free(data.J);
    free(data.L);
    free(data.R);
    free(data.semJ);
    free(data.semL);
    free(data.counter);
    free(data.mutex);
    return 0;
}

