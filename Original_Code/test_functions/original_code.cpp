//#include <random>
//
////==================== Compiler stuff ====================
//#include "polycheck_demo_functions.h"


#define T 2
#define N 32

double Original_A[N][N];
double Transformed_A[N][N];


void Transformed_Sidel(int t,int ilo,int ihi, int jlo, int jhi){
    if (ilo > ihi || jlo > jhi) return;
    if (ilo == ihi && jlo == jhi){
        Transformed_A[ilo][jlo] = Transformed_A[ilo-1][jlo] + Transformed_A[ilo][jlo-1];
    }else{
        Transformed_Sidel(t, ilo,(ilo + ihi) / 2, jlo, (jlo + jhi)/2); //Top-Left
        Transformed_Sidel(t, ilo,(ilo + ihi) / 2, (jlo + jhi) / 2 + 1,jhi); //Top-Right
        Transformed_Sidel(t, (ilo + ihi) / 2 + 1, ihi, jlo, (jlo + jhi) / 2); //Bottom-Left
        Transformed_Sidel(t, (ilo + ihi) / 2 + 1, ihi, (jlo + jhi)/2 + 1, jhi); //Bottom-Right
    }

    if (ilo == 1 && ihi == N - 1 && jlo == 1 && jhi == N-1 && t < (T - 1)){ // The naive poly chech find this bug
        Transformed_Sidel(t+1, ilo, ihi, jlo, jhi);// Next time step
    }
}


void Original_Sidel(){
    for(int t = 0; t < T; t++){
        for(int i = 1; i < N; i++){
            for(int j = 1; j < N; j++){
                Original_A[i][j] = Original_A[i - 1][j] + Original_A[i][j - 1];
            }
        }
    }
}



int main() {
    //=========================== Test Section ======================
    //Generate some random data for Seidel
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            Original_A[i][j] = dist(mt);
            Transformed_A[i][j] = Original_A[i][j];
        }
    }
    Original_Sidel();
    Transformed_Sidel(0, 1, N - 1, 1, N - 1);
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(Original_A[i][j] != Transformed_A[i][j])
                std::cout << "There is a problem in testing" << std::endl;
        }
    }
   return 0;
}


