#include <mex.h>
#include <stdlib.h>

#define PATH 1
#define LEFT 2
#define BOTTOM 3
#define CROSS 4
#define MAXPATH 10000

#define FS(x,y) MyParam.freespace[(y)+(x)*MyParam.h]
#define OUT(x,y) MyParam.out[(y)+(x)*MyParam.h]
#define DIST(x,y) MyParam.dist[(y)+(x)*MyParam.h]
#define MAT(p,m,i,j) p[(i)+(j)*(m)] 

struct _param_t_
{
  mwSize h, w;
  double * freespace;
  double * dist;
  unsigned int * out;
  int cpath;
  struct _point_t_ {
    int x, y;
  } path[MAXPATH];
} MyParam;

void FindPath()
{
  int x, y;
  MyParam.dist = (double *) malloc(sizeof(double) * MyParam.h * MyParam.w);
  for (y = 0; y < MyParam.h; y++)
  for (x = 0; x < MyParam.w; x++)
  {
    OUT(x,y) = PATH;
    DIST(x,y) = FS(x,y);
    if (x > 0 && y > 0 && (OUT(x,y) == PATH || FS(x,y) + DIST(x-1,y-1) < DIST(x,y)))
    {
      OUT(x,y) = CROSS;
      DIST(x,y) = FS(x,y) + DIST(x-1,y-1);
    }
    if (x > 0 && (OUT(x,y) == PATH || FS(x,y) + DIST(x-1,y) < DIST(x,y))) 
    {
      OUT(x,y) = LEFT;
      DIST(x,y) = FS(x,y) + DIST(x-1,y);
    }
    if (y > 0 && (OUT(x,y) == PATH || FS(x,y) + DIST(x,y-1) < DIST(x,y)))
    {
      OUT(x,y) = BOTTOM;
      DIST(x,y) = FS(x,y) + DIST(x,y-1);
    }
  }
  for (x = MyParam.w - 1, y = MyParam.h - 1; x > 0 || y > 0;)
  {
    switch (OUT(x,y))
    {
      case CROSS: OUT(x,y) = PATH; x--; y--; break;
      case LEFT: OUT(x,y) = PATH; x--; break;
      case BOTTOM: OUT(x,y) = PATH; y--; break;
    }
  }
  MyParam.cpath = 0;
  for (y = 0; y < MyParam.h; y++)
  for (x = 0; x < MyParam.w; x++)
  {
    if (OUT(x,y) != PATH) OUT(x,y) = 0;
    else 
    {
      MyParam.path[MyParam.cpath].x = x;
      MyParam.path[MyParam.cpath].y = y;
      MyParam.cpath++;
    }
  }
  free(MyParam.dist);
  MyParam.dist = NULL;
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (!ValidateParams(nlhs, plhs, nrhs, prhs)) return;
  FindPath();
  if (nlhs == 2)
  {
    int i, j;
    size_t m1 = 0, n1 = 0, m2 = 0, n2 = 0;
    double * mat = NULL, * mat1 = NULL, * mat2 = NULL;
    if (nrhs > 1) { m1 = mxGetM(prhs[1]); n1 = mxGetN(prhs[1]); mat1 = mxGetPr(prhs[1]); }
    if (nrhs > 2) { m2 = mxGetM(prhs[2]); n2 = mxGetN(prhs[2]); mat2 = mxGetPr(prhs[2]); }
    plhs[1] = mxCreateDoubleMatrix(MyParam.cpath, n1 + n2, mxREAL);
    mat = mxGetPr(plhs[1]);
    for (i = 0; i < MyParam.cpath; i++)
    {
      for (j = 0; j < n1; j++) MAT(mat, MyParam.cpath, i, j) = MAT(mat1, m1, MyParam.path[i].y-1, j);
      for (j = 0; j < n2; j++) MAT(mat, MyParam.cpath, i, j + n1) = MAT(mat2, m2, MyParam.path[i].x-1, j);
    }
  }
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  // Input: freespace(:,:) of double, m1 of double(:,:), m2 of double(:,:)
  // Output: uint32(:,:), merged m1 and m2
  mwSize ndim;
  const mwSize * dims;
  if (nlhs > 2 || nrhs > 3)
  {
    mexErrMsgTxt("Invalid number of arguments.");
    return 0;
  }
  ndim = mxGetNumberOfDimensions(prhs[0]);
  dims = mxGetDimensions(prhs[0]);
  MyParam.h = 0;
  MyParam.w = 0;
  if (ndim > 0) MyParam.h = dims[0];
  if (ndim > 1) MyParam.w = dims[1];
  MyParam.freespace = mxGetPr(prhs[0]);
  plhs[0] = mxCreateNumericMatrix(MyParam.h, MyParam.w, mxUINT32_CLASS, mxREAL);
  MyParam.out = (unsigned int *) mxGetData(plhs[0]);
  return 1;
}