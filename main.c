#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "det.h"

#define LARGO_MALLA 1024
#define TAMANO_MALLA_X 10
#define TAMANO_MALLA_Y 10

/*
Representación de un vértice, con sus puntos x e y
del plano cartesiano
*/
typedef struct Vertex {
    float x, y;
} Vertex;

/*
Representación de un triángulo, compuesto por un
vector de 3 vértices y 3 vecinos. Por convención,
el vecino i es el vecino opuesto al vértice i
*/
typedef struct Triangle {
     Vertex* vertices[3];
     struct Triangle* next[3];
} Triangle;

void initMesh(Triangle *triangles, Vertex *vertices) {
    int i, j;
    for (i=0; i<LARGO_MALLA+2; i++) {
        vertices[i] = (Vertex){.x=-1,.y=-1};
    }
    for (i=0; i<LARGO_MALLA; i++) {
        for (j=0; j<3; j++) {
            triangles[i].vertices[j] = NULL;
            triangles[i].next[j] = NULL;
        }
    }
    vertices[0].x = 0;
    vertices[0].y = TAMANO_MALLA_Y;
    vertices[1].x = 0;
    vertices[1].y = 0;
    vertices[2].x = TAMANO_MALLA_X;
    vertices[2].y = 0;
    vertices[3].x = TAMANO_MALLA_X;
    vertices[3].y = TAMANO_MALLA_Y;
    triangles[0].vertices[0] = &vertices[0];
    triangles[0].vertices[1] = &vertices[1];
    triangles[0].vertices[2] = &vertices[2];
    triangles[1].vertices[0] = &vertices[0];
    triangles[1].vertices[1] = &vertices[2];
    triangles[1].vertices[2] = &vertices[3];
    triangles[0].next[1] = &triangles[1];
    triangles[1].next[2] = &triangles[0];
}

void calcDets2x2Matrix(float **mat2x2, Triangle *t, float x, float y, float *dets) {
    mat2x2[0][0] = t->vertices[1]->x - t->vertices[0]->x;
    mat2x2[0][1] = x                           - t->vertices[0]->x;
    mat2x2[1][0] = t->vertices[1]->y - t->vertices[0]->y;
    mat2x2[1][1] = y                           - t->vertices[0]->y;
    dets[0] = determinant(mat2x2, 2);
    mat2x2[0][0] = t->vertices[2]->x - t->vertices[1]->x;
    mat2x2[0][1] = x                           - t->vertices[1]->x;
    mat2x2[1][0] = t->vertices[2]->y - t->vertices[1]->y;
    mat2x2[1][1] = y                           - t->vertices[1]->y;
    dets[1] = determinant(mat2x2, 2);
    mat2x2[0][0] = t->vertices[0]->x - t->vertices[2]->x;
    mat2x2[0][1] = x                           - t->vertices[2]->x;
    mat2x2[1][0] = t->vertices[0]->y - t->vertices[2]->y;
    mat2x2[1][1] = y                           - t->vertices[2]->y;
    dets[2] = determinant(mat2x2, 2);
}

void calcDets4x4Matrix(float **mat4x4, Triangle *t, float x, float y, float *det) {
    mat4x4[0][0] = t->vertices[0]->x;
    mat4x4[0][1] = t->vertices[0]->y;
    mat4x4[0][2] = pow(t->vertices[0]->x, 2) + pow(t->vertices[0]->y, 2);
    mat4x4[0][3] = 1;
    mat4x4[1][0] = t->vertices[1]->x;
    mat4x4[1][1] = t->vertices[1]->y;
    mat4x4[1][2] = pow(t->vertices[1]->x, 2) + pow(t->vertices[1]->y, 2);
    mat4x4[1][3] = 1;
    mat4x4[2][0] = t->vertices[2]->x;
    mat4x4[2][1] = t->vertices[2]->y;
    mat4x4[2][2] = pow(t->vertices[2]->x, 2) + pow(t->vertices[2]->y, 2);
    mat4x4[2][3] = 1;
    mat4x4[3][0] = x;
    mat4x4[3][1] = y;
    mat4x4[3][2] = pow(x, 2) + pow(y, 2);
    mat4x4[3][3] = 1;
    *det = determinant(mat4x4, 4);
}

/*
dado 1 vertices y un triangulo, retorna el id de ese vértice
en el triangulo
*/
int getVertexId(Triangle *triangle, Vertex *vertex) {
    int i;
    for(i=0; i<3; i++)
        if (triangle->vertices[i] == vertex)
            return i;
    return -1;
}

/*
dado 2 vertices y un triangulo, retorna el tercero de ese triangulo
*/
Vertex *getThirdVertex(Triangle *triangle, Vertex *vertex1, Vertex *vertex2) {
    int i;
    for(i=0; i<3; i++)
        if (triangle->vertices[i] != vertex1 && triangle->vertices[i] != vertex2)
            return triangle->vertices[i];
    return NULL;
}
/* dado 2 vertices y un triangulo, retorna el id del tercero de ese triangulo */
int getThirdVertexId(Triangle *triangle, Vertex *vertex1, Vertex *vertex2) {
    int i;
    for(i=0; i<3; i++)
        if (triangle->vertices[i] != vertex1 && triangle->vertices[i] != vertex2)
            return i;
    return -1;
}

float circleTest(Triangle *original, Triangle *next, int sharedV1, int sharedV2) {
    int i;
    float det, **mat4x4;
    Vertex *opposite;

    if (next != NULL) {
        opposite = getThirdVertex(next, original->vertices[sharedV1], original->vertices[sharedV2]);
        mat4x4 = calloc(4,sizeof(float*));
        for(i=0; i<4; i++) {
            mat4x4[i] = calloc(4,sizeof(float));
        }
        calcDets4x4Matrix(mat4x4, original, opposite->x, opposite->y, &det);
        for(i=0; i<4; i++) {
            free(mat4x4[i]);
        }
        free(mat4x4);
        return det;
    }
    return -1;
}

void changeDiag(Triangle *original, Triangle *next, int sharedV1, int sharedV2, int oppositeV3) {
    int i, j, nextSharedV1=-1, nextSharedV2=-1, nextOppositeV3=-1, vertexId;
    Vertex *opposite, *vertex;
    Triangle *farNextCopy = NULL;

    if (next != NULL) {
        /* opposite es el vertice opuesto del vecino */
        opposite = getThirdVertex(next, original->vertices[sharedV1], original->vertices[sharedV2]);
        if (opposite==NULL) {
            printf("error grave A");
            return;
        }
        /* se rescatan los id de los vertices del vecino */
        nextSharedV1 = getVertexId(next, original->vertices[sharedV1]);
        nextSharedV2 = getVertexId(next, original->vertices[sharedV2]);
        nextOppositeV3 = getThirdVertexId(next, original->vertices[sharedV1], original->vertices[sharedV2]);
        /* se actualiza el 2do vecino lejano en caso que lo tenga */
        if(next->next[nextSharedV2] != NULL) {
            vertexId = getThirdVertexId(next->next[nextSharedV2], opposite, original->vertices[sharedV1]);
            next->next[nextSharedV2]->next[vertexId] = original;
        }
        /* actualizamos el 1er vecino lejano */
        farNextCopy = original->next[sharedV1];
        if (original->next[sharedV1] != NULL) {
            vertex = getThirdVertex(original->next[sharedV1], original->vertices[sharedV2], original->vertices[oppositeV3]);
            vertexId = getVertexId(original->next[sharedV1], vertex);
            original->next[sharedV1]->next[vertexId] = next;
        }
        /* se cambia la diagonal */
        original->vertices[sharedV2] = opposite;
        next->vertices[nextSharedV1] = original->vertices[oppositeV3];
        /* actualizar vecindario local */
        original->next[oppositeV3] = next->next[nextSharedV2];
        original->next[sharedV1] = next;
        /* original->next[sharedV2] no requiere ser actualizado */
        next->next[nextOppositeV3] = farNextCopy;
        next->next[nextSharedV2] = original;
        /* original->next[sharedV1] no requiere ser actualizado */
    }
}

void pointInside(
    Triangle *triangleA,
    Triangle *triangleB,
    Triangle *triangleC,
    Vertex *currentVertex) {
    int i;
    /*
    Se crearán los triángulos b y c. Se reutiliza el a
    ǁ\
    ||\
    || \
    | | \
    |a | \
    |  X c\
    | /  \ \
    |/__b__\\ 
    */
    triangleB->vertices[0] = currentVertex;
    triangleB->vertices[1] = triangleA->vertices[1];
    triangleB->vertices[2] = triangleA->vertices[2];
    triangleB->next[0] = triangleA->next[0];
    /*
    Si en dirección opuesta al vértice 0 del triángulo "i"
    existe un vecino, entonces se le asigna ese mismo al nuevo
    triángulo "triangles[numTrs]"
    Al mismo tiempo, se asigna desde ese vecino a triangles[numTrs]
    como vecino opuesto cuyo vértice no comparte con el triángulo "i"
    */
    if (triangleB->next[0] != NULL) {
        for(i=0; i<3; i++) {
            if (triangleB->next[0]->vertices[i] != triangleB->vertices[1] && 
                triangleB->next[0]->vertices[i] != triangleB->vertices[2]) {
                    triangleB->next[0]->next[i] = triangleB;
                }
        }
    }
    /*
    Se crea el triángulo "c"
    */
    triangleC->vertices[0] = triangleA->vertices[0];
    triangleC->vertices[1] = currentVertex;
    triangleC->vertices[2] = triangleA->vertices[2];
    triangleC->next[1] = triangleA->next[1];

    /*
    Si el triángulo "i" tiene vecino opuesto al vértice 1,
    entonces ese vecino será el vecino del vértice 1 de
    triangles[numTrs]
    */
    if (triangleC->next[1] != NULL) {
        for(i=0; i<3; i++) {
            if (triangleC->next[1]->vertices[i] != triangleC->vertices[0] && 
                triangleC->next[1]->vertices[i] != triangleC->vertices[2]) {
                    triangleA->next[1]->next[i] = triangleC;
                }
        }
    }
    /*
    El triángulo "i" toma la forma de "a"
    */
    triangleA->vertices[2] = currentVertex;
    /*
    , y se actualizan los vecinos obsoletos
    */
    triangleC->next[0] = triangleB;
    /*trianglesC->next[0] esta listo de antes*/
    triangleC->next[2] = triangleA;
    /*trianglesB->next[0] esta listo de antes*/
    triangleB->next[1] = triangleC;
    triangleB->next[2] = triangleA;
    triangleA->next[0] = triangleB;
    triangleA->next[1] = triangleC;
    /*trianglesA->next[2] conserva su valor original*/
}

void pointOnEdge(
    Triangle *triangleA,
    Triangle *triangleB,
    int idVerticeOpuesto,
    int idVerticeCompartido1,
    int idVerticeCompartido2,
    Vertex *vertice) {
    int i;
    /*
    Se crea el triángulos b. Se reutiliza el a
    ǁ\
    ǁ \
    || \
    | | \
    | |  \
    |a | b\
    |  |   \
    |___X___\
    */
    triangleB->vertices[0] = vertice;
    triangleB->vertices[1] = triangleA->vertices[idVerticeOpuesto];
    triangleB->vertices[2] = triangleA->vertices[idVerticeCompartido2];
    triangleA->vertices[idVerticeCompartido2] = vertice;
    /* se actualizan los vecinos */
    if (triangleA->next[idVerticeCompartido1] != NULL) {
        triangleA->next[idVerticeCompartido1]->next[getThirdVertexId(
            triangleA->next[idVerticeCompartido1],
            triangleB->vertices[1],
            triangleB->vertices[2]
        )];
    }
    triangleB->next[2] = triangleA;
    triangleB->next[0] = triangleA->next[idVerticeCompartido1];
    triangleA->next[idVerticeCompartido1] = triangleB;
}

void debugTriangle(Triangle *triangles) {
    int i;
    printf("%.2f,%.2f %.2f,%.2f %.2f,%.2f vecinos: %p %p %p en %p\n",
        triangles->vertices[0]->x, triangles->vertices[0]->y,
        triangles->vertices[1]->x, triangles->vertices[1]->y,
        triangles->vertices[2]->x, triangles->vertices[2]->y,
        triangles->next[0], triangles->next[1], triangles->next[2],
        triangles);
}

void printTriangles(Triangle *triangles, int numTrs) {
    int i;
    for(i=0; i<numTrs;i++) {
        printf("t%d: %.2f,%.2f %.2f,%.2f %.2f,%.2f vecinos: %p %p %p en %p\n",
            i,triangles[i].vertices[0]->x, triangles[i].vertices[0]->y,
            triangles[i].vertices[1]->x, triangles[i].vertices[1]->y,
            triangles[i].vertices[2]->x, triangles[i].vertices[2]->y,
            triangles[i].next[0], triangles[i].next[1], triangles[i].next[2],
            &triangles[i]);
    }
}

int main()
{
    /*
    Punteros a archivos de entrada y salida, tanto
    para leer los datos, como para escribir los
    resultados. Estos últimos pueden ser "ploteados"
    por gnuplot gracias a un script adjunto
    */
    FILE *fpInput, *fpOutput;
    char *fileInput="puntos.txt",
        *fileOutput="salida.txt";
    /*
    Se inicializan vectores de triángulos y vértices
    de manera estática
    */
    Triangle triangles[LARGO_MALLA], *farNext1, *farNext2;
    Vertex vertices[LARGO_MALLA+2], *opposite;
    /*
    x e y son las coordenadas x e y que se leen
    del archivo de puntos. Dets es un vector
    de determinantes para saber donde esta cada
    nuevo punto en la malla. Las matrices mat2x2 y
    mat4x4 se usan para calcular los determinantes.
    numTrs es el número total de triángulos
    numVs es el número total de vectores
    i, j, k y l son índices genéricos
    */
    float x, y, det, dets[3], **mat2x2, **mat4x4;
    int i, j, k, l, numTrs, numVs, caeEnBorde,
        /*
        idVerticeOpuesto es el indice del vector opuesto a un vecino
        idVerticeCompartido1 es el indice del primer vector que comparte con un vecino
        idVerticeCompartido2 es el indice del segundo vector que comparte con un vecino
        */
        idVerticeOpuesto,
        idVerticeCompartido1,
        idVerticeCompartido2,
        /*
        Los mismo indices, para el vecino del triangulo actual
        */
        idVerticeOpuestoVecino,
        idVertice1Vecino,
        idVertice2Vecino;

    /*
    Se reserva memoria para las matrices que
    ayudarán al cálculo de las determinantes
    */
    mat2x2 = calloc(2,sizeof(float*));
    for(i=0; i<2; i++) {
        mat2x2[i] = calloc(2,sizeof(float));
    }
    /*
    Se inicializa la malla con 2 triángulos
    arbitrarios
    */
    numTrs = 2;
    numVs = 4;

    initMesh(triangles, vertices);

    /*
    Se leen los puntos desde el archivo puntos.txt
    */
    fpInput = fopen(fileInput, "r");
    if(!fpInput) {
        printf("No pude abrir el archivo puntos.txt\n");
        return 1;
    }
    while(!feof(fpInput)) {
        fscanf(fpInput, "%f %f", &x, &y);
        for(i=0; i<numTrs; i++) {
            /*
            Se determina donde esta el punto dentro de la malla de triángulos
            a través del cálculo del determinante de cada lado con el nuevo
            punto
            */
            calcDets2x2Matrix(mat2x2, &triangles[i], x, y, dets);
            /*
            Caso en que el punto cae dentro de un triángulo "i"
            0
            |\
            | \
            |  \
            |   \
            |    \
            |  X  \
            |      \
            |_______\
            1        2
            */
            if (dets[0] > 0 && dets[1] > 0 && dets[2] > 0) {
                vertices[numVs] = (Vertex){.x=x, .y=y};
                pointInside(&triangles[i], &triangles[numTrs], &triangles[numTrs+1], &vertices[numVs]);
                numTrs+=2;
                /*
                Se aplica el test del circulo a los nuevos 3 triangulos
                (2 nuevos y uno modificado), siempre q existan los vecinos
                externos
                */
                /* se hace el test del circulo con  triangulo 'a' y su próximo vecino [2]*/
                if (triangles[i].next[2]!=NULL) {
                    det = circleTest(&triangles[i], triangles[i].next[2], 0, 1);
                    if (det > 0) {
                        changeDiag(&triangles[i], triangles[i].next[2], 0, 1, 2);
                    }
                }
                /* se hace el test del circulo con  triangulo 'b' y su próximo */
                if (triangles[numTrs - 2].next[0]!=NULL) {
                    det = circleTest(&triangles[numTrs - 2], triangles[numTrs - 2].next[0], 1, 2);
                    if (det > 0) {
                        changeDiag(&triangles[numTrs - 2], triangles[numTrs - 2].next[0], 1, 2, 0);
                    }
                }
                /* se hace el test del circulo con  triangulo 'c' y su próximo */
                if (triangles[numTrs - 1].next[1]!=NULL) {
                    det = circleTest(&triangles[numTrs - 1], triangles[numTrs - 1].next[1], 0, 2);
                    if (det > 0) {
                        changeDiag(&triangles[numTrs - 1], triangles[numTrs - 1].next[1], 0, 2, 1);
                    }
                }
                /*
                Se considera que hay un nuevo vértice en la ejecución
                */
                numVs++;
                break;
            }
            /*
            Caso en que el punto cae en un borde
            0
            |\
            | \
            |  \
            |   \
            x    x
            |  i  \
            |      \
            |__x____\
            1        2

            donde:
            caeEnBorde es un indice que permite conocer en la arista del triangulo
            donde cae el nuevo punto. Es sólo para depurar
            idVerticeOpuesto es el numero de indice del vertice opuesto a la arista
            donde cae el nuevo punto
            idVerticeCompartido1 y idVerticeCompartido2 son los vertices restantes
            del triángulo "i" en dirección contra-reloj
            */
            caeEnBorde = -1;
            if (dets[0] == 0 && dets[1] > 0 && dets[2] > 0) {
                caeEnBorde = 1;
                idVerticeOpuesto = 2;
                idVerticeCompartido1 = 1;
                idVerticeCompartido2 = 0;
            } else if (dets[0] > 0 && dets[1] == 0 && dets[2] > 0) {
                caeEnBorde = 2;
                idVerticeOpuesto = 0;
                idVerticeCompartido1 = 1;
                idVerticeCompartido2 = 2;
            } else if (dets[0] > 0 && dets[1] > 0 && dets[2] == 0) {
                caeEnBorde = 3;
                idVerticeOpuesto = 1;
                idVerticeCompartido1 = 0;
                idVerticeCompartido2 = 2;
            }
            if (caeEnBorde > 0) {
                continue;
                printf("punto cae en borde %d de %p\n", caeEnBorde, &triangles[i]);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                /* el punto cae en borde compartido con 2 triangulos */
                if(triangles[i].next[idVerticeOpuesto] != NULL) {
                    /* se regulariza si hay vecino frente al vértice opuesto */
                    if (caeEnBorde==3) {
                        idVertice1Vecino = getVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido2]);
                        idVertice2Vecino = getVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido1]);
                        idVerticeOpuestoVecino = getThirdVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido1], triangles[i].vertices[idVerticeCompartido2]);
                    } else {
                        idVertice1Vecino = getVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido1]);
                        idVertice2Vecino = getVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido2]);
                        idVerticeOpuestoVecino = getThirdVertexId(triangles[i].next[idVerticeOpuesto], triangles[i].vertices[idVerticeCompartido1], triangles[i].vertices[idVerticeCompartido2]);
                    }
                    pointOnEdge(
                        triangles[i].next[idVerticeOpuesto],
                        &triangles[numTrs],
                        idVerticeOpuestoVecino,
                        idVertice1Vecino,
                        idVertice2Vecino,
                        &vertices[numVs]);
                    numTrs++;
                    pointOnEdge(
                        &triangles[i],
                        &triangles[numTrs],
                        idVerticeOpuesto,
                        idVerticeCompartido1,
                        idVerticeCompartido2,
                        &vertices[numVs]);
                    numTrs++;
                    /* se hace la ultima actualizacion del vecindario local (no pude dejarlo mas bonito :-/) */
                    triangles[numTrs-1].next[1] = triangles[i].next[idVerticeOpuesto];
                    triangles[numTrs-2].next[1] = &triangles[i];
                    triangles[i].next[idVerticeOpuesto]->next[idVerticeOpuestoVecino] = &triangles[numTrs-1];
                    triangles[i].next[idVerticeOpuesto] = &triangles[numTrs-2];
                    /* se aplica el test de círculo para los 4 triangulos involucrados y los
                    vertices opuestos de los vecinos externos */
                    /* i */
                    if(triangles[i].next[idVerticeCompartido2] != NULL) {
                        det = circleTest(&triangles[i], triangles[i].next[idVerticeCompartido2], idVerticeCompartido1, idVerticeOpuesto);
                        printf("A: %.2f\n", det);
                        if (det > 0) {
                            changeDiag(&triangles[i], triangles[i].next[idVerticeCompartido2], idVerticeCompartido1, idVerticeOpuesto, idVerticeCompartido2);
                        }
                    }
                    /* vecino de i */
                    if(triangles[i].next[idVerticeOpuesto]->next[idVertice2Vecino] != NULL) {
                        det = circleTest(triangles[i].next[idVerticeOpuesto], triangles[i].next[idVerticeOpuesto]->next[idVertice2Vecino], idVertice1Vecino, idVerticeOpuestoVecino);
                        printf("B: %.2f\n", det);
                        if (det > 0) {
                            changeDiag(triangles[i].next[idVerticeOpuesto], triangles[i].next[idVerticeOpuesto]->next[idVertice2Vecino], idVertice1Vecino, idVerticeOpuestoVecino, idVertice2Vecino);
                        }
                    }
                    /* nuevo 1 */
                    if(triangles[numTrs-1].next[0] != NULL) {
                        det = circleTest(&triangles[numTrs-1], triangles[numTrs-1].next[0], 1, 2);
                        printf("C: %.2f\n", det);
                        if (det > 0) {
                            printf("C\n");
                            changeDiag(&triangles[numTrs-1], triangles[numTrs-1].next[0], 1, 2, 0);
                        }
                    }
                    /* nuevo 2 */
                    if(triangles[numTrs-2].next[0] != NULL) {
                        det = circleTest(&triangles[numTrs-2], triangles[numTrs-2].next[0], 1, 2);
                        printf("D: %.2f\n", det);
                        if (det > 0) {
                            changeDiag(&triangles[numTrs-2], triangles[numTrs-2].next[0], 1, 2, 0);
                        }
                    }

                } else {
                    /* actualizacion prematura de vecinos */
                    if (triangles[i].next[idVerticeCompartido1] != NULL) {
                        triangles[numTrs].next[0] = &triangles[i];
                        idVerticeOpuestoVecino = getThirdVertexId(
                            triangles[i].next[idVerticeCompartido1],
                            triangles[i].vertices[idVerticeCompartido1],
                            triangles[i].vertices[idVerticeOpuesto]
                        );
                    }
                    pointOnEdge(
                        &triangles[i],
                        &triangles[numTrs],
                        idVerticeOpuesto,
                        idVerticeCompartido1,
                        idVerticeCompartido2,
                        &vertices[numVs]);
                    numTrs++;
                    /* test del circulo */
                    if(triangles[i].next[idVerticeCompartido2] != NULL) {
                        det = circleTest(&triangles[i], triangles[i].next[idVerticeCompartido2], idVerticeCompartido1, idVerticeOpuesto);
                        printf("E: %.2f\n", det);
                        if (det > 0) {
                            changeDiag(&triangles[i], triangles[i].next[idVerticeCompartido2], idVerticeCompartido1, idVerticeOpuesto, idVerticeCompartido2);
                        }
                    }
                    if(triangles[numTrs-1].next[0] != NULL) {
                        det = circleTest(&triangles[numTrs-1], triangles[numTrs-1].next[0], 1, 2);
                        if (det > 0) {
                            printf("F\n");
                            changeDiag(&triangles[numTrs-1], triangles[numTrs-1].next[0], 1, 2, 0);
                        }
                    }
                }

                numVs++;
                break;
            }
        }
    }
    printTriangles(triangles, numTrs);
    /*
    Se genera archivo para visualización en gnuplot
    */
    fpOutput = fopen(fileOutput, "w");
    for(i=0; i<numTrs;i++) {
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[0]->x, triangles[i].vertices[0]->y);
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[1]->x, triangles[i].vertices[1]->y);
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[2]->x, triangles[i].vertices[2]->y);
        fprintf(fpOutput,"%f %f\n\n",triangles[i].vertices[0]->x, triangles[i].vertices[0]->y);
    }
    fclose(fpOutput);

    for(i=0; i<2; i++) {
        free(mat2x2[i]);
    }
    free(mat2x2);
    return 0;
}
