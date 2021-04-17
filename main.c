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
    i y j son índices genéricos
    */
    float x, y, det, dets[3], **mat2x2, **mat4x4;
    int i, j, numTrs, numVs;

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
                /*
                Si en dirección opuesta al vértice 0 del triángulo "i"
                existe un vecino, entonces se le asigna ese mismo al nuevo
                triángulo "triangles[numTrs]"
                Al mismo tiempo, se asigna desde ese vecino a triangles[numTrs]
                como vecino opuesto cuyo vértice no comparte con el triángulo "i"
                */
                if (triangles[i].next[0]!=NULL) {
                    triangles[numTrs].next[0] = triangles[i].next[0];
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[1] && 
                            triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("ok a\n");
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
                                printf("ok b\n");
                                triangles[i].next[1]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;

                /*
                El triángulo "i" toma la forma de "a", y se asignan
                los nuevos triángulos como sus vecinos
                */
                triangles[i].vertices[2] = &vertices[numVs];
                triangles[i].next[0] = &triangles[numTrs - 1];
                triangles[i].next[1] = &triangles[numTrs];

                /*
                Se considera que hay un nuevo vértice la ejecución
                */
                numVs++;
                break;
            } 
            /*
            Caso en que el punto cae en el primero lado de un triángulo "i"
            0
            |\
            | \
            |  \
            |   \
            X    \
            |  i  \
            |      \
            |_______\
            1        2
            */
            else if (dets[0] == 0 && dets[1] != 0 && dets[2] != 0) {
                printf("punto encontrado en borde 1 del triangulo %d\n", i);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                /*
                0
                |\
                | \
                |  \
                | a \
                X    \
                |\    \
                |   \  \
                |__b___\\
                1        2
                Se crea el triángulo "b" 
                */
                triangles[numTrs].vertices[0] = &vertices[numVs];
                triangles[numTrs].vertices[1] = triangles[i].vertices[1];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                triangles[numTrs].next[1] = &triangles[i];
                /*
                Caso en que el triángulo "i" tuviese un vecino opuesto a su
                vértice 0
                */
                if (triangles[i].next[0] != NULL) {
                    triangles[numTrs].next[0] = triangles[i].next[0];
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[1] && 
                            triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("ok c\n");
                                triangles[i].next[0]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;

                /*
                Caso en que el triángulo "i" tuviese un vecino opuesto a su
                vértice 2. Esto requerirá crear un nuevo triángulo
                */
                if (triangles[i].next[2] != NULL) {
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[2]->vertices[j] != triangles[i].vertices[0] && 
                            triangles[i].next[2]->vertices[j] != triangles[i].vertices[1]) {
                                printf("ok d\n");
                                triangles[numTrs].vertices[0] = triangles[i].next[2]->vertices[j];
                                triangles[numTrs].vertices[1] = triangles[i].vertices[1];
                                triangles[numTrs].vertices[2] = &vertices[numVs];
                                triangles[numTrs].next[0] = &triangles[numTrs-1];
                                triangles[numTrs].next[1] = triangles[i].next[2];
                                triangles[numTrs-1].next[2] = &triangles[numTrs];
                                numTrs++;
                        }
                        if (triangles[i].next[2]->vertices[j] == triangles[i].vertices[1]) {
                            triangles[i].next[2]->vertices[j] = &vertices[numVs];
                        }
                    }
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[2]->vertices[j] == triangles[i].vertices[0]) {
                            if (triangles[i].next[2]->next[j] != NULL) {
                                triangles[numTrs-1].next[2] = triangles[i].next[2]->next[j];
                            }
                            triangles[i].next[2]->next[j] = &triangles[numTrs-1];
                        }
                    }
                }

                triangles[i].vertices[1] = &vertices[numVs];
                triangles[i].next[0] = &triangles[numTrs-2];

                /*
                Test del círculo
                */
                if (triangles[i].next[1]!=NULL) {
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[1]->vertices[j] != triangles[i].vertices[0] && 
                            triangles[i].next[1]->vertices[j] != triangles[i].vertices[2]) {
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
                            mat4x4[3][0] = triangles[i].next[1]->vertices[j]->x;
                            mat4x4[3][1] = triangles[i].next[1]->vertices[j]->y;
                            mat4x4[3][2] = pow(triangles[i].next[1]->vertices[j]->x, 2) + pow(triangles[i].next[1]->vertices[j]->y, 2);
                            mat4x4[3][3] = 1;
                            det = determinant(mat2x2, 2);                            
                            printf("-->%f<--\n", det);
                        }
                    }
                }

                numVs++;
                break;
            } else if (dets[0] != 0 && dets[1] == 0 && dets[2] != 0) {
                printf("punto encontrado en borde 2 del triangulo %d\n", i);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                /*
                0
                |\
                ||\
                || \
                | | \
                | |  \
                |a | b\
                |  |   \
                |___X___\
                1        2
                Se crea el triángulo "b"
                */
                triangles[numTrs].vertices[0] = triangles[i].vertices[0];
                triangles[numTrs].vertices[1] = &vertices[numVs];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                triangles[numTrs].next[2] = &triangles[i];
                /*
                Caso en que exista un vecino opuesto al vértice 1
                */
                if (triangles[i].next[1] != NULL) {
                    triangles[numTrs].next[1] = triangles[i].next[1];
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[0] && 
                            triangles[i].next[0]->vertices[j] != triangles[numTrs].vertices[2]) {
                                printf("ok aa\n");
                                triangles[i].next[0]->next[j] = &triangles[numTrs];
                            }
                    }
                }
                numTrs++;
                /*
                Caso en que el triángulo "i" tuviese un vecino opuesto a su
                vértice 0. Esto requerirá crear un nuevo triángulo
                */
                if (triangles[i].next[0] != NULL) {
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[0]->vertices[j] != triangles[i].vertices[1] && 
                            triangles[i].next[0]->vertices[j] != triangles[i].vertices[2]) {
                                printf("ok ab\n");
                                triangles[numTrs].vertices[0] = &vertices[numVs];
                                triangles[numTrs].vertices[1] = triangles[i].next[0]->vertices[j];
                                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                                /*
                                si triangles[i].next[0] tiene un vecino en los vertices que comparte con
                                triangles[numTrs], entonces se deben actualizar esos vecinos

                                triangles[numTrs].next[0] = 
                                triangles[numTrs].next[1] = 
                                triangles[numTrs-1].next[2] = 
                                */
                                numTrs++;
                        }
                        if (triangles[i].next[2]->vertices[j] == triangles[i].vertices[1]) {
                            triangles[i].next[2]->vertices[j] = &vertices[numVs];
                        }
                    }
                    for(j=0;j<3;j++) {
                        if (triangles[i].next[2]->vertices[j] == triangles[i].vertices[0]) {
                            if (triangles[i].next[2]->next[j] != NULL) {
                                triangles[numTrs-1].next[2] = triangles[i].next[2]->next[j];
                            }
                            triangles[i].next[2]->next[j] = &triangles[numTrs-1];
                        }
                    }
                }
                triangles[i].vertices[2] = &vertices[numVs];
                numVs++;
                break;
            } else if (dets[0] != 0 && dets[1] != 0 && dets[2] == 0) {
                printf("punto encontrado en borde 3 del triangulo %d\n", i);
                vertices[numVs] = (Vertex){.x=x, .y=y};
                
                triangles[numTrs].vertices[0] = &vertices[numVs];
                triangles[numTrs].vertices[1] = triangles[i].vertices[1];
                triangles[numTrs].vertices[2] = triangles[i].vertices[2];
                numTrs++;

                triangles[i].vertices[3] = &vertices[numVs];
                numVs++;

                if (triangles[i].next[1]!=NULL) {
                    
                }
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
