#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <omp.h>
#include "matrix.h"


void multiply(char *c_file, char *a_file, char *b_file, double(*f_mul)(double**,double**,double**,int,int,int)) {
	int m, n, p;	
	int i;
	double **a = read_matrix(a_file, &m, &p);
	double **b = read_matrix(b_file, &p, &n);
	double t;


	//double **c = (*f_mul)(a, b, m, p, n); 

	/*printf("Matrix A:\n");*/
	/*print_matrix(a, m, p);*/
	/*printf("Matrix B:\n");*/
	/*print_matrix(b, p, n);*/
	//allocs matrix c
	double **c = (double **)malloc(sizeof(double*)*m);
	for(i = 0; i < m; i++) {
		c[i] = (double*)malloc(sizeof(double)*n);
	}

	//regular matrix mult
	zeros(c, m, n);
	t = regular(c, a, b, m, p, n);
	//printf("Matrix C:\n");
	//print_matrix(c, m, n);
	printf("Time taken by regular: %lf\n", t);

	//optimized matrix mult
	zeros(c, m, n);
	t = (*f_mul)(c, a, b, m, p, n);
	//printf("Matrix C:\n");
	//print_matrix(c, m, n);
	printf("Time taken by optimization: %lf\n", t);

	//write_matrix(c_file, c, m, n);
	free_matrix(a, m);
	free(a);
	free_matrix(b, p);
	free(b);
	free_matrix(c, m);
	free(c);
}

void print_matrix(double **mat, int m, int n) {
	int i, j;
	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++)
			printf("%.2lf ", mat[i][j]);
		printf("\n");
	}
	printf("\n");
}

double **read_matrix(char *file_name, int *row, int *col) {
	FILE *fp = NULL;
	int i, j;
	double val;
	fp = fopen(file_name, "r+");	
	if(fp == NULL) {
		printf("Could not open file %s\n", file_name);
		exit(1);
	}	
	fscanf(fp, "%d", row);
	fscanf(fp, "%d", col);
	double **m = (double **)malloc(sizeof(double*)*(*row));
	for(i = 0; i < (*row); i++) {
		m[i] = (double*)malloc(sizeof(double)*(*col));
	}
	zeros(m, *row, *col);
	while((fscanf(fp, "%d%d%lf", &i, &j, &val)) != EOF) {
		m[i-1][j-1] = val;		
	}

	fclose(fp);
	return m;
}

void zeros(double **mat, int m, int n) {
	int i, j;
	for(i = 0; i < m; i++)
		for(j = 0; j < n; j++)
			mat[i][j] = 0.f;
}

void free_matrix(double **mat, int row) {
	int i;
	for(i = 0; i < row; i++)
		free(mat[i]);
}

void write_matrix(char *file_name, double **c, int row, int col) {
	int i, j;
	FILE *fp = NULL;
	fp = fopen(file_name, "w+");
	if(fp == NULL) {
		printf("Could not open file: %s\n", file_name);
		exit(0);
	}	
	fprintf(fp, "%d %d\n", row, col);

	for(i = 0; i < row; i++)
		for(j = 0; j < col; j++)
			if(c[i][j] != 0)
				fprintf(fp, "%d %d %lf\n", i+1, j+1, c[i][j]);
	fclose(fp);
}

void transpose(double **b_t, double **b, int m, int n) {
	int i, j;
	for(i = 0; i < m; i++) 
		for(j = 0; j < n; j++)
			b_t[i][j] = b[j][i];
}

double pt(double **c, double **a, double **b, int m, int p, int n) {
	return 0.f;
}

double omp(double **c, double **a, double **b, int m, int p, int n) {
	double t, wtime, sum;
	
	//transpose matrix b here .....
	double **b_t = (double**)malloc(sizeof(double*)*p);
	int i;
	for(i = 0; i < p; i++) {
		b_t[i] = (double*)malloc(sizeof(double)*n);
	}
	transpose(b_t, b, p, n);
	
	wtime = omp_get_wtime();
	#pragma omp parallel
	{
		int i, j, k;
		//omp matrix mutiplication
		#pragma omp for
		for(i = 0; i < m; i++) {
			for(j = 0; j < n; j++) {
				for(k = 0; k < p; k++) {
					c[i][j] += a[i][k] * b_t[j][k];	
					//c[i][j] += a[i][k] * b[k][j];	
				}
			}
		}
	}
	wtime = omp_get_wtime() - wtime;
	free_matrix(b_t, p);
	free(b_t);
	return wtime;

}

double regular(double **c, double **a, double **b, int m, int p, int n) {
	int i, j, k;
	clock_t t1, t2;
	double t;
	/*//transpose matrix b here .....*/
	/*double **b_t = (double**)malloc(sizeof(double*)*p);*/
	/*for(i = 0; i < p; i++) {*/
		/*b_t[i] = (double*)malloc(sizeof(double)*n);*/
	/*}*/
	/*transpose(b_t, b, p, n);*/
	t1 = clock();
	//regular matrix mutiplication
	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++) {
			for(k = 0; k < p; k++) {
				c[i][j] += a[i][k] * b[k][j];	
				//c[i][j] += a[i][k] * b_t[j][k];	
			}
		}
	}
	t2 = clock() - t1;
	t = ((double)t2/CLOCKS_PER_SEC);
	//free_matrix(b_t, p);
	return t;

}

