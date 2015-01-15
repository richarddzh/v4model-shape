#include <mex.h>
#include <math.h>

#define RIDGE(x,y) MyParam.ridge[(y)+(x)*MyParam.h]
#define DEG(x,y) MyParam.degree[(y)+(x)*MyParam.h]
#define OUT(x,y) MyParam.out[(y)+(x)*MyParam.h]
#define MAXLINE 10000
#define MAXNUMLINE 32
#define TREE 10001

struct _param_t_
{
  mwSize h, w;
  mxLogical * ridge;
  double * degree;
  unsigned int * out;
  int minLength;
  int neighbor;
} MyParam;

struct _tree_t_
{
  int parent, x, y, root, depth, neighbor;
} MyTree[MAXLINE];

struct _line_t_ 
{
  int length;
  int line[MAXLINE][2];
  int count;
  mxArray * cells[MAXNUMLINE];
} MyLines;

int ExtendTree(int fromx, int fromy)
{
  int i, j, treeLen = 1, x, y, end1, end2, minNeighbor;
  if (!RIDGE(fromx,fromy) || OUT(fromx,fromy)) return 0;
  MyTree[0].parent = 0;
  MyTree[0].x = fromx;
  MyTree[0].y = fromy;
  MyTree[0].root = 0; /* the ancestor that is direct child of MyTree[0] */
  MyTree[0].depth = 0; /* levels from MyTree[0] */
  MyTree[0].neighbor = 0; /* extended neighbor */
  OUT(fromx,fromy) = TREE;

  /* extend tree node */
  do 
  {
    minNeighbor = MyParam.neighbor;
    for (i = 0; i < treeLen && i < MAXLINE; i++)
    {
      if (MyTree[i].neighbor >= MyParam.neighbor) continue;
      MyTree[i].neighbor++;
      for (x = MyTree[i].x - MyTree[i].neighbor; x <= MyTree[i].x + MyTree[i].neighbor; x++)
      for (y = MyTree[i].y - MyTree[i].neighbor; y <= MyTree[i].y + MyTree[i].neighbor; y++)
      {
        if (treeLen >= MAXLINE) break;
        if (x < 0 || y < 0 || x >= MyParam.w || y >= MyParam.h) continue;
        if (!RIDGE(x,y) || OUT(x,y)) continue;
        MyTree[treeLen].x = x;
        MyTree[treeLen].y = y;
        MyTree[treeLen].parent = i;
        MyTree[treeLen].depth = MyTree[i].depth + 1;
        MyTree[treeLen].root = (i == 0) ? treeLen : MyTree[i].root;
        OUT(x,y) = TREE;
        treeLen++;
      }
      if (MyTree[i].neighbor < minNeighbor) minNeighbor = MyTree[i].neighbor;
    }
  } while (minNeighbor < MyParam.neighbor);
  
  /* find longest path */
  for (end1 = -1, i = 0; i < treeLen; i++) if (end1 == -1 || MyTree[i].depth > MyTree[end1].depth) end1 = i;
  for (end2 = -1, i = 0; i < treeLen; i++)
  {
    OUT(MyTree[i].x, MyTree[i].y) = 0;
    if (MyTree[i].root == MyTree[end1].root) continue;
    if (end2 == -1 || MyTree[i].depth > MyTree[end2].depth) end2 = i;
  }
  if (end1 == -1 || end2 == -1) return 0;
  MyLines.length = MyTree[end1].depth + MyTree[end2].depth + 1;
  if (MyLines.length < MyParam.minLength) return 0;
  MyLines.count++;
  for (j = 0, i = end1; MyTree[i].root; i = MyTree[i].parent) 
  {
    OUT(MyTree[i].x, MyTree[i].y) = MyLines.count;
    MyLines.line[j][0] = MyTree[i].x;
    MyLines.line[j][1] = MyTree[i].y;
    j++;
  }
  OUT(MyTree[0].x, MyTree[0].y) = MyLines.count;
  MyLines.line[j][0] = MyTree[0].x;
  MyLines.line[j][1] = MyTree[0].y;
  for (j = MyLines.length, i = end2; MyTree[i].root; i = MyTree[i].parent) 
  {
    OUT(MyTree[i].x, MyTree[i].y) = MyLines.count;
    j--;
    MyLines.line[j][0] = MyTree[i].x;
    MyLines.line[j][1] = MyTree[i].y;
  }
  return 1;
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mwSize x, y;
  int i;
  MyLines.count = 0;
  if (!ValidateParams(nlhs, plhs, nrhs, prhs)) return;
  for (y = 0; y < MyParam.h; y++) 
  for (x = 0; x < MyParam.w; x++)
  {
    if (ExtendTree(x, y))
    {
      if (nlhs == 2 && MyLines.count <= MAXNUMLINE)
      {
        double * thisLine = NULL;
        MyLines.cells[MyLines.count - 1] = mxCreateDoubleMatrix(MyLines.length, 3, mxREAL);
        thisLine = mxGetPr(MyLines.cells[MyLines.count - 1]);
        for (i = 0; i < MyLines.length; i++)
        {
          thisLine[i] = MyLines.line[i][0];
          thisLine[i + MyLines.length] = MyLines.line[i][1];
          thisLine[i + MyLines.length * 2] = DEG(MyLines.line[i][0], MyLines.line[i][1]);
        }
      }
    }
  }
  if (nlhs == 2)
  {
    plhs[1] = mxCreateCellMatrix(1, MyLines.count);
    for (i = 0; i < MyLines.count && i < MAXNUMLINE; i++)
    {
      mxSetCell(plhs[1], i, MyLines.cells[i]);
    }
  }
}

int ValidateParams(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  // Input: ridge(:,:) of logical, degree(:,:) of double, minLength, neighbor
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
  MyParam.minLength = (int) mxGetScalar(prhs[2]);
  MyParam.neighbor = (int) mxGetScalar(prhs[3]);
  plhs[0] = mxCreateNumericMatrix(MyParam.h, MyParam.w, mxUINT32_CLASS, mxREAL);
  MyParam.out = (unsigned int *) mxGetData(plhs[0]);
  return 1;
}