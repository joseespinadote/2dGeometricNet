#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "det.h"

#define LARGO_MALLA 32
#define TAMANO_BUFFER 64
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

int main()
{
    /*
    Punteros a archivos de entrada y salida, tanto
    para leer los datos, como para escribir los
    resultados. Estos últimos pueden ser "ploteados"
    por gnuplot gracias a un script adjunto
    */
    FILE *fpInput, *fpOutput;
    char fileBuffer[TAMANO_BUFFER],
        *fileInput="puntos.txt",
        *fileOutput="salida.txt";
    /*
    Se inicializan vectores de triángulos y vértices
    de manera estática
    */
    Triangle triangles[LARGO_MALLA];
    Vertex vertices[LARGO_MALLA+2];
    /*
    x e y son las coordenadas x e y que se leen
    del archivo de puntos. Dets es un vector
    de determinantes para saber donde esta cada
    nuevo punto en la malla. Las matrices mat2x2 y
    mat4x4 se usan para calcular los determinantes.
    numTrs es el número total de triángulos
    numVs es el número total de vectores
    i, j y k son índices genéricos
    */
    float x, y, det, dets[3], **mat2x2, **mat4x4;
    int i, j, k, numTrs, numVs, idBorde,
        idVerticeOpuesto, idVerticeCompartido1,
        idVerticeCompartido2;

    /*
    Se reserva memoria para las matrices que
    ayudarán al cálculo de las determinantes
    */
    mat2x2 = calloc(2,sizeof(float*));
    for(i=0; i<2; i++) {
        mat2x2[i] = calloc(2,sizeof(float));
    }
    mat4x4 = calloc(4,sizeof(float*));
    for(i=0; i<4; i++) {
        mat4x4[i] = calloc(4,sizeof(float));
    }
    /*
    Se inicializa la malla con 2 triángulos
    arbitrarios
    */
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
    triangles[0].next[0] = NULL;
    triangles[0].next[1] = &triangles[1];
    triangles[0].next[2] = NULL;
    triangles[1].next[0] = NULL;
    triangles[1].next[1] = NULL;
    triangles[1].next[2] = &triangles[0];
    numTrs = 2;
    numVs = 4;

    /*
    Se leen los puntos desde el archivo puntos.txt
    */
    fpInput = fopen(fileInput, "r");
    if(!fpInput) {
        printf("can't open file\n");
        return 1;
    }
    while(!feof(fpInput)) {
        fscanf(fpInput, "%f %f", &x, &y);
        printf("Nuevo punto: %f %f\n",x, y);        
        for(i=0; i<numTrs; i++) {
            /*
            Se determina donde esta el punto dentro de la malla de triángulos
            a través del cálculo del determinante de cada lado con el nuevo
            punto
            */
            mat2x2[0][0] = triangles[i].vertices[1]->x - triangles[i].vertices[0]->x;
            mat2x2[0][1] = x                           - triangles[i].vertices[0]->x;
            mat2x2[1][0] = triangles[i].vertices[1]->y - triangles[i].vertices[0]->y;
            mat2x2[1][1] = y                           - triangles[i].vertices[0]->y;
            dets[0] = determinant(mat2x2, 2);
            mat2x2[0][0] = triangles[i].vertices[2]->x - triangles[i].vertices[1]->x;
            mat2x2[0][1] = x                           - triangles[i].vertices[1]->x;
            mat2x2[1][0] = triangles[i].vertices[2]->y - triangles[i].vertices[1]->y;
            mat2x2[1][1] = y                           - triangles[i].vertices[1]->y;
            dets[1] = determinant(mat2x2, 2);
            mat2x2[0][0] = triangles[i].vertices[0]->x - triangles[i].vertices[2]->x;
            mat2x2[0][1] = x                           - triangles[i].vertices[2]->x;
            mat2x2[1][0] = triangles[i].vertices[0]->y - triangles[i].vertices[2]->y;
            mat2x2[1][1] = y                           - triangles[i].vertices[2]->y;
            dets[2] = determinant(mat2x2, 2);
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
                printf("punto encontrado al interior del triangulo %d\n", i);
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
                if (triangles[i].next[0] != NULL) {
                    triangles[numTrs].next[0] = triangles[i].next[0];
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[1] && 
                            triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("a) existe un vecino que tiene un vertice opuesto en %d\n", j);
                                triangles[i].next[0]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;
                /*
                Se crea el triángulo "c"
                */
                triangles[numTrs].vertices[0] = triangles[i].vertices[0];
                triangles[numTrs].vertices[1] = &vertices[numVs];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                triangles[numTrs].next[1] = triangles[i].next[1];
                /*
                Si el triángulo "i" tiene vecino opuesto al vértice 1,
                entonces ese vecino será el vecino del vértice 1 de
                triangles[numTrs]
                */
                if (triangles[i].next[1] != NULL) {
                    triangles[numTrs].next[1] = triangles[i].next[1];
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[1]->vertices[j] != triangles[numTrs].vertices[0] && 
                            triangles[i].next[1]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("b) existe un vecino que tiene un vertice opuesto en %d\n", j);
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
                , y se actualizan lo que falta respecto a los
                vecinos
                */
                triangles[numTrs - 1].next[0] = &triangles[numTrs - 2];
                triangles[numTrs - 1].next[2] = &triangles[i];
                triangles[numTrs - 2].next[1] = &triangles[numTrs - 1];
                triangles[numTrs - 2].next[2] = &triangles[i];
                triangles[i].next[0] = &triangles[numTrs - 2];
                triangles[i].next[1] = &triangles[numTrs - 1];
                /*
                Se aplica el test del circulo a los nuevos 3 triangulos
                (2 nuevos y uno modificado), siempre q existan los vecinos
                externos
                test del circulo triangulo "a"
                */
                if (triangles[i].next[2] != NULL) {
                    for(j=0; j<3; j++) {
                        if(triangles[i].next[2]->vertices[j] != triangles[i].vertices[0] &&
                            triangles[i].next[2]->vertices[j] != triangles[i].vertices[1]) {
                            printf("test circulo entre triagulo 'a' %p y vertice %d de %p\n",
                                &triangles[i], j, triangles[i].next[2]);
                            mat4x4[0][0] = triangles[i].vertices[0]->x;
                            mat4x4[0][1] = triangles[i].vertices[0]->y;
                            mat4x4[0][2] = pow(triangles[i].vertices[0]->x, 2) + pow(triangles[i].vertices[0]->y, 2);
                            mat4x4[0][3] = 1;
                            mat4x4[1][0] = triangles[i].vertices[1]->x;
                            mat4x4[1][1] = triangles[i].vertices[1]->y;
                            mat4x4[1][2] = pow(triangles[i].vertices[1]->x, 2) + pow(triangles[i].vertices[1]->y, 2);
                            mat4x4[1][3] = 1;
                            mat4x4[2][0] = triangles[i].vertices[2]->x;
                            mat4x4[2][1] = triangles[i].vertices[2]->y;
                            mat4x4[2][2] = pow(triangles[i].vertices[2]->x, 2) + pow(triangles[i].vertices[2]->y, 2);
                            mat4x4[2][3] = 1;
                            mat4x4[3][0] = triangles[i].next[2]->vertices[j]->x;
                            mat4x4[3][1] = triangles[i].next[2]->vertices[j]->y;
                            mat4x4[3][2] = pow(triangles[i].next[2]->vertices[j]->x, 2) + pow(triangles[i].next[2]->vertices[j]->y, 2);
                            mat4x4[3][3] = 1;
                            det = determinant(mat4x4, 4);
                            printf("test circulo 'a' %p: %.1f\n", &triangles[i], det);
                            if (det>0) {
                                for(k=0; k<3; k++) {
                                    if(triangles[i].next[2]->vertices[k] == triangles[i].vertices[0]) {
                                        triangles[i].next[2]->vertices[k] = &vertices[numVs];
                                        break;
                                    }
                                }
                                triangles[i].vertices[1] = triangles[i].next[2]->vertices[j];
                            }
                        }
                    }
                }
                /*
                test del círculo para el triángulo "b"
                */
                if (triangles[numTrs - 2].next[0] != NULL) {
                    for(j=0; j<3; j++) {
                        if(triangles[numTrs - 2].next[0]->vertices[j] != triangles[numTrs - 2].vertices[1] &&
                            triangles[numTrs - 2].next[0]->vertices[j] != triangles[numTrs - 2].vertices[2]) {
                            printf("test circulo entre triagulo 'b' %p y vertice %d de %p\n",
                                &triangles[numTrs - 2], j, triangles[numTrs - 2].next[0]);
                            mat4x4[0][0] = triangles[numTrs - 2].vertices[0]->x;
                            mat4x4[0][1] = triangles[numTrs - 2].vertices[0]->y;
                            mat4x4[0][2] = pow(triangles[numTrs - 2].vertices[0]->x, 2) + pow(triangles[numTrs - 2].vertices[0]->y, 2);
                            mat4x4[0][3] = 1;
                            mat4x4[1][0] = triangles[numTrs - 2].vertices[1]->x;
                            mat4x4[1][1] = triangles[numTrs - 2].vertices[1]->y;
                            mat4x4[1][2] = pow(triangles[numTrs - 2].vertices[1]->x, 2) + pow(triangles[numTrs - 2].vertices[1]->y, 2);
                            mat4x4[1][3] = 1;
                            mat4x4[2][0] = triangles[numTrs - 2].vertices[2]->x;
                            mat4x4[2][1] = triangles[numTrs - 2].vertices[2]->y;
                            mat4x4[2][2] = pow(triangles[numTrs - 2].vertices[2]->x, 2) + pow(triangles[numTrs - 2].vertices[2]->y, 2);
                            mat4x4[2][3] = 1;
                            mat4x4[3][0] = triangles[numTrs - 2].next[0]->vertices[j]->x;
                            mat4x4[3][1] = triangles[numTrs - 2].next[0]->vertices[j]->y;
                            mat4x4[3][2] = pow(triangles[numTrs - 2].next[0]->vertices[j]->x, 2) + pow(triangles[numTrs - 2].next[0]->vertices[j]->y, 2);
                            mat4x4[3][3] = 1;
                            det = determinant(mat4x4, 4);
                            printf("test circulo 'b' %p: %.1f\n", &triangles[numTrs - 2], det);
                            if (det>0) {
                                for(k=0; k<3; k++) {
                                    if(triangles[numTrs - 2].next[0]->vertices[k] == triangles[numTrs - 2].vertices[1]) {
                                        triangles[numTrs - 2].next[0]->vertices[k] = &vertices[numVs];
                                        break;
                                    }
                                }
                                triangles[numTrs - 2].vertices[2] = triangles[numTrs - 2].next[0]->vertices[j];
                            }
                        }
                    }
                }
                /*
                test del círculo para el triángulo "c"
                */
                if (triangles[numTrs - 1].next[1] != NULL) {
                    for(j=0; j<3; j++) {
                        if(triangles[numTrs - 1].next[1]->vertices[j] != triangles[numTrs - 1].vertices[0] &&
                            triangles[numTrs - 1].next[1]->vertices[j] != triangles[numTrs - 1].vertices[2]) {
                            printf("test circulo entre triagulo 'c' %p y vertice %d de %p\n",
                                &triangles[numTrs - 1], j, triangles[numTrs - 1].next[1]);
                            mat4x4[0][0] = triangles[numTrs - 1].vertices[0]->x;
                            mat4x4[0][1] = triangles[numTrs - 1].vertices[0]->y;
                            mat4x4[0][2] = pow(triangles[numTrs - 1].vertices[0]->x, 2) + pow(triangles[numTrs - 1].vertices[0]->y, 2);
                            mat4x4[0][3] = 1;
                            mat4x4[1][0] = triangles[numTrs - 1].vertices[1]->x;
                            mat4x4[1][1] = triangles[numTrs - 1].vertices[1]->y;
                            mat4x4[1][2] = pow(triangles[numTrs - 1].vertices[1]->x, 2) + pow(triangles[numTrs - 1].vertices[1]->y, 2);
                            mat4x4[1][3] = 1;
                            mat4x4[2][0] = triangles[numTrs - 1].vertices[2]->x;
                            mat4x4[2][1] = triangles[numTrs - 1].vertices[2]->y;
                            mat4x4[2][2] = pow(triangles[numTrs - 1].vertices[2]->x, 2) + pow(triangles[numTrs - 1].vertices[2]->y, 2);
                            mat4x4[2][3] = 1;
                            mat4x4[3][0] = triangles[numTrs - 1].next[1]->vertices[j]->x;
                            mat4x4[3][1] = triangles[numTrs - 1].next[1]->vertices[j]->y;
                            mat4x4[3][2] = pow(triangles[numTrs - 1].next[1]->vertices[j]->x, 2) + pow(triangles[numTrs - 1].next[1]->vertices[j]->y, 2);
                            mat4x4[3][3] = 1;
                            det = determinant(mat4x4, 4);
                            printf("test circulo 'c' %p: %.1f\n", &triangles[numTrs - 1], det);
                            if (det>0) {
                                for(k=0; k<3; k++) {
                                    if(triangles[numTrs - 1].next[1]->vertices[k] == triangles[numTrs - 1].vertices[2]) {
                                        triangles[numTrs - 1].next[1]->vertices[k] = &vertices[numVs];
                                        break;
                                    }
                                }
                                triangles[numTrs - 1].vertices[0] = triangles[numTrs - 1].next[1]->vertices[j];
                            }
                        }
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
            X    x
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
                idVerticeCompartido1 = 2;
                idVerticeCompartido2 = 0;
            }
            if (idBorde>0) {
                printf("punto encontrado en triangulo %d, en borde %d\n", i, idBorde);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                if (triangles[i].next[idVerticeOpuesto] != NULL) {
                    /*
                    Se crea nuevo triangulo sobre el vecino
                    */
                    for(j=0; j<3; j++) {
                        /*
                        Se busca el vertice opuesto del vecino
                        */
                        if(triangles[i].next[idVerticeOpuesto]->vertices[j] != triangles[i].vertices[idVerticeCompartido1] &&
                            triangles[i].next[idVerticeOpuesto]->vertices[j] != triangles[i].vertices[idVerticeCompartido2]) {
                            printf("triangulo %d en %p tiene vecino %p opuesto al vertice %d el que tiene vertice opuesto %d\n", i, &triangles[i], triangles[i].next[idVerticeOpuesto], idVerticeOpuesto, j);
                            triangles[numTrs].vertices[0] = triangles[i].next[idVerticeOpuesto]->vertices[j];
                            triangles[numTrs].vertices[1] = triangles[i].vertices[idVerticeCompartido2];
                            triangles[numTrs].vertices[2] = &vertices[numVs];
                            triangles[numTrs].next[1] = triangles[i].next[idVerticeOpuesto];
                        }
                        if(triangles[i].next[idVerticeOpuesto]->vertices[j] == triangles[i].vertices[idVerticeCompartido1]) {
                            triangles[numTrs].next[2] = triangles[i].next[idVerticeOpuesto]->next[j];
                            triangles[i].next[idVerticeOpuesto]->next[j] = &triangles[numTrs];
                        }
                    }
                    numTrs++;
                    /*
                    Se modifica el vecino para que no se solape
                    */
                    for(j=0; j<3; j++) {
                        if(triangles[i].next[idVerticeOpuesto]->vertices[j] == triangles[i].vertices[idVerticeCompartido2]) {
                            triangles[i].next[idVerticeOpuesto]->vertices[j] = &vertices[numVs];
                            break;
                        }
                    }
                }
                /*
                se crea nuevo triangulo dentro de "i"
                */
                triangles[numTrs].vertices[0] = &vertices[numVs];
                triangles[numTrs].vertices[1] = triangles[i].vertices[idVerticeCompartido2];
                triangles[numTrs].vertices[2] = triangles[i].vertices[idVerticeOpuesto];
                /*
                se actualizan los vecinos
                */
                triangles[numTrs].next[0] = triangles[i].next[idVerticeCompartido1];
                triangles[numTrs].next[1] = &triangles[i];
                if (triangles[i].next[idVerticeOpuesto] != NULL) {
                    triangles[numTrs].next[2] = &triangles[numTrs-1];
                    triangles[numTrs-1].next[0] = &triangles[numTrs];
                }
                numTrs++;
                /*
                se modifica el triangulo "i" para que no se solape con el último nuevo triángulo
                triangles[i].vertices[idVerticeCompartido1] = &vertices[numVs];
                */
                numVs++;
                break;
            }
        }
    }
    fpOutput = fopen(fileOutput, "w");
    for(i=0; i<numTrs;i++) {
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[0]->x, triangles[i].vertices[0]->y);
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[1]->x, triangles[i].vertices[1]->y);
        fprintf(fpOutput,"%f %f\n",triangles[i].vertices[2]->x, triangles[i].vertices[2]->y);
        fprintf(fpOutput,"%f %f\n\n",triangles[i].vertices[0]->x, triangles[i].vertices[0]->y);
    }
    fclose(fpOutput);

    for(i=0; i<numTrs;i++) {
        printf("t%d: %.2f,%.2f %.2f,%.2f %.2f,%.2f vecinos: %p %p %p en %p\n",
            i,triangles[i].vertices[0]->x, triangles[i].vertices[0]->y,
            triangles[i].vertices[1]->x, triangles[i].vertices[1]->y,
            triangles[i].vertices[2]->x, triangles[i].vertices[2]->y,
            triangles[i].next[0], triangles[i].next[1], triangles[i].next[2],
            &triangles[i]);
    }

    for(i=0; i<2; i++) {
        free(mat2x2[i]);
    }
    free(mat2x2);
    for(i=0; i<4; i++) {
        free(mat4x4[i]);
    }
    free(mat4x4);
    return 0;
}
