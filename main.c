#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "det.h"

#define LARGO_MALLA 512
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
    for (i=0; i<LARGO_MALLA; i++) {
        for (j=0; j<3; j++) {
            triangles[i].vertices[j] = NULL;
            triangles[i].next[j] = NULL;
        }
    }
    for (i=0; i<LARGO_MALLA+2; i++) {
        vertices[i] = (Vertex){.x=-1,.y=-1};
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

float circleTest(Triangle *original, Triangle *next, int sharedV1, int sharedV2) {
    int i;
    float det, **mat4x4;
    Vertex *opposite;

    if (next != NULL) {
        for(i=0; i<3; i++) {
            /* rescate vertice opuesto */
            if (next->vertices[i] != original->vertices[sharedV1] &&
                next->vertices[i] != original->vertices[sharedV2]) {
                printf("encontre el vertice opuesto de indice %d\n", i);
                opposite = next->vertices[i];
                break;
            }
        }
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
    int i, j, nextSharedV1=-1, nextSharedV2=-1, nextOppositeV3=-1;
    Vertex *opposite;
    Triangle *farNext2 = NULL, *farNext1 = NULL;

    if (next != NULL) {
        for(i=0; i<3; i++) {
            /* rescate vertice opuesto */
            if (next->vertices[i] != original->vertices[sharedV1] &&
                next->vertices[i] != original->vertices[sharedV2]) {
                printf("encontre el vertice opuesto de indice %d (otra vez???) !\n", i);
                opposite = next->vertices[i];
                nextOppositeV3 = i;
                break;
            }
        }
        /* actualizar 2do vecino lejano */
        for(i=0; i<3; i++) {
            if (next->vertices[i] == original->vertices[sharedV2]) {
                nextSharedV2 = i;
                /*
                si existe tal vecino lejano, cambiar vecino del vertice
                opuesto (para no tener drama en el futuro
                */
                if(next->next[i] != NULL) {
                    farNext2 = next->next[i];
                    for(j=0; j<3; j++) {
                        if( next->next[i]->vertices[j] != opposite &&
                            next->next[i]->vertices[j] != original->vertices[sharedV1]) {
                                next->next[i]->next[j] = original;
                                printf("vecino lejano actualizado\n");
                                break;
                        }
                    }
                }
                break;
            }
        }
        /* este dato lo usaremos despues para actualizar vecindario local */
        for(i=0; i<3; i++) {
            if (next->vertices[i] == original->vertices[sharedV1]) {
                nextSharedV1 = i;
                break;
            }
        }
        /* actualizar 1er vecino lejano */
        farNext1 = original->next[sharedV1];
        if (original->next[sharedV1] != NULL) {
            for(i=0; i<3; i++) {
                if (original->next[sharedV1]->vertices[i] != original->vertices[sharedV2] &&
                    original->next[sharedV1]->vertices[i] != original->vertices[oppositeV3]) {
                    original->next[sharedV1]->next[i] = next;
                }
                break;
            }
        }
        /* error check */
        if (nextSharedV1==-1 || nextSharedV2 == -1 || oppositeV3 == -1) {
            printf("noo... problemas! :'-(\n");
            return;
        }
        /* cambiar la diagonal */
        original->vertices[sharedV2] = opposite;
        next->vertices[nextSharedV1] = original->vertices[oppositeV3];
        /* actualizar vecindario local */
        original->next[oppositeV3] = farNext2;
        original->next[sharedV1] = next;
        /* original->next[sharedV2] no requiere ser actualizado */
        next->next[nextOppositeV3] = farNext1;
        next->next[nextSharedV2] = original;
        /* original->next[sharedV1] no requiere ser actualizado */
    }
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
    int i, j, k, l, numTrs, numVs, idBorde,
        idVerticeOpuesto, idVerticeCompartido1,
        idVerticeCompartido2, idVerticeOpuestoVecino,
        idVertice1Vecino, idVertice2Vecino;

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

    printf("Se inicializa la malla\n");
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
        printf("Se lee nuevo punto: %.2f %.2f\n",x, y);
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
                printf("El punto está en el triangulo %d, en %p\n", i, &triangles[i]);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                /*
                Se crea el triángulo "b"
                ǁ\
                ||\
                || \
                | | \
                |a | \
                |  X c\
                | /  \ \
                |/__b__\\ 
                */
                printf("Se crea triangulo 'b', id %d en %p\n", numTrs, &triangles[numTrs]);
                triangles[numTrs].vertices[0] = &vertices[numVs];
                triangles[numTrs].vertices[1] = triangles[i].vertices[1];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                triangles[numTrs].next[0] = triangles[i].next[0];

                /*
                Si en dirección opuesta al vértice 0 del triángulo "i"
                existe un vecino, entonces se le asigna ese mismo al nuevo
                triángulo "triangles[numTrs]"
                Al mismo tiempo, se asigna desde ese vecino a triangles[numTrs]
                como vecino opuesto cuyo vértice no comparte con el triángulo "i"
                */
                if (triangles[numTrs].next[0] != NULL) {
                    for(j=0; j<3; j++) {
                        if (triangles[numTrs].next[0]->vertices[j] != triangles[numTrs].vertices[1] && 
                            triangles[numTrs].next[0]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("'b' %d en %p, tiene vecino %p con vertice opuesto en %d\n", numTrs, &triangles[numTrs], triangles[numTrs].next[0], j);
                                triangles[numTrs].next[0]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;

                /*
                Se crea el triángulo "c"
                */
                printf("Se crea triangulo 'c', id %d en %p\n", numTrs, &triangles[numTrs]);
                triangles[numTrs].vertices[0] = triangles[i].vertices[0];
                triangles[numTrs].vertices[1] = &vertices[numVs];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                triangles[numTrs].next[1] = triangles[i].next[1];

                /*
                Si el triángulo "i" tiene vecino opuesto al vértice 1,
                entonces ese vecino será el vecino del vértice 1 de
                triangles[numTrs]
                */
                if (triangles[numTrs].next[1] != NULL) {
                    for(j=0; j<3; j++) {
                        if (triangles[numTrs].next[1]->vertices[j] != triangles[numTrs].vertices[0] && 
                            triangles[numTrs].next[1]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("'c' %d en %p, tiene vecino %p con vertice opuesto en %d\n", numTrs, &triangles[numTrs], triangles[numTrs].next[1], j);
                                triangles[i].next[1]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;

                /*
                El triángulo "i" toma la forma de "a"
                */
                triangles[i].vertices[2] = &vertices[numVs];
                /*
                , y se actualizan los vecinos obsoletos
                a estas alturas,
                    a = triangles[i]
                    b = triangles[numTrs - 2]
                    c = triangles[numTrs - 1]
                */
                triangles[numTrs - 1].next[0] = &triangles[numTrs - 2];
                /*triangles[numTrs - 1].next[0] esta listo de antes*/
                triangles[numTrs - 1].next[2] = &triangles[i];
                /*triangles[numTrs - 2].next[0] esta listo de antes*/
                triangles[numTrs - 2].next[1] = &triangles[numTrs - 1];
                triangles[numTrs - 2].next[2] = &triangles[i];
                triangles[i].next[0] = &triangles[numTrs - 2];
                triangles[i].next[1] = &triangles[numTrs - 1];
                /*triangles[i].next[2] conserva su valor original*/

                /*
                Se aplica el test del circulo a los nuevos 3 triangulos
                (2 nuevos y uno modificado), siempre q existan los vecinos
                externos
                */
                /* se hace el test del circulo con  triangulo 'a' y su próximo vecino [2]*/
                if (triangles[i].next[2]!=NULL) {
                    det = circleTest(&triangles[i], triangles[i].next[2], 0, 1);
                    if (det > 0) {
                        /*
                        si no pasa el test del circulo, se hace el cambio de diagonal
                        tr original:           triangles[i]
                        tr proximo:            triangles[i].next[2]
                        tr vecino lejano 1:    triangles[i].next[0]
                        tr vecino lejano 2:    es el vecino que comparte triangles[i].vertices[0] con triangles[i].next[2]
                        */
                        printf("cambio de diag 'a' de %p y vecino %p\n", &triangles[i], triangles[i].next[2]);
                        changeDiag(&triangles[i], triangles[i].next[2], 0, 1, 2);
                    }
                }
                /* se hace el test del circulo con  triangulo 'b' y su próximo */
                if (triangles[numTrs - 2].next[0]!=NULL) {
                    det = circleTest(&triangles[numTrs - 2], triangles[numTrs - 2].next[0], 1, 2);
                    if (det > 0) {
                        printf("cambio de diag 'b' de %p y vecino %p\n", &triangles[numTrs - 2], triangles[numTrs - 2].next[0]);
                        changeDiag(&triangles[numTrs - 2], triangles[numTrs - 2].next[0], 1, 2, 0);
                    }
                }
                /* se hace el test del circulo con  triangulo 'c' y su próximo */
                if (triangles[numTrs - 1].next[1]!=NULL) {
                    det = circleTest(&triangles[numTrs - 1], triangles[numTrs - 1].next[1], 0, 2);
                    if (det > 0) {
                        printf("cambio de diag 'c' de %p y vecino %p\n", &triangles[numTrs - 1], triangles[numTrs - 1].next[1]);
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
            idBorde es un indice que permite conocer en la arista del triangulo
            donde cae el nuevo punto. Es sólo para depurar
            idVerticeOpuesto es el numero de indice del vertice opuesto a la arista
            donde cae el nuevo punto
            idVerticeCompartido1 y idVerticeCompartido2 son los vertices restantes
            del triángulo "i" en dirección contra-reloj
            */
            idBorde = -1;
            if (dets[0] == 0 && dets[1] > 0 && dets[2] > 0) {
                idBorde = 1;
                idVerticeOpuesto = 2;
                idVerticeCompartido1 = 0;
                idVerticeCompartido2 = 1;
            } else if (dets[0] > 0 && dets[1] == 0 && dets[2] > 0) {
                idBorde = 2;
                idVerticeOpuesto = 0;
                idVerticeCompartido1 = 1;
                idVerticeCompartido2 = 2;
            } else if (dets[0] > 0 && dets[1] > 0 && dets[2] == 0) {
                idBorde = 3;
                idVerticeOpuesto = 1;
                idVerticeCompartido1 = 0;
                idVerticeCompartido2 = 2;
            }
            if (idBorde>0) {
                printf("punto encontrado en triangulo %d, en borde %d\n", i, idBorde);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                /*
                se crea nuevo triangulo dentro de "i", con v0 en el nuevo punto
                */
                triangles[numTrs].vertices[0] = &vertices[numVs];
                triangles[numTrs].vertices[1] = triangles[i].vertices[idVerticeCompartido2];
                triangles[numTrs].vertices[2] = triangles[i].vertices[idVerticeOpuesto];
                /*
                se actualiza la forma de "i"
                */
                triangles[i].vertices[idVerticeCompartido2] = &vertices[numVs];
                /*
                se actualiza la vecindad local
                */
                farNext1 = triangles[i].next[idVerticeCompartido1];
                triangles[i].next[idVerticeCompartido1] = &triangles[numTrs];
                /* triangles[i].next[idVerticeCompartido2] no cambia! */
                /* triangles[i].next[idVerticeOpuesto] en espera! ver [5]*/
                triangles[numTrs].next[0] = farNext1;
                triangles[numTrs].next[1] = &triangles[i];
                triangles[numTrs].next[2] = NULL;
                numTrs++;
                /*
                [5] si es que el triangulo "i" tenia vecino opuesto en idVerticeOpuesto
                se sigue trabajando! 
                */
                if (triangles[i].next[idVerticeOpuesto] != NULL) {
                    opposite = NULL;
                    farNext2 = NULL;
                    idVertice2Vecino = -1;
                    idVertice1Vecino = -1;
                    idVerticeOpuestoVecino = -1;
                    /*
                    se busca el vertice opuesto en el vecino opuesto
                    almacenamos el id y una referencia a ese vertice
                    */
                    for(j=0; j<3; j++) {
                        if (triangles[i].next[idVerticeOpuesto]->vertices[j] !=
                            triangles[i].vertices[idVerticeCompartido1] &&
                            triangles[i].next[idVerticeOpuesto]->vertices[j] !=
                            triangles[numTrs-1].vertices[1]) {
                            idVerticeOpuestoVecino = j;
                            opposite = triangles[i].next[idVerticeOpuesto]->vertices[j];
                        }
                    }
                    if (opposite==NULL) {
                        printf("algo anda mal... lo siento mucho!\n");
                        return;
                    }
                    /* se crea un segundo triangulo */
                    triangles[numTrs].vertices[0] = opposite;
                    triangles[numTrs].vertices[1] = triangles[numTrs-1].vertices[1];
                    triangles[numTrs].vertices[2] = &vertices[numVs];
                    /* se ajusta la forma del vecino de "i" */
                    for(j=0; j<3; j++) {
                        if (triangles[i].next[idVerticeOpuesto]->vertices[j] == triangles[numTrs].vertices[1]) {
                            triangles[i].next[idVerticeOpuesto]->vertices[j] = &vertices[numVs];
                            idVertice1Vecino = j;
                            printf("arreglando al vecino!\n");
                            break;
                        }
                    }
                    /* se rescata el segundo vecino lejano*/
                    for(j=0; j<3; j++) {
                        if (triangles[i].next[idVerticeOpuesto]->vertices[j] == triangles[i].vertices[idVerticeCompartido1]) {
                            printf(".\n");
                            farNext2 = triangles[i].next[idVerticeOpuesto]->next[j];
                            idVertice2Vecino = j;
                            if (triangles[i].next[idVerticeOpuesto]->next[j] != NULL) {
                                for(k=0; k<3; k++) {
                                    if (triangles[i].next[idVerticeOpuesto]->next[j]->vertices[k] != triangles[numTrs].vertices[0] &&
                                    triangles[i].next[idVerticeOpuesto]->next[j]->vertices[k] != triangles[numTrs].vertices[1]) {
                                        triangles[i].next[idVerticeOpuesto]->next[j]->next[k] = &triangles[numTrs];
                                        printf("vecino lejano actualizado!\n");
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if (idVertice1Vecino == -1 || idVertice2Vecino == -1 || idVerticeOpuestoVecino == -1) {
                        printf("algo anda muuy mal... lo siento mucho!\n");
                    }
                    /* se debe hacer una nueva actualizacion de vecinos en este nuevo escenario */
                    triangles[numTrs].next[0] = &triangles[numTrs-1];
                    triangles[numTrs].next[1] = triangles[i].next[idVerticeOpuesto];
                    triangles[numTrs].next[2] = farNext2;
                    triangles[numTrs-1].next[2] = &triangles[numTrs];

                    triangles[i].next[idVerticeOpuesto]->next[idVerticeOpuestoVecino] = &triangles[i];
                    /*
                    triangles[i].next[idVerticeOpuesto]->next[idVertice1Vecino] no requiere actualizarse
                    */
                    triangles[i].next[idVerticeOpuesto]->next[idVertice2Vecino] = &triangles[numTrs];
                    numTrs++;
                }
                /* test del circulo en triangulos */
                /*
                circleTest(&triangles[i], )
                
                .next[idVerticeOpuesto]
                triangles[numTrs];
                triangles[numTrs-1];
                */
                numVs++;
                break;
            }
        }
        printf(".");
    }
    printf("\n");
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
