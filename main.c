#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LARGO_MALLA 32
#define TAMANO_BUFFER 64

struct triangle;

typedef struct vertex {
    float x, y;
} vertex;

typedef struct edge {
    vertex *v1, *v2;
    struct triangle *vecino;
} edge;

typedef struct triangle {
    edge *e1, *e2, *e3;
} triangle;

int main()
{
    FILE *fpInput, *fpOutput;
    char fileBuffer[TAMANO_BUFFER];
    char *fileInput="puntos.txt";
    char *fileOutput="salida.txt";
    triangle triangulos[LARGO_MALLA];
    vertex vertices[LARGO_MALLA+2];
    vertex newPoints[LARGO_MALLA+2]; /* para depurar */
    vertex *tempVertex;
    float newPointX, newPointY, detE1,
        detE2, detE3;
    int i, j, numPoints, numTriangulos = 0,
        offsetTriangle = 0;

    printf(".::el trianguleitor::. v0.3\n");

    vertices[0].x = 0; vertices[0].y = 4;
    vertices[1].x = 0; vertices[1].y = 0;
    vertices[2].x = 4; vertices[2].y = 0;
    vertices[3].x = 4; vertices[3].y = 4;
    triangulos[0].e1 = &(edge){.v1=&vertices[0], .v2=&vertices[1], NULL};
    triangulos[0].e2 = &(edge){.v1=&vertices[1], .v2=&vertices[2], NULL};
    triangulos[0].e3 = &(edge){.v1=&vertices[2], .v2=&vertices[0], NULL};
    triangulos[1].e1 = &(edge){.v1=&vertices[2], .v2=&vertices[3], NULL};
    triangulos[1].e2 = &(edge){.v1=&vertices[3], .v2=&vertices[0], NULL};
    triangulos[1].e3 = &(edge){.v1=&vertices[0], .v2=&vertices[2], NULL};
    triangulos[0].e3->vecino = &triangulos[1];
    triangulos[1].e3->vecino = &triangulos[0];
    numTriangulos = 2;

    fpInput = fopen(fileInput, "r"); {}
    if(!fpInput) {
        printf("can't open file\n");
        return 1;
    }
    numPoints=0;
    offsetTriangle = numTriangulos;
    while(!feof(fpInput)) {
        fscanf(fpInput, "%f %f", &newPointX, &newPointY);
        newPoints[numPoints] = (vertex){.x=newPointX, .y=newPointY};
        for(i=0; i<numTriangulos;i++) {
            /*
            d=(x−x1)(y2−y1)−(y−y1)(x2−x1)
            ref: https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located
            */
            detE1 = (newPointX-triangulos[i].e1->v1->x)*
                (triangulos[i].e1->v2->y-triangulos[i].e1->v1->y)-
                (newPointY-triangulos[i].e1->v1->y)*
                (triangulos[i].e1->v2->x-triangulos[i].e1->v1->x);
            detE2 = (newPointX-triangulos[i].e2->v1->x)*
                (triangulos[i].e2->v2->y-triangulos[i].e2->v1->y)-
                (newPointY-triangulos[i].e2->v1->y)*
                (triangulos[i].e2->v2->x-triangulos[i].e2->v1->x);
            detE3 = (newPointX-triangulos[i].e3->v1->x)*
                (triangulos[i].e3->v2->y-triangulos[i].e3->v1->y)-
                (newPointY-triangulos[i].e3->v1->y)*
                (triangulos[i].e3->v2->x-triangulos[i].e3->v1->x);
            if (detE1 == 0 && detE2 < 0 && detE3 < 0) {
                tempVertex = malloc(sizeof(vertex));
                *tempVertex = *triangulos[i].e1->v1;
                triangulos[i].e1 = &(edge) {
                    .v1=&newPoints[numPoints],
                    .v2=triangulos[i].e1->v2
                };
                triangulos[i].e3 = &(edge) {
                    .v1=triangulos[i].e2->v2,
                    .v2=triangulos[i].e1->v1
                };
                triangulos[offsetTriangle].e1 = &(edge) {
                    .v1=tempVertex,
                    .v2=triangulos[i].e1->v1
                };
                triangulos[offsetTriangle].e2 = &(edge) {
                    .v1=triangulos[offsetTriangle].e1->v2,
                    .v2=triangulos[i].e2->v2
                };
                triangulos[offsetTriangle].e3 = &(edge) {
                    .v1=triangulos[offsetTriangle].e2->v2,
                    .v2=triangulos[offsetTriangle].e1->v1
                };
                offsetTriangle++;
            } else if(detE1 < 0 && detE2 == 0 && detE3 < 0) {
                
            } else if(detE1 < 0 && detE2 < 0 && detE3 == 0) {

            } else if(detE1 < 0 && detE2 < 0 && detE3 < 0) {
                
            } else {
                continue;
            }
            numPoints++;
        }
    }
    fclose(fpInput);
    
    fpOutput = fopen(fileOutput, "w");
    for(i=0; i<offsetTriangle; i++) {
        fprintf(fpOutput, "%f %f\n", triangulos[i].e1->v1->x, triangulos[i].e1->v1->y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e1->v2->x, triangulos[i].e1->v2->y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e2->v1->x, triangulos[i].e2->v1->y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e2->v2->x, triangulos[i].e2->v2->y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e3->v1->x, triangulos[i].e3->v1->y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e3->v2->x, triangulos[i].e3->v2->y);
    }
    fprintf(fpOutput, "\n\n");
    for(i=0; i<numPoints; i++) {
        fprintf(fpOutput, "%f %f\n", newPoints[i].x, newPoints[i].y);
    }
    fclose(fpOutput);
    /*
    for(i=offsetTriangle-1; i!=0; i--) {
        free(triangulos[i].e1->v1);
        free(triangulos[i].e1->v2);
        free(triangulos[i].e2->v1);
        free(triangulos[i].e2->v2);
        free(triangulos[i].e3->v1);
        free(triangulos[i].e3->v2);
    }
    */
    return 0;
}
