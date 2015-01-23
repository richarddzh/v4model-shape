#include <mex.h>
#include <math.h>

#define RIDGE(x,y) MyParam.ridge[(y)+(x)*MyParam.h]
#define DEG(x,y) MyParam.degree[(y)+(x)*MyParam.h]
#define OUT(x,y) MyParam.out[(y)+(x)*MyParam.h]
#define MAXLINE 1024
#define MAXNUMLINE 10000

struct _param_t_
{
  mwSize h, w;
  mxLogical * ridge;
  double * degree;
  unsigned int * out;
  double degreeThreshold;
  int minLength;
} MyParam;

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

mxArray * LineCells[MAXNUMLINE];
int Line[MAXLINE][2];
int Move[8][2] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}};
int DegMove[4][8] = 
{
  /* -22.5~ 22.5 */ {1,0,1,1,1,1,0,1},
  /*  22.5~ 67.5 */ {0,1,1,1,1,1,1,0},
  /*  67.5~112.5 */ {1,1,1,0,0,1,1,1},
  /* 112.5~157.5 */ {1,1,0,1,1,0,1,1}
};
int NextMove[8][8] = 
{
  {1,1,1,1,0,1,0,0},
  {1,1,1,1,1,0,0,0},
  {1,1,1,0,1,0,0,1},
  {1,1,0,1,0,1,1,0},
  {0,1,1,0,1,0,1,1},
  {1,0,0,1,0,1,1,1},
  {0,0,0,1,1,1,1,1},
  {0,0,1,0,1,1,1,1}
};

int DegreeIndex(double deg)
{
  if (deg < 22.5 || deg >= 157.5) return 0;
  if (deg < 67.5) return 1;
  if (deg < 112.5) return 2;
  return 3;
}

double DegreeDiff(double a, double b)
{
  double d = a > b ? a - b : b - a;
  return d <= 90 ? d : 180 - d;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mwSize x, y;
  int px, py, lastMove, i, m, nx, ny, len;
  double degDiff;
  unsigned int path = 0;
  if (!ValidateParams(nlhs, plhs, nrhs, prhs)) return;
  for (y = 0; y < MyParam.h; y++) 
  for (x = 0; x < MyParam.w; x++)
  {
    if (!RIDGE(x,y)) continue;
    if (OUT(x,y)) continue;
    for (px = x, py = y, lastMove = -1, len = 0; 1; )
    {
      for (i = 0; i < len; i++) if (Line[i][0] == px && Line[i][1] == py) break;
      if (i < len) break;
      Line[len][0] = px;
      Line[len][1] = py;
      len++;
      if (len >= MAXLINE) break;
      for (m = -1, i = 0; i < 8; i++)
      {
        if (lastMove != -1 && !NextMove[lastMove][i]) continue;
        if (!DegMove[DegreeIndex(DEG(px,py))]) continue;
        nx = Move[i][0] + px;
        ny = Move[i][1] + py;
        if (nx < 0 || nx >= MyParam.w || ny < 0 || ny >= MyParam.h) continue;
        if (OUT(nx,ny) > 0 || !RIDGE(nx,ny)) continue;
        if (m == -1 || degDiff > DegreeDiff(DEG(nx,ny), DEG(px,py)))
        {
          m = i;
          degDiff = DegreeDiff(DEG(nx,ny), DEG(px,py));
        }
      }
      if (m == -1 || degDiff > MyParam.degreeThreshold) break;
      lastMove = m;
      px += Move[m][0];
      py += Move[m][1];
    }
    if (len >= MyParam.minLength) 
    {
      double * thisLine = NULL;
      if (nlhs == 2 && path < MAXNUMLINE) 
      {
        LineCells[path] = mxCreateDoubleMatrix(len, 3, mxREAL);
        thisLine = mxGetPr(LineCells[path]);
      }
      path++;
      for (i = 0; i < len; i++)
      {
        OUT(Line[i][0], Line[i][1]) = path;
        if (thisLine)
        {
          thisLine[i] = Line[i][0];
          thisLine[i + len] = Line[i][1];
          thisLine[i + len * 2] = DEG(Line[i][0], Line[i][1]);
        }
      }
    }
  }
  if (nlhs == 2)
  {
    plhs[1] = mxCreateCellMatrix(1, path);
    for (i = 0; i < path && i < MAXNUMLINE; i++)
    {
      mxSetCell(plhs[1], i, LineCells[i]);
    }
  }
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  // Input: ridge(:,:) of logical, degree(:,:) of double, degreeThreshold, minLength
  // Output: uint32(:,:), {double(:,1:3)}
  mwSize ndim;
  const mwSize * dims;
  if (nlhs > 2 || nrhs != 4)
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
  MyParam.ridge = mxGetLogicals(prhs[0]);
  MyParam.degree = mxGetPr(prhs[1]);
  MyParam.degreeThreshold = mxGetScalar(prhs[2]);
  MyParam.minLength = (int) mxGetScalar(prhs[3]);
  plhs[0] = mxCreateNumericMatrix(MyParam.h, MyParam.w, mxUINT32_CLASS, mxREAL);
  MyParam.out = (unsigned int *) mxGetData(plhs[0]);
  return 1;
}