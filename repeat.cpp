#include <iostream>
#include <getopt.h>
#include <stack>
#include <bitset>
#include <tuple>
#include <map>
#include <stxxl/stack>

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
int main(int argc, char* argv[]) {

    // Parâmetros padrão
    bool type1 = true;
    bool type2 = true;
    bool use_stl = false;
    bool use_stxxl = false;
    int mem_limit = 4096;
    string bwt_file = "";
    string lcp_file = "";

    // Parsing dos argumentos
    while (int opt = getopt(argc, argv, "12stlstxxlm:lcp:bwt:") != -1) {
        switch (opt) {
            case '1':
                type1 = true;
                break;
            case '2':
                type2 = true;
                break;
            case 's':
                if (optarg[0] == 't' && optarg[1] == 'l') {
                    use_stl = true;
                } else if (optarg[0] == 't' && optarg[1] == 'x' && optarg[2] == 'x' && optarg[3] == 'l') {
                    use_stxxl = true;
                } else {
                    cerr << "Unknown option: -" << optopt << endl;
                    return 1;
                }
                break;
            case 'm':
                mem_limit = atoi(optarg);
                break;
            case 'b':
                bwt_file = optarg;
                break;
            case 'l':
                lcp_file = optarg;
                break;
            default:
                cerr << "Unknown option: -" << optopt << endl;
                return 1;
        }
    }

    // Checa se os arquivos bwt e lcp foram especificados
    if (bwt_file.empty() || lcp_file.empty()) {
        cerr << "Missing .bwt or .lcp file" << endl;
        return 1;
    }

    // Executa o programa com os parâmetros especificados
    cout << "Repetitions of Type 1: " << type1 << endl;
    cout << "Repetitions of Type 2: " << type2 << endl;
    cout << "Use STL: " << use_stl << endl;
    cout << "Use STXXL: " << use_stxxl << endl;
    cout << "Memory Limit: " << mem_limit << endl;
    cout << "BWT file: " << bwt_file << endl;
    cout << "LCP file: " << lcp_file << endl;

    // Nome de arquivos padrão com base no arquivo bwt
    size_t dot_pos = bwt_file.find_last_of(".");
    string out_file = bwt_file.substr(0, dot_pos);

    if(type1 && !type2){
        if(use_stl){
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
        if(use_stxxl){

        }
    }
    else if(!type1 && type2){
        if(use_stl){
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
        if(use_stxxl){

        }
    }
    else{ //Type1 and Type2
        if(use_stl){

        }
        if(use_stxxl){

        }
    }
}



