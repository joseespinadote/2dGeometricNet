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
    vertex v1, v2;
    struct triangle *vecino;
} edge;

typedef struct triangle {
    edge e1, e2, e3;
} triangle;

int main()
{
    FILE *fpInput, *fpOutput;
    char fileBuffer[TAMANO_BUFFER];
    char *fileInput="puntos.txt";
    char *fileOutput="salida.txt";
    triangle triangulos[LARGO_MALLA];
    vertex vertices[LARGO_MALLA+2];
    vertex newPoints[LARGO_MALLA+2];
    triangle tempTriangle;
    edge tempEdge;
    float newPointX, newPointY, detE1,
        detE2, detE3;
    int i, j, numPoints, numTriangulos = 0;

    printf(".::el trianguleitor::.\n");

    vertices[0].x = 0; vertices[0].y = 4;
    vertices[1].x = 0; vertices[1].y = 0;
    vertices[2].x = 4; vertices[2].y = 0;
    vertices[3].x = 4; vertices[3].y = 4;
    triangulos[0].e1 = (edge){.v1=vertices[0], .v2=vertices[1], NULL};
    triangulos[0].e2 = (edge){.v1=vertices[1], .v2=vertices[2], NULL};
    triangulos[0].e3 = (edge){.v1=vertices[2], .v2=vertices[0], NULL};
    triangulos[1].e1 = (edge){.v1=vertices[0], .v2=vertices[2], NULL};
    triangulos[1].e2 = (edge){.v1=vertices[2], .v2=vertices[3], NULL};
    triangulos[1].e3 = (edge){.v1=vertices[3], .v2=vertices[0], NULL};
    triangulos[0].e3.vecino = &triangulos[1];
    triangulos[1].e1.vecino = &triangulos[0];
    numTriangulos = 2;

    fpInput = fopen(fileInput, "r"); {}
    if(!fpInput) {
        printf("can't open file\n");
        return 1;
    }
    numPoints=0;
    while(!feof(fpInput)) {
        fscanf(fpInput, "%f %f", &newPointX, &newPointY);
        printf("punto: %f %f\n",newPointX, newPointY);
        newPoints[numPoints] = (vertex){.x=newPointX, .y=newPointY};
        for(i=0; i<numTriangulos;i++) {
            /*
            d=(x−x1)(y2−y1)−(y−y1)(x2−x1)
            ref: https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located
            */
            detE1 = (newPointX-triangulos[i].e1.v1.x)*
                (triangulos[i].e1.v2.y-triangulos[i].e1.v1.y)-
                (newPointY-triangulos[i].e1.v1.y)*
                (triangulos[i].e1.v2.x-triangulos[i].e1.v1.x);
            detE2 = (newPointX-triangulos[i].e2.v1.x)*
                (triangulos[i].e2.v2.y-triangulos[i].e2.v1.y)-
                (newPointY-triangulos[i].e2.v1.y)*
                (triangulos[i].e2.v2.x-triangulos[i].e2.v1.x);
            detE3 = (newPointX-triangulos[i].e3.v1.x)*
                (triangulos[i].e3.v2.y-triangulos[i].e3.v1.y)-
                (newPointY-triangulos[i].e3.v1.y)*
                (triangulos[i].e3.v2.x-triangulos[i].e3.v1.x);
            if (detE1 == 0 && detE2 < 0 && detE3 < 0) {
                tempEdge = triangulos[i].e2;
                triangulos[i].e1 = (edge) {
                    .v1=triangulos[i].e1.v1,
                    .v2=newPoints[numPoints],
                    .vecino=triangulos[i].e1.vecino
                };
                triangulos[i].e2 = (edge) {
                    .v1=triangulos[i].e1.v2,
                    .v2=triangulos[i].e2.v2,
                    .vecino=NULL
                };
                triangulos[numTriangulos].e1 = (edge) {
                    .v1=triangulos[i].e1.v2,
                    .v2=tempEdge.v1,
                    .vecino=NULL
                };
                triangulos[numTriangulos].e2 = (edge) {
                    .v1=triangulos[numTriangulos].e1.v2,
                    .v2=tempEdge.v2,
                    .vecino=tempEdge.vecino
                };
                triangulos[numTriangulos].e3 = (edge) {
                    .v1=triangulos[numTriangulos].e2.v2,
                    .v2=triangulos[numTriangulos].e1.v1,
                    .vecino=&triangulos[i]
                };
                numTriangulos++;
                if (triangulos[i].e1.vecino!=NULL) {
                    printf("tiene vecino en e1!\n");
                    tempEdge=triangulos[i].e1.vecino->e2;
                    triangulos[i].e1.vecino->e2 = (edge) {
                        .v1=triangulos[i].e1.vecino->e1.v2,
                        .v2=triangulos[i].e1.v2,
                        .vecino=NULL
                    };
                    triangulos[i].e1.vecino->e3 = (edge) {
                        .v1=triangulos[i].e1.vecino->e2.v2,
                        .v2=triangulos[i].e1.vecino->e1.v1,
                        .vecino=&triangulos[i]
                    };
                    triangulos[numTriangulos].e1 = (edge) {
                        .v1=triangulos[i].e1.vecino->e2.v1,
                        .v2=tempEdge.v2,
                        .vecino=tempEdge.vecino
                    };
                    triangulos[numTriangulos].e2 = (edge) {
                        .v1=triangulos[numTriangulos].e1.v2,
                        .v2=triangulos[i].e1.vecino->e2.v2,
                        .vecino=&triangulos[numTriangulos-1]
                    };
                    triangulos[numTriangulos].e3 = (edge) {
                        .v1=triangulos[numTriangulos].e2.v2,
                        .v2=triangulos[numTriangulos].e1.v1,
                        .vecino=triangulos[i].e1.vecino
                    };
                    triangulos[numTriangulos-1].e1.vecino = &triangulos[numTriangulos];
                    triangulos[i].e1.vecino->e2.vecino = &triangulos[numTriangulos];
                    numTriangulos++;
                }
                break;
            } else if(detE1 < 0 && detE2 == 0 && detE3 < 0) {
                tempEdge=triangulos[i].e3;
                triangulos[i].e2 = (edge) {
                    .v1=triangulos[i].e2.v1,
                    .v2=newPoints[numPoints],
                    .vecino=triangulos[i].e2.vecino
                };
                triangulos[i].e3 = (edge) {
                    .v1=triangulos[i].e2.v2,
                    .v2=triangulos[i].e1.v1,
                    .vecino=NULL
                };
                triangulos[numTriangulos].e1 = (edge) {
                    .v1=triangulos[i].e1.v1,
                    .v2=triangulos[i].e2.v2,
                    .vecino=&triangulos[i]
                };
                triangulos[numTriangulos].e2 = (edge) {
                    .v1=triangulos[numTriangulos].e1.v2,
                    .v2=tempEdge.v1,
                    .vecino=NULL
                };
                triangulos[numTriangulos].e3 = (edge) {
                    .v1=triangulos[numTriangulos].e2.v2,
                    .v2=triangulos[numTriangulos].e1.v1,
                    .vecino=tempEdge.vecino
                };
                triangulos[i].e3.vecino = &triangulos[numTriangulos];
                numTriangulos++;
                if (triangulos[i].e2.vecino!=NULL) {
                    printf("tiene vecino en e2!\n");
                    tempEdge=triangulos[i].e2.vecino->e2;
                    triangulos[i].e2.vecino->e2 = (edge) {
                        .v1=triangulos[i].e1.vecino->e1.v2,
                        .v2=triangulos[i].e2.v2,
                        .vecino=NULL
                    };
                    triangulos[i].e2.vecino->e3 = (edge) {
                        .v1=triangulos[i].e1.vecino->e2.v2,
                        .v2=triangulos[i].e1.vecino->e1.v1,
                        .vecino=&triangulos[i]
                    };
                    triangulos[numTriangulos].e1 = (edge) {
                        .v1=triangulos[i].e2.vecino->e2.v2,
                        .v2=tempEdge.v1,
                        .vecino=triangulos[i].e2.vecino
                    };
                    triangulos[numTriangulos].e2 = (edge) {
                        .v1=triangulos[numTriangulos].e1.v2,
                        .v2=tempEdge.v2,
                        .vecino=tempEdge.vecino
                    };
                    triangulos[numTriangulos].e3 = (edge) {
                        .v1=triangulos[numTriangulos].e2.v2,
                        .v2=triangulos[numTriangulos].e1.v1,
                        .vecino=&triangulos[numTriangulos-1]
                    };
                    triangulos[numTriangulos-1].e2.vecino = &triangulos[numTriangulos];
                    triangulos[i].e2.vecino->e2.vecino = &triangulos[numTriangulos];
                    numTriangulos++;
                }
                break;
            } else if(detE1 < 0 && detE2 < 0 && detE3 == 0) {
                tempEdge=triangulos[i].e1;
                triangulos[i].e1 = (edge) {
                    .v1=newPoints[numPoints],
                    .v2=triangulos[i].e1.v2,
                    .vecino=NULL
                };
                triangulos[i].e3 = (edge) {
                    .v1=triangulos[i].e2.v2,
                    .v2=triangulos[i].e1.v1,
                    .vecino=triangulos[i].e3.vecino
                };
                triangulos[numTriangulos].e1 = (edge) {
                    .v1=tempEdge.v1,
                    .v2=tempEdge.v2,
                    .vecino=tempEdge.vecino
                };
                triangulos[numTriangulos].e2 = (edge) {
                    .v1=triangulos[numTriangulos].e1.v2,
                    .v2=triangulos[i].e1.v1,
                    .vecino=&triangulos[i]
                };
                triangulos[numTriangulos].e3 = (edge) {
                    .v1=triangulos[numTriangulos].e2.v2,
                    .v2=triangulos[numTriangulos].e1.v1,
                    .vecino=NULL
                };
                triangulos[i].e1.vecino = &triangulos[numTriangulos];
                numTriangulos++;
                if (triangulos[i].e3.vecino!=NULL) {
                    printf("tiene vecino en e3!\n");
                    tempEdge=triangulos[i].e3.vecino->e3;
                    triangulos[i].e3.vecino->e1 = (edge) {
                        .v1=triangulos[i].e1.v1,
                        .v2=triangulos[i].e2.v2,
                        .vecino=&triangulos[i]
                    };
                    triangulos[i].e3.vecino->e3 = (edge) {
                        .v1=tempEdge.v1,
                        .v2=triangulos[i].e1.v1,
                        .vecino=NULL
                    };
                    triangulos[numTriangulos].e1 = (edge) {
                        .v1=tempEdge.v2,
                        .v2=triangulos[i].e3.vecino->e1.v1,
                        .vecino=&triangulos[numTriangulos-1]
                    };
                    triangulos[numTriangulos].e2 = (edge) {
                        .v1=triangulos[numTriangulos].e1.v2,
                        .v2=tempEdge.v1,
                        .vecino=triangulos[i].e3.vecino
                    };
                    triangulos[numTriangulos].e3 = (edge) {
                        .v1=triangulos[numTriangulos].e2.v2,
                        .v2=triangulos[numTriangulos].e1.v1,
                        .vecino=tempEdge.vecino
                    };
                    triangulos[numTriangulos-1].e3.vecino = &triangulos[numTriangulos];
                    triangulos[i].e3.vecino->e3.vecino = &triangulos[numTriangulos];
                    numTriangulos++;
                }
                break;
            } else if(detE1 < 0 && detE2 < 0 && detE3 < 0) {
                printf("el punto esta en %d!\n",i);
                tempTriangle=triangulos[i];
                triangulos[i].e2 = (edge) {
                    .v1=triangulos[i].e2.v1,
                    .v2=newPoints[numPoints],
                    NULL
                };
                triangulos[i].e3 = (edge) {
                    .v1=triangulos[i].e2.v2,
                    .v2=triangulos[i].e1.v1,
                    NULL
                };
                triangulos[numTriangulos].e1 = (edge) {
                    .v1=triangulos[i].e2.v2,
                    .v2=triangulos[i].e1.v2,
                    .vecino=&triangulos[i]
                };
                triangulos[numTriangulos].e2 = (edge) {
                    .v1=triangulos[numTriangulos].e1.v2,
                    .v2=tempTriangle.e2.v2,
                    .vecino=tempTriangle.e2.vecino
                };
                triangulos[numTriangulos].e3 = (edge) {
                    .v1=triangulos[numTriangulos].e2.v2,
                    .v2=triangulos[numTriangulos].e1.v1,
                    .vecino=NULL
                };
                numTriangulos++;
                triangulos[numTriangulos].e1 = (edge) {
                    .v1=triangulos[i].e1.v1,
                    .v2=triangulos[i].e2.v2,
                    .vecino=&triangulos[i]
                };
                triangulos[numTriangulos].e2 = (edge) {
                    .v1=triangulos[numTriangulos].e1.v2,
                    .v2=triangulos[numTriangulos-1].e2.v2,
                    .vecino=&triangulos[numTriangulos-1]
                };
                triangulos[numTriangulos].e3 = (edge) {
                    .v1=triangulos[numTriangulos].e2.v2,
                    .v2=triangulos[numTriangulos].e1.v1,
                    .vecino=tempTriangle.e3.vecino
                };
                triangulos[i].e2.vecino=&triangulos[numTriangulos-1];
                triangulos[i].e3.vecino=&triangulos[numTriangulos];
                triangulos[numTriangulos-1].e3.vecino=&triangulos[numTriangulos];
                numTriangulos++;
                break;
            } else {
                printf("el punto NO esta en %d\n",i);
            }
        }
        numPoints++;
    }
    fclose(fpInput);
    
    fpOutput = fopen(fileOutput, "w");
    for(i=0; i<numTriangulos; i++) {
        printf("Triangulo %d, con direccion %p\n\
e1: (%.2f,%.2f) -> (%.2f,%.2f), vecino %p\n\
e2: (%.2f,%.2f) -> (%.2f,%.2f), vecino %p\n\
e3: (%.2f,%.2f) -> (%.2f,%.2f), vecino %p\n",
            i,&triangulos[i],
            triangulos[i].e1.v1.x, triangulos[i].e1.v1.y,
            triangulos[i].e1.v2.x, triangulos[i].e1.v2.y,
            triangulos[i].e1.vecino,
            triangulos[i].e2.v1.x, triangulos[i].e2.v1.y,
            triangulos[i].e2.v2.x, triangulos[i].e2.v2.y,
            triangulos[i].e2.vecino,
            triangulos[i].e3.v1.x, triangulos[i].e3.v1.y,
            triangulos[i].e3.v2.x, triangulos[i].e3.v2.y,
            triangulos[i].e3.vecino);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e1.v1.x, triangulos[i].e1.v1.y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e1.v2.x, triangulos[i].e1.v2.y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e2.v1.x, triangulos[i].e2.v1.y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e2.v2.x, triangulos[i].e2.v2.y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e3.v1.x, triangulos[i].e3.v1.y);
        fprintf(fpOutput, "%f %f\n", triangulos[i].e3.v2.x, triangulos[i].e3.v2.y);
    }
    fprintf(fpOutput, "\n\n");
    for(i=0; i<numPoints; i++) {
        fprintf(fpOutput, "%f %f\n", newPoints[i].x, newPoints[i].y);
    }
    fclose(fpOutput);
    /*
    for(i=numTriangulos-1; i!=0; i--) {
        free(triangulos[i].e1.v1);
        free(triangulos[i].e1.v2);
        free(triangulos[i].e2.v1);
        free(triangulos[i].e2.v2);
        free(triangulos[i].e3.v1);
        free(triangulos[i].e3.v2);
    }
    */
    return 0;
}
