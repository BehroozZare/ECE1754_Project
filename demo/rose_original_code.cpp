#include <random>
#include <iostream>
#define T 2
#define N 32
#include "polycheck_demo_functions.h" 
double Original_A[32][32];
double Transformed_A[32][32];
//====================== Compiler stuff ===================
double Instrument_A[N][N];
polyfunc::StateInst shadow[N][N];
polyfunc::ExpressionMapping mapping;
polyfunc::StateInst min_bound;
polyfunc::StateInst max_bound;
std::vector<bool> wref_fix_flag;
std::vector<int> read_const;

void Transformed_Sidel(int t,int ilo,int ihi,int jlo,int jhi)
{
  if (ilo > ihi || jlo > jhi) 
    return ;
  if (ilo == ihi && jlo == jhi) {
    Transformed_A[ilo][jlo] = Transformed_A[ilo - 1][jlo] + Transformed_A[ilo][jlo - 1];
//====================== Compiler stuff ===================
std::vector<int> wref{ilo,jlo};
polyfunc::StateInst optStat;
if(shadow[ilo][jlo].isInit() || !shadow[ilo][jlo].isValid()){
optStat = polyfunc::firstWriter(min_bound, wref, wref_fix_flag, mapping.LHS_MAP);
 assert(optStat.isValid());
} else {
 optStat = polyfunc::nextWriter(shadow[ilo][jlo], max_bound, wref, wref_fix_flag, mapping.LHS_MAP);
 assert(optStat.isValid());
}
std::vector<int> read_ref{ilo - 1,jlo,ilo,jlo - 1};
for(int rr_ptr = 0; rr_ptr < read_ref.size(); rr_ptr++){
assert(polyfunc::dotProduct(mapping.RHS_MAP[rr_ptr], optStat.instance) + read_const[rr_ptr] == read_ref[rr_ptr]);
}
std::vector<int> read_ref0(read_ref.begin() + 0, read_ref.begin() + 2);
std::vector<int> read_ref1(read_ref.begin() + 2, read_ref.begin() + 4);
assert(shadow[ilo - 1][jlo]== polyfunc::writeBeforeRead(optStat, min_bound, read_ref0, wref_fix_flag, mapping.LHS_MAP));
assert(shadow[ilo][jlo - 1]== polyfunc::writeBeforeRead(optStat, min_bound, read_ref1, wref_fix_flag, mapping.LHS_MAP));
shadow[ilo][jlo]= optStat;
//==========================================================
  }
   else {
    Transformed_Sidel(t,ilo,(ilo + ihi) / 2,jlo,(jlo + jhi) / 2);
//Top-Left
    Transformed_Sidel(t,ilo,(ilo + ihi) / 2,(jlo + jhi) / 2 + 1,jhi);
//Top-Right
    Transformed_Sidel(t,(ilo + ihi) / 2 + 1,ihi,jlo,(jlo + jhi) / 2);
//Bottom-Left
    Transformed_Sidel(t,(ilo + ihi) / 2 + 1,ihi,(jlo + jhi) / 2 + 1,jhi);
//Bottom-Right
  }
  if (ilo == 1 && ihi == 32 - 1 && jlo == 1 && jhi == 32 - 1 && t < 2 - 1) {
// The naive poly chech find this bug
    Transformed_Sidel(t + 1,ilo,ihi,jlo,jhi);
// Next time step
  }
}

void Original_Sidel()
{
  for (int t = 0; t < 2; t++) {
    for (int i = 1; i < 32; i++) {
      for (int j = 1; j < 32; j++) {
        Original_A[i][j] = Original_A[i - 1][j] + Original_A[i][j - 1];
      }
    }
  }
}

int main()
{
// =========================== Test Section ======================
// Generate some random data for Seidel
  class std::random_device rd;
  std::mt19937 mt(rd());
  class std::uniform_real_distribution< double  > dist(1.0,10.0);
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      Original_A[i][j] = dist(mt);
      Transformed_A[i][j] = Original_A[i][j];
    }
  }
//====================== Compiler stuff ===================
std::vector<std::vector<bool>> readWriteSet(N, std::vector<bool>(N, true));
for(int i=1;i<32;i++){
for(int j=1;j<32;j++){
if(readWriteSet[1*i + 0][1*j + 0]){
shadow[1*i + 0][1*j + 0].invalidate();
shadow[1*i + 0][1*j + 0].makeNoInit();
readWriteSet[1*i + 0][1*j + 0]= false;
}
if(readWriteSet[1*i + -1][1*j + 0]){
shadow[1*i + -1][1*j + 0].makeValid();
shadow[1*i + -1][1*j + 0].makeInit();
readWriteSet[1*i + -1][1*j + 0]= false;
}
if(readWriteSet[1*i + 0][1*j + -1]){
shadow[1*i + 0][1*j + -1].makeValid();
shadow[1*i + 0][1*j + -1].makeInit();
readWriteSet[1*i + 0][1*j + -1]= false;
}
}
}
mapping.LHS_MAP.push_back(std::vector<int>({1,0}));
mapping.LHS_MAP.push_back(std::vector<int>({0,1}));
mapping.RHS_MAP.push_back(std::vector<int>({0,1,0}));
mapping.RHS_MAP.push_back(std::vector<int>({0,0,1}));
mapping.RHS_MAP.push_back(std::vector<int>({0,1,0}));
mapping.RHS_MAP.push_back(std::vector<int>({0,0,1}));
min_bound.instance.resize(3);
min_bound.instance[0] = 0;
min_bound.instance[1] = 1;
min_bound.instance[2] = 1;
min_bound.makeValid();
max_bound.instance.resize(3);
max_bound.instance[0] = 2;
max_bound.instance[1] = 32;
max_bound.instance[2] = 32;
max_bound.makeValid();
wref_fix_flag.push_back(false);
wref_fix_flag.push_back(true);
wref_fix_flag.push_back(true);
read_const.push_back(-1);
read_const.push_back(0);
read_const.push_back(0);
read_const.push_back(-1);
//==========================================================
  Original_Sidel();
  Transformed_Sidel(0,1,32 - 1,1,32 - 1);
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      if (Original_A[i][j] != Transformed_A[i][j]) {
        (std::cout<<"There is a problem in testing") << std::endl;
      }
    }
  }
//====================== Compiler stuff ===================
for(int i=1;i<32;i++){
for(int j=1;j<32;j++){
std::vector<int> wref{1*i + 0,1*j + 0};
assert(shadow[i][j] == polyfunc::lastWriter(max_bound, wref, wref_fix_flag, mapping.LHS_MAP));
}
}
//==========================================================
  return 0;
}
