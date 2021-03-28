#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct triangle;
const LARGO_MALLA = 10;

typedef struct {
    float x, y;
} vertex;

typedef struct {
    vertex *v1, *v2;
    struct triangle *vecino;
} edge;

typedef struct {
    edge *e1, *e2, *e3;
} triangle;

triangle* initialize(float x1, float y1, float x2, float y2) {
    triangle *net = malloc(LARGO_MALLA * sizeof *net);
    net[0] = (triangle){
        &(edge){&(vertex){x1,y1},&(vertex){x2,y1}},
        &(edge){&(vertex){x2,y1},&(vertex){x2,y2}},
        &(edge){&(vertex){x2,y2},&(vertex){x1,y2}}};;
    net[1] = (triangle){
        &(edge){&(vertex){x1,y2},&(vertex){x1,y2}},
        &(edge){&(vertex){x2,y1},&(vertex){x2,y2}},
        &(edge){&(vertex){x2,y2},&(vertex){x1,y2}}};;
    return net;
}

int main()
{
    triangle* net;
    FILE *fp;
    char fileBuffer[255];
    float newPointX, newPointY;
    int i;

    printf(".::el triangulator::. v0.2\n");
    net = initialize(0,0,4,4);

    fp = fopen("./puntos.txt", "r"); {}
    if(!fp) {
        printf("No pude encontrar el archivo\n");
    }
    while(!feof(fp)) {
        fscanf(fp, "%f %f", &newPointX, &newPointY);
        /*printf("%f %f\n", newPointX, newPointY);*/
    }
    for(i=0;i<2;i++) {
        printf("%f %f\n", net[i].e1->v1->x, net[i].e1->v1->y);
    }
    return 0;
}
