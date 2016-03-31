#include <mex.h>
#include <math.h>

#define RIDGE(x,y) MyParam.ridge[(y)+(x)*MyParam.h]
#define DEG(x,y) MyParam.degree[(y)+(x)*MyParam.h]
#define OUT(x,y) MyParam.out[(y)+(x)*MyParam.h]
#define MAXLINE 50000
#define MAXNUMLINE 1000
#define TREE 90001

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
  struct _tree1_t_
  {
    int parent, firstChild, nextSibling, depth, extended;
  } build;
  struct _tree2_t_
  {
    int parent, depth, visited;
  } traverse;
  int x, y;
} MyTree[MAXLINE];

struct _line_t_ 
{
  int length;
  int line[MAXLINE][2];
  int count;
  mxArray * cells[MAXNUMLINE];
} MyLines;

void TraverseTree(int node, int depth, int parent, int * maxDepthIdx)
{
  int i;
  if (node < 0 || MyTree[node].traverse.visited) return;
  MyTree[node].traverse.parent = parent;
  MyTree[node].traverse.depth = depth;
  MyTree[node].traverse.visited = 1;
  if (depth > MyTree[*maxDepthIdx].traverse.depth) *maxDepthIdx = node;
  TraverseTree(MyTree[node].build.parent, depth + 1, node, maxDepthIdx);
  for (i = MyTree[node].build.firstChild; i >= 0; i = MyTree[i].build.nextSibling)
  {
    TraverseTree(i, depth + 1, node, maxDepthIdx);
  }
}

int ExtendTree(int fromx, int fromy)
{
  int i, j, treeLen = 1, x, y, end1, end2, minNeighbor;
  if (!RIDGE(fromx,fromy) || OUT(fromx,fromy)) return 0;
  MyTree[0].x = fromx;
  MyTree[0].y = fromy;
  MyTree[0].build.parent = -1;
  MyTree[0].build.firstChild = -1;
  MyTree[0].build.nextSibling = -1;
  MyTree[0].build.depth = 0;
  MyTree[0].build.extended = 0;
  MyTree[0].traverse.visited = 0;
  OUT(fromx,fromy) = TREE;

  /* extend tree node */
  do 
  {
    minNeighbor = MyParam.neighbor;
    for (i = 0; i < treeLen && i < MAXLINE; i++)
    {
      int extended = MyTree[i].build.extended;
      if (treeLen >= MAXLINE) break;
      if (extended >= MyParam.neighbor) continue;
      extended++;
      for (x = MyTree[i].x - extended; x <= MyTree[i].x + extended; x++)
      for (y = MyTree[i].y - extended; y <= MyTree[i].y + extended; y++)
      {
        if (treeLen >= MAXLINE) break;
        if (x < 0 || y < 0 || x >= MyParam.w || y >= MyParam.h) continue;
        if (!RIDGE(x,y) || OUT(x,y)) continue;
        MyTree[treeLen].x = x;
        MyTree[treeLen].y = y;
        MyTree[treeLen].build.parent = i;
        MyTree[treeLen].build.firstChild = -1;
        MyTree[treeLen].build.nextSibling = MyTree[i].build.firstChild;
        MyTree[i].build.firstChild = treeLen;
        MyTree[treeLen].build.depth = MyTree[i].build.depth + 1;
        MyTree[treeLen].build.extended = 0;
        MyTree[treeLen].traverse.visited = 0;
        minNeighbor = 0;
        OUT(x,y) = TREE;
        treeLen++;
      }
      if (extended < minNeighbor) minNeighbor = extended;
      MyTree[i].build.extended = extended;
    }
  } while (minNeighbor < MyParam.neighbor);
  
  /* find longest path */
  for (end1 = -1, i = 0; i < treeLen; i++) 
  {
    OUT(MyTree[i].x, MyTree[i].y) = 0;
    if (end1 == -1 || MyTree[i].build.depth > MyTree[end1].build.depth) end1 = i;
  }
  if (end1 == -1) return 0;
  end2 = end1;
  TraverseTree(end1, 0, -1, &end2);
  MyLines.length = MyTree[end2].traverse.depth + 1;
  if (MyLines.length < MyParam.minLength) return 0;

  /* save path from end2 to end1 */
  MyLines.count++;
  for (j = 0, i = end2; i >= 0; i = MyTree[i].traverse.parent) 
  {
    OUT(MyTree[i].x, MyTree[i].y) = MyLines.count;
    MyLines.line[j][0] = MyTree[i].x;
    MyLines.line[j][1] = MyTree[i].y;
    j++;
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