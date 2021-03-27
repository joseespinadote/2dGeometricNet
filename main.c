#include <stdio.h>

typedef struct vertex;
typedef struct triangle;
typedef struct edge;

typedef struct
{
    float x;
    float y;
    edge *oppositeEdge;
} vertex;

typedef struct
{
    int id; // para fines de depuracion
    float x;
    float y;
} triangle;

typedef struct
{
    vertex *v1;
    vertex *v2;
    triangle *vecino;
} edge;

int main()
{
    FILE *fp;
    char fileBuffer[255];
    
    // TODO: inicializacion de la malla a partir de un rectángulo (¿o 4 puntos?) inicial dado
    printf("ok\n");
    return 0;
}
