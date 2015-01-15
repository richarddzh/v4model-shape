#include <mex.h>
#include <float.h>

#define IN(x,y) MyParam.in[offset + (x) * MyParam.h + (y)]
#define OUT(x,y) MyParam.out[offset + (x) * MyParam.h + (y)]

struct _param_t_ 
{
  mwSize h, w, n, neighbor;
  double * in;
  mxLogical * out;
} MyParam;

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mwSize x, y, n, offset, i;
  double xmax1, ymax1, xmax2, ymax2, thisxy;
  if (!ValidateParams(nlhs, plhs, nrhs, prhs)) return;
  for (n = 0; n < MyParam.n; n++)
  {
    offset = MyParam.h * MyParam.w * n;
    for (y = MyParam.neighbor; y + MyParam.neighbor < MyParam.h; y++)
    for (x = MyParam.neighbor; x + MyParam.neighbor < MyParam.w; x++)
    {
      xmax1 = ymax1 = xmax2 = ymax2 = -DBL_MAX;
      thisxy = IN(x, y);
      for (i = 1; i <= MyParam.neighbor; i++)
      {
        if (IN(x-i,y) > xmax1) xmax1 = IN(x-i,y);
        if (IN(x+i,y) > xmax2) xmax2 = IN(x+i,y);
        if (IN(x,y-i) > ymax1) ymax1 = IN(x,y-i);
        if (IN(x,y+i) > ymax2) ymax2 = IN(x,y+i);
        if ((xmax1 >= thisxy || xmax2 > thisxy) && (ymax1 >= thisxy || ymax2 > thisxy)) break;
      }
      if ((xmax1 < thisxy && xmax2 <= thisxy) || (ymax1 < thisxy && ymax2 <= thisxy)) OUT(x,y) = 1;
    }
  }
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  // Input: double(:,:), neighbor
  // Output: logical(:,:)
  mwSize ndim, i;
  const mwSize * dims;
  if (nlhs > 1 || nrhs != 2)
  {
    mexErrMsgTxt("Invalid number of arguments.");
    return 0;
  }
  ndim = mxGetNumberOfDimensions(prhs[0]);
  dims = mxGetDimensions(prhs[0]);
  plhs[0] = mxCreateLogicalArray(ndim, dims);
  MyParam.h = 0;
  MyParam.w = 0;
  MyParam.n = 1;
  if (ndim > 0) MyParam.h = dims[0];
  if (ndim > 1) MyParam.w = dims[1];
  for (i = 2; i < ndim; i++) MyParam.n *= dims[i];
  MyParam.in = mxGetPr(prhs[0]);
  MyParam.out = mxGetLogicals(plhs[0]);
  MyParam.neighbor = (mwSize) mxGetScalar(prhs[1]);
  return 1;
}