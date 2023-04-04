#include <iostream>
#include <unistd.h>
#include <stack>
#include <bitset>
#include <tuple>
#include <map>
#include <stxxl/stack>
#include "external/egap/malloc_count/malloc_count.h"

// STDL
using namespace std;

// DEFINES AND MACROS
#define MIN_LEN_REPEAT1 1
#define MIN_LEN_REPEAT2 1

// Alphabet Size
#define SIGMA_N 6 //Sigma.size()
// Alphabet Elements
map <char, int> Sigma = {
  {'$', 0},
  {'A', 1},
  {'C', 2},
  {'G', 3},
  {'T', 4},
  {'N', 5}
};

// Main
int main(int argc, char** argv) {

    // Parâmetros padrão
    bool type1 = false;
    bool type2 = false;
    bool stl = false;
    bool stxxl = false;
    int mem_limit = 4096;
    string bwt_file = "";
    string lcp_file = "";
    string sa_file = "";
    string output = "";

    // Parsing dos argumentos
    int opt;
    while((opt = getopt(argc, argv, "12hms::o:")) != -1) {
        switch (opt) {
            case '1':
                //Compute Repeat type 1
                type1 = true;
                break;
            case '2':
                //Compute Repeat type 2
                type2 = true;
                break;
            case 'h':
                // Show help mensage
                return 1;
                break;
            case 's':
                // Define if will use stl, stxxl or both, default is stl
                if (optarg[0] == 't' && optarg[1] == 'l') {
                    stl = true;
                } else if (optarg[0] == 't' && optarg[1] == 'x' && optarg[2] == 'x' && optarg[3] == 'l') {
                    stxxl = true;
                } else {
                    cerr << "Unknown option: -" << optopt << endl;
                    return 1;
                }
                break;
            case 'm':
                // Define memory limit usage
                mem_limit = atoi(optarg);
                break;
            case 'o':
                // Output
                output = optarg;
                break;
            default:
                cerr << "Unknown option " << optarg << ": ABORTING" << endl;
                // return 1;
                break;
        }
    }

    //Defaults
    if(!type1 and !type2) type1 = type2 = true;
    if(!stl and !stxxl) stxxl = true;


    // Files
    for (int i = optind; i < argc; i++) {
        if (string(argv[i]).ends_with(".lcp")) {
            lcp_file = argv[i];
        } else if (string(argv[i]).ends_with(".bwt")) {
            bwt_file = argv[i];
        } else if (string(argv[i]).ends_with(".sa")) {
            sa_file = argv[i];
        } else {
            cerr << "Unknown file " << argv[i] << ": ABORTING" << endl;
            return 1;
        }
    }

    // Check files
    if (bwt_file.empty() || lcp_file.empty() || sa_file.empty()) {
        if(bwt_file.empty()) cerr << "Missing .bwt file" << endl;
        if(lcp_file.empty()) cerr << "Missing .lcp file" << endl;
        if(sa_file.empty()) cerr << "Missing .sa file" << endl;
        return 1;
    }

    // Output from bwt file name
    if(output.empty()){
        size_t dot_pos = bwt_file.find_last_of(".");
        output = bwt_file.substr(0, dot_pos);
    }

    // Executa o programa com os parâmetros especificados
    cout << "Repetitions of Type 1: " << type1 << endl;
    cout << "Repetitions of Type 2: " << type2 << endl;
    cout << "Use STL: " << stl << endl;
    cout << "Use STXXL: " << stxxl << endl;
    cout << "Memory Limit: " << mem_limit << endl;
    cout << "BWT file: " << bwt_file << endl;
    cout << "LCP file: " << lcp_file << endl;
    cout << "SA file: " << sa_file << endl;
    cout << "Output: " << output << endl;


    //if(type1 && !type2){
    if(type1){
        if(stl){
            return 0;
            //FALTA: Abrir o arquivo e pegar a bwt
            char* bwt = NULL;
            //FALTA: Abrir o arquivo e pegar a lcp
            int* lcp = NULL;
            int n = 0;

            stack< pair<int, int> > repeats; // Return subsequences repeats type 1

            typedef struct element{
                int pos;
                int lcp;
                bitset<SIGMA_N> B;
            } Element;

            Element Rp; //Auxiliar Element
            stack<Element> Stack; //Stack from pair <j, lcp[h]> and the bitvector

            for (int i = 0; i < n; i++)
            {
                //Is a valid repeat
                if(lcp[i]>=MIN_LEN_REPEAT1){
                if(!Stack.empty()) Rp = Stack.top();

                // Stack empty or
                // Stack.top().lcp < lcp[i] (new possible repeat)
                if (Stack.empty() || Rp.lcp < lcp[i])
                {
                    Rp.pos = i;
                    Rp.lcp = lcp[i];
                    Rp.B.reset();
                    Rp.B.set(Sigma[bwt[i]]);

                    Stack.push(Rp);
                    continue;
                }

                //Update bitvector case lcp[i]==Stack.top().lcp
                if (Rp.lcp == lcp[i]){
                    Stack.pop();
                    Rp.B.set(Sigma[bwt[i]]);
                    Stack.push(Rp);
                    continue;
                }
                }

                // lcp[i] < Stack.top().lcp
                // Pop and check the repeats
                while (!Stack.empty() && lcp[i] < Rp.lcp)
                {
                    Stack.pop();
                    // Is a repeat Type1 if there more than one diferent char
                    if(Rp.B.count() > 1)
                        repeats.push({Rp.pos, Rp.lcp});

                    if(!Stack.empty()){
                        bitset<SIGMA_N> AuxBitSet(Rp.B);
                        Rp = Stack.top();
                        Rp.B |= AuxBitSet;
                    }
                }

                if(lcp[i]>=MIN_LEN_REPEAT1){
                // Case is a new repeat
                if (Stack.empty())
                {
                    Rp.pos = i;
                    Rp.lcp = lcp[i];
                    Rp.B.reset();
                    Rp.B.set(Sigma[bwt[i]]);

                    Stack.push(Rp);
                    continue;
                }
                // Case remain a pair with Stack.top().lcp < lcp[i]
                else if (Rp.lcp < lcp[i])
                {
                    Rp.B.set(Sigma[bwt[i]]);
                    Rp.lcp = lcp[i];

                    Stack.push(Rp);
                }
                }
            }

            //FALTA: Abrir o arquivo para salvar as repetições
        }
        if(stxxl){

        }
    }
    //else if(!type1 && type2){
    if(type2){
        if(stl){
            return 0;
            //FALTA: Abrir o arquivo e pegar a bwt
            char* bwt = NULL;
            //FALTA: Abrir o arquivo e pegar a lcp
            int* lcp = NULL;
            int n = 0;

            stack< pair<int, int> > repeats;// Return subsequences repeats type 2

            pair<int, int> pair = {-1, -1}; // pair<j+1, lcp[j+1]>
            bitset<SIGMA_N> B;              // bitvector

            for(int i=0; i<n-1; i++){
                //New possible repeat
                if(pair.first == -1){
                if(lcp[i]<lcp[i+1] && lcp[i+1]>MIN_LEN_REPEAT2 && bwt[i]!=bwt[i+1]){
                    pair = {i, lcp[i]};
                    B.reset();
                    B.set(Sigma[bwt[i]]);
                    B.set(Sigma[bwt[i+1]]);
                }
                continue;
                }

                //Better repeat
                if( pair.second < lcp[i+1] ){
                pair.first = i+1;
                pair.second = lcp[i+1];
                B.reset();
                B.set(Sigma[bwt[i]]);
                B.set( Sigma[bwt[i+1]] );
                }
                else if( pair.second == lcp[i+1] ){
                //bwt[i+1] is not set
                if( !B.test( Sigma[ bwt[i+1] ] ) )
                    B.set( Sigma[ bwt[i+1] ] );
                //Reset
                else pair.first = -1;
                }
                else{ //pair.second > lcp[i+1];
                repeats.push(pair);
                }
            }

            //FALTA: Abrir o arquivo para salvar as repetições
        }
        if(stxxl){

        }
    }
    else{ //Type1 and Type2
        if(stl){

        }
        if(stxxl){

        }
    }

    return 0;
}



