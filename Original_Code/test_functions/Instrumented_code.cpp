#include <random>

//==================== Compiler stuff ====================
#include "polycheck_demo_functions.h"


#define T 2
#define N 32


double Original_A[N][N];
double Transformed_A[N][N];


//====================== Compiler stuff ===================
double Instrument_A[N][N];
polyfunc::StateInst shadow[N][N];
polyfunc::ExpressionMapping mapping;
polyfunc::StateInst min_bound;
polyfunc::StateInst max_bound;
std::vector<bool> wref_fix_flag;
std::vector<int> read_const;
//=========================================================


void Instrument_Sidel(int t,int ilo,int ihi, int jlo, int jhi){
    if (ilo > ihi || jlo > jhi) return;
    if (ilo == ihi && jlo == jhi){
        Instrument_A[ilo][jlo] = Instrument_A[ilo-1][jlo] + Instrument_A[ilo][jlo-1];
        // ACopy[ilo][jlo] = ACopy[ilo][jlo-1] + ACopy[ilo-1][jlo];
        //===== Insert the checker code after static analysis =====
        // Note that the paper skip this part in its example, because
        // finding the FirstWriter and NextWriter is actually depends on number of statemetns
        // in the original and optimized code. So there should be a way to match an statement in
        // The optimized code to a statement in Original Code (They didn't explain this part)
        // They also check the operator too, but I omit that part because I need parsing and syntax reading
        std::vector<int> wref{ilo, jlo};
        polyfunc::StateInst optStat;
        if(shadow[ilo][jlo].isInit() || !shadow[ilo][jlo].isValid()){
            optStat = polyfunc::firstWriter(min_bound, wref, wref_fix_flag, mapping.LHS_MAP);
            assert(optStat.isValid());
        } else {
            optStat = polyfunc::nextWriter(shadow[ilo][jlo], max_bound, wref, wref_fix_flag, mapping.LHS_MAP);
            assert(optStat.isValid());
        }

        //Checking the right hand side
        std::vector<int> read_ref{ilo - 1, jlo, ilo, jlo - 1};
        for(int rr_ptr = 0; rr_ptr < read_ref.size(); rr_ptr++){
            assert(polyfunc::dotProduct(mapping.RHS_MAP[rr_ptr], optStat.instance) + read_const[rr_ptr] == read_ref[rr_ptr]);
        }

        //Checking the validity of the value of rhs arrays
        std::vector<int> read_ref0(read_ref.begin(), read_ref.begin() + 2);
        assert(shadow[ilo - 1][jlo] == polyfunc::writeBeforeRead(optStat, min_bound, read_ref0, wref_fix_flag, mapping.LHS_MAP));
        std::vector<int> read_ref1(read_ref.begin() + 2, read_ref.begin() + 4);
        assert(shadow[ilo][jlo - 1] == polyfunc::writeBeforeRead(optStat, min_bound, read_ref1, wref_fix_flag, mapping.LHS_MAP));
        shadow[ilo][jlo] = optStat;
        //=======================================================


    }else{
        Instrument_Sidel(t, ilo,(ilo + ihi) / 2, jlo, (jlo + jhi)/2); //Top-Left
        Instrument_Sidel(t, ilo,(ilo + ihi) / 2, (jlo + jhi) / 2 + 1,jhi); //Top-Right
        Instrument_Sidel(t, (ilo + ihi) / 2 + 1, ihi, jlo, (jlo + jhi) / 2); //Bottom-Left
        Instrument_Sidel(t, (ilo + ihi) / 2 + 1, ihi, (jlo + jhi)/2 + 1, jhi); //Bottom-Right
    }

    if (ilo == 1 && ihi == N - 1 && jlo == 1 && jhi == N-1 && t < (T - 1)){ // The naive poly chech find this bug
        Instrument_Sidel(t+1, ilo, ihi, jlo, jhi);// Next time step
    }
}

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
    //======================= Compiler stuff =======================
    std::vector<std::vector<bool>> readWriteSet(N, std::vector<bool>(N, true));//True for read set and False for the write set

    //Initialize Shadow variables
    for(int i = 1; i < N; i++){
        for(int j = 1; j < N; j++){
            //WriteSet
            if(readWriteSet[i][j]){//Write ref
                shadow[i][j].invalidate();
                shadow[i][j].makeNoInit();
                readWriteSet[i][j] = false;
            }

            if(readWriteSet[i - 1][j]){//Read ref
                shadow[i - 1][j].makeValid();
                shadow[i - 1][j].makeInit();
                readWriteSet[i - 1][j] = false;
            }

            if(readWriteSet[i][j - 1]){//Read ref
                shadow[i][j - 1].makeValid();
                shadow[i][j - 1].makeInit();
                readWriteSet[i][j - 1] = false;
            }
        }
    }

    //Initialize Mapping
    mapping.LHS_MAP.push_back(std::vector<int> {1, 0}); // Mapping for first subscript
    mapping.LHS_MAP.push_back(std::vector<int> {0, 1}); // Mapping for second subscript

    mapping.RHS_MAP.push_back(std::vector<int>({0, 1, 0}));
    mapping.RHS_MAP.push_back(std::vector<int>({0, 0, 1}));
    mapping.RHS_MAP.push_back(std::vector<int>({0, 1, 0}));
    mapping.RHS_MAP.push_back(std::vector<int>({0, 0, 1}));

    //Compute Bounds
    min_bound.instance.resize(3);
    min_bound.instance[0] = 0;
    min_bound.instance[1] = 1;
    min_bound.instance[2] = 1;
    min_bound.makeValid();

    max_bound.instance.resize(3);
    max_bound.instance[0] = T;
    max_bound.instance[1] = N;
    max_bound.instance[2] = N;
    max_bound.makeValid();

    //Compute wref related invariants
    wref_fix_flag.push_back(false);
    wref_fix_flag.push_back(true);
    wref_fix_flag.push_back(true);

    //Extract constants from the read reference
    read_const.push_back(-1);
    read_const.push_back(0);
    read_const.push_back(0);
    read_const.push_back(-1);
    //=========================== Test Section ======================
    //Generate some random data for Seidel
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            Original_A[i][j] = dist(mt);
            Transformed_A[i][j] = Original_A[i][j];
            Instrument_A[i][j] = Original_A[i][j];
        }
    }
    Original_Sidel();
    Transformed_Sidel(0, 1, N - 1, 1, N - 1);
    Instrument_Sidel(0, 1, N - 1, 1, N - 1);
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(Original_A[i][j] != Transformed_A[i][j])
                std::cout << "There is a problem in testing" << std::endl;
        }
    }

    //======================= Compiler stuff =======================
    //Last Writer Stuff Mapping
    for(int i = 1; i < N; i++){
        for(int j = 1; j < N; j++){
            std::vector<int> wref{i,j};
            assert(shadow[i][j] == polyfunc::lastWriter(max_bound, wref, wref_fix_flag, mapping.LHS_MAP));
        }
    }
    //================================================================


   return 0;
}


