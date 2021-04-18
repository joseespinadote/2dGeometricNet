/*ref: https://github.com/BadLice/determinante.git*/

#include "math.h"

float determinant(float** mat, int size);
float** minore(float **mat, int i, int j, int dim);

float determinant(float** mat, int size)
{
  if(size==1)
    return mat[0][0];
  if(size==2)
      return ((mat[0][0]*mat[1][1])-(mat[1][0]*mat[0][1]));
  int i;
  float sum=0;
  for(i=0;i<size;i++)
  {
    float** min = minore(mat,i,0,size);
    sum += (pow(-1,i+0))*mat[i][0]*determinant(min,size-1);
  }
  return sum;
}

float** minore(float **mat, int i, int j, int dim) {
    int s;
    int t;
    float ** min = calloc(dim-1, sizeof(int*));
    for(s=0; s<dim-1; s++)
        min[s]=calloc(dim-1,sizeof(int));
    int a=0,b=0;
    for(s=0; s<dim; s++)
    {
        b=0;
        for(t=0; t<dim; t++)
        {
            if(!(s==i || t==j))
            {
                min[a][b] = mat[s][t];
                b++;
            }
        }
        if(!(s==i || t==j))
        a++;
    }
    return min;
}
