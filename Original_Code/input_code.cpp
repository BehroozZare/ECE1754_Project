#include <iostream>
#include <string>
#include <vector>
#include <cassert>  
#include <math.h>
#include <time.h>
#include <sys/time.h>


#define N 1000
#define T 4
#define PolyCheck_Active

#ifdef PolyCheck_Active
#define D(x) x
#else
#define D(x)
#endif

double read_timer( )
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

struct StateInst{
    int t;
    int i;
    int j;

    bool operator==(const StateInst& rhs) const{
        if(i != rhs.i){
            return false;
        } 
        if(j != rhs.j){
            return false;
        } 
        if(t != rhs.t){
            return false;
        } 
        return true;
    }
    bool operator!=(const StateInst& rhs) const{
        if(*this == rhs){
            return false;
        } else {
            return true;
        }
    }

    bool operator>(const StateInst& rhs) const{
        if(t > rhs.t){
            return true;
        } else if (i > rhs.i){
            return true;
        } else if (j > rhs.j){
            return true;
        } else {
            return false;
        }
    }
};

struct Operand{
    int operandDimention;
    std::string operandName;
    int dataSpace[2];
    int map[3][3];
    void IterationSpaceToDataSpace(StateInst& inst){
        //convert inst to iteration space
        int iterSpace[2];
        iterSpace[0] = inst.i; 
        iterSpace[1] = inst.j;
        //convert iteration space into dataspace using original program mapping 
        dataSpace[0] = map[0][0] * iterSpace[0] + map[0][1] * iterSpace[1] + map[0][2];
        dataSpace[1] = map[1][0] * iterSpace[0] + map[1][1] * iterSpace[1] + map[1][2];
    }
};

struct Statement{
    int id;
    StateInst inst;
    std::vector<Operand> rhs;
    Operand lhs;

    bool operator==(const Statement& rhs) const{
        if(id != rhs.id){
            return false;
        } else if (inst != rhs.inst){
            return false;
        }
        return true;
    }

    bool operator!=(const Statement& rhs) const{
        if(*this == rhs){
            return false;
        } else {
            return true;
        }
    }    
};

// ======= Global Variables ======= 
std::vector<Statement> OriginalStatemetns;

Statement InvalidStatement;
Statement InitStatement;

//Data space
int A[N][N];
int ACopy[N][N];
// Shadow variable for ACopy
Statement Shadow[N][N];
// ================================ 
// Auxiliary functions
//Initialize the maping and the  operand of a statment based on its id
void Statement_initalizer(Statement& S){
    if(S.id == 1){
        S.lhs.operandDimention = 2;
        S.lhs.operandName = "A";
        S.rhs.resize(2);
        S.rhs[0].operandDimention = 2;
        S.rhs[1].operandDimention = 2;
        S.rhs[0].operandName = "A";
        S.rhs[1].operandName = "A";

        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                if(i == j){
                    S.lhs.map[i][j] = 1;
                    S.rhs[0].map[i][j] = 1;
                    S.rhs[1].map[i][j] = 1;
                } else {
                    S.lhs.map[i][j] = 0;
                    S.rhs[0].map[i][j] = 0;
                    S.rhs[1].map[i][j] = 0;
                }
            }
        }
        S.rhs[0].map[0][2] = -1;
        S.rhs[1].map[1][2] = -1;
    } else if (S.id == 0){
        S.inst.i = -1;
        S.inst.j = -1;
        S.inst.t = -1;
    }
}
// PolyCheck toy functions
// First write in a specific address of A
// Note that we receive an address from optimized program
// and translate the address into iteration space of the original program
//(There is no need to do static analysis of data-flow and original scheduler for the optimized program)
Statement FirstWriter(int i, int j){
    if(i == 0 || j == 0){
        return InvalidStatement;
    }
    Statement Sfirst;
    Sfirst.id = 1;
    Statement_initalizer(Sfirst);
    Sfirst.inst.i = i;
    Sfirst.inst.j = j;
    Sfirst.inst.t = 0;
    return Sfirst;
}

//Next statement instance that write into this location
Statement NextWriter(Statement prev, int i, int j){
    // Iterate over all the statemetns related to the rhs A
    // and find the NextWrite of all that statement based on prev
    // Then it will compute a lexmin between all those instances 
    if(prev == InitStatement){
        prev.id = 1;
        Statement_initalizer(prev);
    }
    prev.inst.t++;
    if(prev.inst.t < T){
        return prev;
    } else {
        return InvalidStatement;
    }

}

Statement WriteBeforeRead(Statement current, int i, int j){
    if(i == 0 || j == 0){
        return InitStatement;   
    }

    Statement output;
    output.id = 1;
    Statement_initalizer(output);
    output.inst.i = i;
    output.inst.j = j;
    output.inst.t = current.inst.t;

    if(current.inst > output.inst){
        return output;
    } else {
        output.inst.t--;
        if(output.inst.t >= 0){
            return output;
        } else {
            return InvalidStatement;
        }
    }
        
}

Statement LastWriter(int i, int j){
    if(i == 0 || j == 0){
        return InitStatement;
    }

    Statement S;
    S.id = 1;
    Statement_initalizer(S);
    S.inst.i = i;
    S.inst.j = j;
    S.inst.t = T - 1;
    return S;
}

// === Debugging functions ===
void print_state(Statement& state){
    if(state.id == 0){
        std::cout << "Init Statements" << std::endl;
        return;
    } else if (state.id == -1){
        std::cout << "Invalid Statements" << std::endl;
    } else {
        std::cout << "S" << state.id << "<0 , t="  << state.inst.t << ", 0, i=" << state.inst.i <<
                ", 0, j=" << state.inst.j << ">" << std::endl;

        std::cout << "lhs "<< state.lhs.operandName << " DataSpace: (" 
                << state.lhs.dataSpace[0] << ", " << state.lhs.dataSpace[1] << ")" << std::endl;
        std::cout << "rhs 0 " << state.rhs[0].operandName << " DataSpace: (" <<
                state.rhs[0].dataSpace[0] << ", " << state.rhs[0].dataSpace[1] << ")" << std::endl;
        std::cout << "rhs 1 " << state.rhs[1].operandName << " DataSpace: (" <<
                state.rhs[1].dataSpace[0] << ", " << state.rhs[1].dataSpace[1] << ")" << std::endl;
    }
}
// ===========================

// ==== Simulate static analysis ====
// Generate FirstWriter
// Generate NextWriter
// Generate WriteBeforeRead
// Generate LastWriter
// Define WriteSet
// Define InputSet
// Define Statemetns Set
// ============================
void origSidel_Static_Sumulation(){
    // Extract statements from AST
    Statement S;
    // Exrtacts tokens from the statemetn string and do these things automatically
    S.id = 1;
    Statement_initalizer(S);
    OriginalStatemetns.push_back(S);
}

void origSidel(){
    D(origSidel_Static_Sumulation();)
    for(int t = 0; t < T; t++){
        for(int i = 1; i < N; i++){
            for(int j = 1; j < N; j++){
                A[i][j] = A[i - 1][j] + A[i][j - 1];
            }
        }
    }
}
//Break the  2d array into 4 sub-region. Then traverse trough the top-left, top-right, bottom-left bottom-right
//For example:
// ------------------------------
//|  1.1 |  1.2  |  2.1 |  2.2  |
// ------------------------------
//|  1.3 |  1.4  |  2.3 |  2.4  |
/// -----------------------------
//|  3.1 |  3.2  |  4.1 |  4.2  |
// ------------------------------
//|  3.3 |  3.4  |  4.3 |  4.4  |
/// -----------------------------

void recursiveSidel(int t,int ilo,int ihi, int jlo, int jhi){
    if (ilo > ihi || jlo > jhi) return;
    if (ilo == ihi && jlo == jhi){
        ACopy[ilo][jlo] = ACopy[ilo-1][jlo] + ACopy[ilo][jlo-1];
        // ACopy[ilo][jlo] = ACopy[ilo][jlo-1] + ACopy[ilo-1][jlo];
        D(
        //===== Insert the checker code after static analysis =====
        // Note that the paper skip this part in its example, because 
        // finding the FirstWriter and NextWriter is actually depends on number of statemetns
        // in the original and optimized code. So there should be a way to match an statement in
        // The optimized code to a statement in Original Code (They didn't explain this part)
        // They also check the operator too, but I omit that part because I need parsing and syntax reading
        Statement optStat;
        if(Shadow[ilo][jlo] == InvalidStatement){
            optStat = FirstWriter(ilo, jlo);
        } else {
            optStat = NextWriter(Shadow[ilo][jlo], ilo, jlo);
        }
        optStat.lhs.IterationSpaceToDataSpace(optStat.inst);
        assert(optStat.lhs.operandName == "A");
        assert(optStat.lhs.dataSpace[0] == ilo);
        assert(optStat.lhs.dataSpace[1] == jlo);
        //Checking the right hand side (They also check the name too)
        optStat.rhs[0].IterationSpaceToDataSpace(optStat.inst);
        assert(optStat.rhs[0].operandName == "A");
        assert(optStat.rhs[0].dataSpace[0] == (ilo - 1));
        assert(optStat.rhs[0].dataSpace[1] == jlo);
        // assert(optStat.rhs[0].dataSpace[0] == ilo);
        // assert(optStat.rhs[0].dataSpace[1] == (jlo - 1));
        optStat.rhs[1].IterationSpaceToDataSpace(optStat.inst);
        assert(optStat.rhs[1].operandName == "A");
        assert(optStat.rhs[1].dataSpace[0] == ilo);
        assert(optStat.rhs[1].dataSpace[1] == (jlo - 1));
        // assert(optStat.rhs[1].dataSpace[0] == (ilo - 1));
        // assert(optStat.rhs[1].dataSpace[1] == jlo);
        //Checking the validity of the value of rhs arrays
        assert(Shadow[ilo - 1][jlo] == WriteBeforeRead(optStat, ilo - 1, jlo)); 
        assert(Shadow[ilo][jlo - 1] == WriteBeforeRead(optStat, ilo, jlo - 1));
        Shadow[ilo][jlo] = optStat;
        //=======================================================
        )

    }else{
        recursiveSidel(t, ilo,(ilo + ihi) / 2, jlo, (jlo + jhi)/2); //Top-Left
        recursiveSidel(t, ilo,(ilo + ihi) / 2, (jlo + jhi) / 2 + 1,jhi); //Top-Right
        recursiveSidel(t, (ilo + ihi) / 2 + 1, ihi, jlo, (jlo + jhi) / 2); //Bottom-Left
        recursiveSidel(t, (ilo + ihi) / 2 + 1, ihi, (jlo + jhi)/2 + 1, jhi); //Bottom-Right
    }

    if (ilo == 1 && ihi == N-1 && jlo == 1 && jhi == N-1 && t < (T)){ // The naive poly chech find this bug
        recursiveSidel(t+1, ilo, ihi, jlo, jhi);// Next time step
    }
}

int main() {
    
    //Generate some random data for Seidel
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            A[i][j] = rand()%100;
            ACopy[i][j] = A[i][j];
            // ACopy[i][j] = A[i][j] = 0;
        }
    }

    // ===== Some pre-initilization and static analysis definition =====
    InvalidStatement.id = -1;
    Statement_initalizer(InvalidStatement);
    InitStatement.id = 0;
    Statement_initalizer(InitStatement);
    //Initialize Shadow variables
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(i == 0 || j == 0){//Read before write set
                Shadow[i][j] = InitStatement;
            } else { //
                Shadow[i][j] = InvalidStatement; // Write first set
            }
        }
    }
    // =================================================================
    double simulation_time = read_timer( );
    origSidel();
    simulation_time = read_timer( ) - simulation_time;
    std::cout << "Iterative Sidel Time:" << simulation_time << std::endl;

    simulation_time = read_timer( );
    recursiveSidel(0, 1, N - 1, 1, N - 1);
    // the last part of PolyCheck
    D(for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            // std::cout << i << "\t" << j << std::endl;
            assert(Shadow[i][j] == LastWriter(i,j));
            // assert(InvalidStatement == InitStatement);
        }
    })
    simulation_time = read_timer( ) - simulation_time;
    std::cout << "Recursive Sidel Time:" << simulation_time << std::endl;

    // Check whether the two results are equal.
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(A[i][j] != ACopy[i][j])
                std::cout << "There is a problem in testing" << std::endl;
        }
    }

    return 0;
}