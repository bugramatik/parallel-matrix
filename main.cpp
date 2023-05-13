#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <cstdio>
using namespace std;

typedef int** Matrix;

typedef struct {
    Matrix A, B, C, D, J, L, R;
    int N, M, K;
    sem_t* semJ;
    sem_t* semL;
    pthread_t* threads;
} MatrixData;

//Function to read input
void readInput(MatrixData& data){
    int n, m, k;
    scanf("%d %d", &n, &m);
    data.N = n;
    data.M = m;

    // Allocate memory for the matrices and semaphores
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
    data.semL = (sem_t*)malloc(m * sizeof(sem_t));
    for(int i = 0; i < m; ++i){
        data.C[i] = (int*)malloc(k * sizeof(int));
        data.D[i] = (int*)malloc(k * sizeof(int));
        data.L[i] = (int*)malloc(k * sizeof(int));
        sem_init(&data.semL[i], 0, 0);
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

    // Allocate memory for the final result
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
            sem_wait(&data->semJ[i]);
            for (int j = 0; j < data->M; ++j) {
                data->J[i][j] = data->A[i][j] + data->B[i][j];
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

    for (int i = data->N; i < data->M; ++i) {
        if (pthread_equal(self, data->threads[i])) {
            sem_wait(&data->semL[i]);
            for (int j = 0; j < data->M; ++j) {
                data->L[i][j] = data->A[i][j] + data->B[i][j];
            }
            sem_post(&data->semL[i]);
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
            sem_wait(&data->semL[i]);
            for (int j = 0; j < data->K; ++j) {
                data->R[i][j] = 0;
                for (int l = 0; l < data->M; ++l) {
                    data->R[i][j] += data->J[i][l] * data->L[l][j];
                }
            }
            sem_post(&data->semJ[i]);
            sem_post(&data->semL[i]);
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
    MatrixData data;
    readInput(data);

    // Create the threads
    data.threads = (pthread_t*)malloc((data.N + data.M + data.K) * sizeof(pthread_t));

    //Row adding threads for J
    for (int i = 0; i < data.N; ++i) {
        pthread_create(&data.threads[i], NULL, addRowsJ, &data);
    }

    //Row adding threads for L
    for (int i = 0; i < data.M; ++i) {
        pthread_create(&data.threads[i + data.N], NULL, addRowsL, &data);
    }

    //Row multiplying threads
    for (int i = 0; i < data.K; ++i) {
        pthread_create(&data.threads[i + data.N + data.M], NULL, multiplyRowAndColumn, &data);
    }

    // Wait for the threads to finish
    for (int i = 0; i < data.N + data.M + data.K; ++i) {
        pthread_join(data.threads[i], NULL);
    }

    // Print the resulting matrix
    printMatrix(data.L, data.N, data.K);

    // Destroy the semaphores
    for (int i = 0; i < data.N; ++i) {
        sem_destroy(&data.semJ[i]);
    }
    for (int i = 0; i < data.M; ++i) {
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

    return 0;
}

