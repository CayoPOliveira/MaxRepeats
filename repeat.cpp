#include <iostream>
#include <unistd.h>
#include <stack>
#include <stxxl/stack>
#include "external/malloc_count/malloc_count.h"

// DEFINES AND MACROS
#ifndef BITSET
#define BITSET 0
#else
#include <bitset>
#endif

#define MIN_LEN_REPEAT1 1
#define MIN_LEN_REPEAT2 1

#define BWTBYTES 1
#define LCPBYTES 2
#define SABYTES 4
#define TOTALBYTES (BWTBYTES + LCPBYTES + SABYTES)
#define MAXREPEATSIZE (1 << (8 * LCPBYTES))

// DNA
#ifdef GENETIC
#define SIGMADNA 7
// Alphabet Elements
uint8_t mapDNA(uint8_t c)
{
    if (c == 'A')
        return 0;
    if (c == 'C')
        return 1;
    if (c == 'G')
        return 2;
    if (c == 'T')
        return 3;
    if (c == 'U')
        return 4;
    if (c == 'N')
        return 5;
    if (c == '$')
        return 6;
}
#endif

// ASCII Size
#define ASCII 128 - 33
#define MODASCI (-32) // First 32 non printable characters

// STDL
using namespace std;

// TYPEDEFS
typedef struct
{
    uint32_t pos;
    uint16_t lcp;
#if BITSET
    bitset<ASCII> B;
#else
    __uint128_t B;
#endif
} Element;

void reset_pair(Element *pair)
{
    pair->pos = pair->lcp = 0;
#if BITSET
    (pair->B).reset();
#else
    pair->B = (__uint128_t)0;
#endif
}

// TIME
void time_start(time_t *t_time, clock_t *c_clock)
{
    *t_time = time(NULL);
    *c_clock = clock();
}

double time_stop(time_t t_time, clock_t c_clock)
{
    double aux1 = (clock() - c_clock) / (double)(CLOCKS_PER_SEC);
    double aux2 = difftime(time(NULL), t_time);
    cout << "CLOCK = " << aux1 << " TIME = " << aux2 << "\n";
    return aux1;
}

// READFILES AND CHECK
uint64_t readFiles(uint64_t N, uint8_t **bwt, FILE *BWT, uint16_t **lcp, FILE *LCP, uint32_t **sa, FILE *SA)
{
    size_t M = fread(*bwt, BWTBYTES, N, BWT);
    if (M != fread(*lcp, LCPBYTES, N, LCP) || M != fread(*sa, SABYTES, N, SA))
        return 0;
    return M;
}

// WRAPPERS
void Fclose(FILE *f)
{
    if (fclose(f) == EOF)
        cout << "Error closing file\n";
}

void Fseek(FILE *stream, long int offset, int origin)
{
    if (fseek(stream, offset, origin) != 0 || ferror(stream))
    {
        cout << "fseek error\n";
        exit(1);
    }
}

FILE *Fopen(char *fname, char *mode)
{
    FILE *file = fopen(fname, mode);
    if (file == NULL)
    {
        cout << "Problem to open " << fname << endl;
        exit(1);
    }
    return file;
}

void *Malloc(uint64_t N)
{
    void *p = malloc(N);
    if (p == NULL)
    {
        cout << "Malloc: Segmentation Fault\n";
        exit(1);
    }
    return p;
}

// MAIN
int main(int argc, char **argv)
{
    extern int optind;
    extern char *optarg;

    // Defaults
    bool verbose = false;
    bool type1 = false;
    bool type2 = false;
    bool stl = false;
    bool stxxl = false;

    uint64_t mem_limit = (uint64_t)50 * 1024 * 1024; // 50MB

    char *str_fname = NULL;
    char *bwt_fname = NULL;
    char *lcp_fname = NULL;
    char *sa_fname = NULL;
    char *output = NULL;

    // Parsing
    int opt;
    while ((opt = getopt(argc, argv, "12hvm:s:o:")) != -1)
    {
        switch (opt)
        {
        case '1':
            // Compute Repeat type 1
            type1 = true;
            break;
        case '2':
            // Compute Repeat type 2
            type2 = true;
            break;
        case 'h':
            // Show help mensage
            return 1;
            break;
        case 'v':
            // Verbose
            verbose = true;
            break;
        case 's':
            // Define if will use stl, stxxl or both, default is stl
            if (optarg[0] == 't' && optarg[1] == 'l')
            {
                stl = true;
            }
            else if (optarg[0] == 't' && optarg[1] == 'x' && optarg[2] == 'x' && optarg[3] == 'l')
            {
                stxxl = true;
            }
            else
            {
                cout << "Use -stl or -stxxl\n";
                return 1;
            }
            break;
        case 'm':
            // Define memory limit usage in MB
            mem_limit = (uint64_t)atoi(optarg) * 1024 * 1024;
            break;
        case 'o':
            // Output
            output = optarg;
            break;
        default:
            cout << "Unknown option " << optarg << ": ABORTING\n";
            return 1;
            break;
        }
    }

    // Defaults
    if (!type1 and !type2)
        type1 = type2 = true;
    if (!stl and !stxxl)
        stl = true;

    // Files
    for (uint8_t i = optind; i < argc; i++)
    {
        if (string(argv[i]).rfind(".lcp") != string::npos)
            lcp_fname = argv[i];
        else if (string(argv[i]).rfind(".bwt") != string::npos)
            bwt_fname = argv[i];
        else if (string(argv[i]).rfind(".sa") != string::npos)
            sa_fname = argv[i];
        else if (string(argv[i]).rfind(".txt") == string(argv[i]).size() - 4)
            str_fname = argv[i];
        else
        {
            cout << "Unknown file type " << argv[i] << ": ABORTING" << endl;
            return 1;
        }
    }

    // Check files
    if (bwt_fname == NULL)
    {
        cout << "Missing .bwt file" << endl;
        return 1;
    }
    if (lcp_fname == NULL)
    {
        cout << "Missing .lcp file" << endl;
        return 1;
    }
    if (sa_fname == NULL)
    {
        cout << "Missing .sa file" << endl;
        return 1;
    }
    if (str_fname == NULL)
    {
        cout << "Missing .txt file" << endl;
        return 1;
    }
    // Output from bwt file name
    if (output == NULL)
    {
        output = bwt_fname;
    }

    // log file
    string log_fname(output);
    log_fname += ".repeats.log";
    FILE *LOG = Fopen((char *)log_fname.c_str(), (char *)"w");

    // Redirect stdout and stderr to a log file
    if (dup2(fileno(LOG), STDOUT_FILENO) == -1 || dup2(fileno(LOG), STDERR_FILENO == -1))
    {
        cout << "Error redirecting the log!!\n";
        exit(1);
    }
    // Desativa o buffer da saída para garantir que cada chamada seja enviada imediatamente
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);
    freopen((char *)log_fname.c_str(), "w", stdout);
    freopen((char *)log_fname.c_str(), "a", stderr);

    cout << "____repeats.log ___\n";

    if (verbose)
    {
        // Executa o programa com os parâmetros especificados
        cout << "Repetitions of Type 1: " << type1 << endl;
        cout << "Repetitions of Type 2: " << type2 << endl;
        cout << "Use STL: " << stl << endl;
        cout << "Use STXXL: " << stxxl << endl;
        cout << "Memory Limit: " << mem_limit << endl;
        cout << "STR file: " << str_fname << endl;
        cout << "BWT file: " << bwt_fname << endl;
        cout << "LCP file: " << lcp_fname << endl;
        cout << "SA file: " << sa_fname << endl;
        cout << "Log: " << log_fname << endl;
    }

    // input check completed start measuring time
    clock_t c_clock = clock();
    time_t t_time = time(NULL);

    cout << "Settings Done!!\nMem: " << malloc_count_peak() << " peak, " << malloc_count_current() << " current\n";
    if (type1)
    {
        if (stl)
        {
            cout << "Starting Type1 with STL!!\nEnding Type1 with STL!!\n";
            time_start(&t_time, &c_clock);

            // FILES: OPEN
            FILE *STRfile = Fopen(str_fname, (char *)"rt");
            FILE *BWTfile = Fopen(bwt_fname, (char *)"rt");
            FILE *LCPfile = Fopen(lcp_fname, (char *)"rb");
            FILE *SAfile = Fopen(sa_fname, (char *)"rb");
            if (verbose)
                cout << "Output repeats type1: " << (string)(output) + ".rt1" << endl;
            FILE *OUTfile = Fopen((char *)((string)(output) + ".rt1").c_str(), (char *)"wt");

            Fseek(BWTfile, 0, SEEK_END);
            uint64_t n = fteel(BWTfile);
            Fseek(BWTfile, 0, SEEK_SET);

            // Number that itens from files
            uint64_t N = (mem_limit - MAXREPEATSIZE) / (BWTBYTES + LCPBYTES + SABYTES); // TOTALBYTES;

            uint8_t *bwt = (uint8_t *)Malloc(N * sizeof(uint8_t));
            uint16_t *lcp = (uint16_t *)Malloc(N * sizeof(uint16_t));
            uint32_t *sa = (uint32_t *)Malloc(N * sizeof(uint32_t));
            char *Repeat1 = (char *)Malloc((MAXREPEATSIZE + 1) * sizeof(char));

            // STACK
            stack<Element> Stack; // Stack from pair <j, lcp[h]> and the bitvector

            Element Rp; // Auxiliar Element
            reset_pair(&Rp);

            // First Read
            N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
            if (!N)
            {
                cout << "Error reading files 1\n";
                Fclose(STRfile);
                Fclose(BWTfile);
                Fclose(LCPfile);
                Fclose(SAfile);
                Fclose(OUTfile);
                exit(1);
            }

            // STACK PART
            while (!feof(STRfile) && !feof(BWTfile) && !feof(LCPfile) && !feof(SAfile))
            {
                for (uint64_t i = 0; i < N; i++)
                {
                    if (Stack.empty() && lcp[i] < MIN_LEN_REPEAT1)
                        continue;

                    if (!Stack.empty())
                    {
                        Rp = Stack.top();
                        // cout << "Rp - sa:" << Rp.pos << ", lcp:" << Rp.lcp << endl;
                    }

                    // Stack empty or
                    // Stack.top().lcp < lcp[i] (new possible repeat)
                    if (MIN_LEN_REPEAT1 <= lcp[i] && (Stack.empty() || Rp.lcp < lcp[i]))
                    {
                        Rp.pos = sa[i];
                        Rp.lcp = lcp[i];
#if BITSET
                        Rp.B.reset();
                        Rp.B.set((int)bwt[i] - 32);
#else
                        Rp.B = (__uint128_t)1 << bwt[i];
#endif
                        Stack.push(Rp);
                    }

                    // Update bitvector case lcp[i]==Stack.top().lcp
                    else if (MIN_LEN_REPEAT1 <= lcp[i] && Rp.lcp == lcp[i])
                    {
                        Stack.pop();
#if BITSET
                        Rp.B.set((int)bwt[i] - 32);
#else
                        Rp.B |= (__uint128_t)1 << bwt[i];
#endif
                        Stack.push(Rp);
                    }

                    // lcp[i] < Stack.top().lcp
                    else if (lcp[i] < Rp.lcp)
                    {
#if BITSET
                        bitset<ASCII> AuxBitSet(Rp.B);
#else
                        __uint128_t AuxBitSet = Rp.B;
#endif
                        // Pop and check the repeats
                        while (!Stack.empty() && lcp[i] < Rp.lcp)
                        {
                            Stack.pop();

                            // Is a repeat Type1 if there at least 2 diferent bwt chars
#if BITSET
                            if (Rp.B.count() > 1)
#else
                            if (Rp.B - (Rp.B & -Rp.B) != (__uint128_t)0) // At least 2 bit sets
#endif
                            {
                                Fseek(STRfile, Rp.pos, SEEK_SET);
                                if (fread(Repeat1, sizeof(char), Rp.lcp, STRfile) != Rp.lcp)
                                {
                                    cout << "Repeat oversized from: " << Rp.pos << ", can't read lcp: " << Rp.lcp << ", doesn't exists\n";
                                    exit(1);
                                }
                                Repeat1[Rp.lcp] = '\0';
                                fprintf(OUTfile, "%s\n", Repeat1);
                            }

                            if (!Stack.empty())
                            {
                                Rp = Stack.top();
                                AuxBitSet |= Rp.B;
                            }
                        }

                        if (!Stack.empty())
                        {
#if BITSET
                            AuxBitSet.set((int)bwt[i] - 32);
#else
                            AuxBitSet |= (__uint128_t)1 << bwt[i];
#endif
                            Rp.B = AuxBitSet;
                            Stack.pop();
                            Stack.push(Rp);
                        }
                        if (MIN_LEN_REPEAT1 <= lcp[i] && (Stack.empty() || Rp.lcp < lcp[i]))
                        {
                            Rp.pos = sa[i];
                            Rp.lcp = lcp[i];
#if BITSET
                            Rp.B.reset();
                            Rp.B.set((int)bwt[i] - 32);
#else
                            Rp.B = (__uint128_t)1 << bwt[i];
#endif
                            Stack.push(Rp);
                        }
                    }
                }

                N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
                if (!N)
                {
                    cout << "Error reading files 2\n";
                    Fclose(STRfile);
                    Fclose(BWTfile);
                    Fclose(LCPfile);
                    Fclose(SAfile);
                    Fclose(OUTfile);
                    exit(1);
                }
            }

            // Remaining Stack Entrys
            while (!Stack.empty())
            {
                Rp = Stack.top();
                Stack.pop();
                // Is a repeat Type1 if there at least 2 diferent bwt chars
#if BITSET
                if (Rp.B.count() > 1)
#else
                if (Rp.B - (Rp.B & -Rp.B) != (__uint128_t)0) // At least 2 bit sets
#endif
                {
                    Fseek(STRfile, Rp.pos, SEEK_SET);
                    if (fread(Repeat1, sizeof(char), Rp.lcp, STRfile) != Rp.lcp)
                    {
                        cout << "Repeat oversized from: " << Rp.pos << ", can't read lcp: " << Rp.lcp << ", doesn't exists\n";
                        exit(1);
                    }
                    Repeat1[Rp.lcp] = '\0';
                    fprintf(OUTfile, "%s\n", Repeat1);
                }
            }

            // HEAP: FREE
            free(bwt);
            free(lcp);
            free(sa);
            free(Repeat1);

            if (feof(BWTfile) && (!feof(LCPfile) || !feof(SAfile)))
                cout << "Remaining data in lcp or sa file\n";

            // Files close
            Fclose(STRfile);
            Fclose(BWTfile);
            Fclose(LCPfile);
            Fclose(SAfile);
            Fclose(OUTfile);

            time_stop(t_time, c_clock);
            cout << "Mem: " << malloc_count_peak() << " peak, " << malloc_count_current() << " current\n";
        }
        if (stxxl)
        {
            cout << "Starting Type1 with STXXL!!\nEnding Type1 with STXXL!!\n";
            time_start(&t_time, &c_clock);

            // Number that itens from files
            uint64_t N = (mem_limit - MAXREPEATSIZE) / (BWTBYTES + LCPBYTES + SABYTES); // TOTALBYTES;

            uint8_t *bwt = (uint8_t *)Malloc(N * sizeof(uint8_t));
            uint16_t *lcp = (uint16_t *)Malloc(N * sizeof(uint16_t));
            uint32_t *sa = (uint32_t *)Malloc(N * sizeof(uint32_t));
            char *Repeat1 = (char *)Malloc((MAXREPEATSIZE + 1) * sizeof(char));

            // STACK
            // template parameter <data_type, externality, behaviour, blocks_per_page, block_size, internal_stack_type, migrating_critical_size, allocation_strategy, size_type>
            // typedef stxxl::STACK_GENERATOR<Element, stxxl::external, stxxl::normal, 4, 4 * 1024, stack<Element>, >::result stxxl_stack;
            // stxxl_stack Stack; // Stack from pair <j, lcp[h]> and the bitvector
            // stack<Element> Stack;

            Element Rp; // Auxiliar Element
            reset_pair(&Rp);

            // FILES: OPEN
            FILE *STRfile = Fopen(str_fname, (char *)"rt");
            FILE *BWTfile = Fopen(bwt_fname, (char *)"rt");
            FILE *LCPfile = Fopen(lcp_fname, (char *)"rb");
            FILE *SAfile = Fopen(sa_fname, (char *)"rb");
            if (verbose)
                cout << "Output repeats type1: " << (string)(output) + ".rt1" << endl;
            FILE *OUTfile = Fopen((char *)((string)(output) + ".rt1").c_str(), (char *)"wt");

            // First Read
            N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
            if (!N)
            {
                cout << "Error reading files 1\n";
                Fclose(STRfile);
                Fclose(BWTfile);
                Fclose(LCPfile);
                Fclose(SAfile);
                Fclose(OUTfile);
                exit(1);
            }

            // STACK PART
            while (!feof(STRfile) && !feof(BWTfile) && !feof(LCPfile) && !feof(SAfile))
            {
                for (uint64_t i = 0; i < N; i++)
                {
                    if (Stack.empty() && lcp[i] < MIN_LEN_REPEAT1)
                        continue;

                    if (!Stack.empty())
                    {
                        Rp = Stack.top();
                        // cout << "Rp - sa:" << Rp.pos << ", lcp:" << Rp.lcp << endl;
                    }

                    // Stack empty or
                    // Stack.top().lcp < lcp[i] (new possible repeat)
                    if (MIN_LEN_REPEAT1 <= lcp[i] && (Stack.empty() || Rp.lcp < lcp[i]))
                    {
                        Rp.pos = sa[i];
                        Rp.lcp = lcp[i];
#if BITSET
                        Rp.B.reset();
                        Rp.B.set((int)bwt[i] - 32);
#else
                        Rp.B = (__uint128_t)1 << bwt[i];
#endif
                        Stack.push(Rp);
                    }

                    // Update bitvector case lcp[i]==Stack.top().lcp
                    else if (MIN_LEN_REPEAT1 <= lcp[i] && Rp.lcp == lcp[i])
                    {
                        Stack.pop();
#if BITSET
                        Rp.B.set((int)bwt[i] - 32);
#else
                        Rp.B |= (__uint128_t)1 << bwt[i];
#endif
                        Stack.push(Rp);
                    }

                    // lcp[i] < Stack.top().lcp
                    else if (lcp[i] < Rp.lcp)
                    {
#if BITSET
                        bitset<ASCII> AuxBitSet(Rp.B);
#else
                        __uint128_t AuxBitSet = Rp.B;
#endif
                        // Pop and check the repeats
                        while (!Stack.empty() && lcp[i] < Rp.lcp)
                        {
                            Stack.pop();

                            // Is a repeat Type1 if there at least 2 diferent bwt chars
#if BITSET
                            if (Rp.B.count() > 1)
#else
                            if (Rp.B - (Rp.B & -Rp.B) != (__uint128_t)0) // At least 2 bit sets
#endif
                            {
                                Fseek(STRfile, Rp.pos, SEEK_SET);
                                if (fread(Repeat1, sizeof(char), Rp.lcp, STRfile) != Rp.lcp)
                                {
                                    cout << "Repeat oversized from: " << Rp.pos << ", can't read lcp: " << Rp.lcp << ", doesn't exists\n";
                                    exit(1);
                                }
                                Repeat1[Rp.lcp] = '\0';
                                fprintf(OUTfile, "%s\n", Repeat1);
                            }

                            if (!Stack.empty())
                            {
                                Rp = Stack.top();
                                AuxBitSet |= Rp.B;
                            }
                        }

                        if (!Stack.empty())
                        {
#if BITSET
                            AuxBitSet.set((int)bwt[i] - 32);
#else
                            AuxBitSet |= (__uint128_t)1 << bwt[i];
#endif
                            Rp.B = AuxBitSet;
                            Stack.pop();
                            Stack.push(Rp);
                        }
                        if (MIN_LEN_REPEAT1 <= lcp[i] && (Stack.empty() || Rp.lcp < lcp[i]))
                        {
                            Rp.pos = sa[i];
                            Rp.lcp = lcp[i];
#if BITSET
                            Rp.B.reset();
                            Rp.B.set((int)bwt[i] - 32);
#else
                            Rp.B = (__uint128_t)1 << bwt[i];
#endif
                            Stack.push(Rp);
                        }
                    }
                }

                N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
                if (!N)
                {
                    cout << "Error reading files 2\n";
                    Fclose(STRfile);
                    Fclose(BWTfile);
                    Fclose(LCPfile);
                    Fclose(SAfile);
                    Fclose(OUTfile);
                    exit(1);
                }
            }

            // Remaining Stack Entrys
            while (!Stack.empty())
            {
                Rp = Stack.top();
                Stack.pop();
                // Is a repeat Type1 if there at least 2 diferent bwt chars
#if BITSET
                if (Rp.B.count() > 1)
#else
                if (Rp.B - (Rp.B & -Rp.B) != (__uint128_t)0) // At least 2 bit sets
#endif
                {
                    Fseek(STRfile, Rp.pos, SEEK_SET);
                    if (fread(Repeat1, sizeof(char), Rp.lcp, STRfile) != Rp.lcp)
                    {
                        cout << "Repeat oversized from: " << Rp.pos << ", can't read lcp: " << Rp.lcp << ", doesn't exists\n";
                        exit(1);
                    }
                    Repeat1[Rp.lcp] = '\0';
                    fprintf(OUTfile, "%s\n", Repeat1);
                }
            }

            // HEAP: FREE
            free(bwt);
            free(lcp);
            free(sa);
            free(Repeat1);

            if (feof(BWTfile) && (!feof(LCPfile) || !feof(SAfile)))
                cout << "Remaining data in lcp or sa file\n";

            // Files close
            Fclose(STRfile);
            Fclose(BWTfile);
            Fclose(LCPfile);
            Fclose(SAfile);
            Fclose(OUTfile);

            time_stop(t_time, c_clock);
            cout << "Mem: " << malloc_count_peak() << " peak, " << malloc_count_current() << " current\n";
        }
    }
    if (type2)
    {
        cout << "Starting Type2!!\nEnding Type2!!\n";
        time_start(&t_time, &c_clock);

        // Number that itens from files
        uint64_t N = (mem_limit - MAXREPEATSIZE) / (BWTBYTES + LCPBYTES + SABYTES); // TOTALBYTES;

        uint8_t *bwt = (uint8_t *)Malloc(N * sizeof(uint8_t));
        uint16_t *lcp = (uint16_t *)Malloc(N * sizeof(uint16_t));
        uint32_t *sa = (uint32_t *)Malloc(N * sizeof(uint32_t));
        char *Repeat2 = (char *)Malloc((MAXREPEATSIZE + 1) * sizeof(char));

        Element pair; // pair<j+1, lcp[j+1]>
        reset_pair(&pair);

        // FILES: OPEN
        FILE *STRfile = Fopen(str_fname, (char *)"rt");
        FILE *BWTfile = Fopen(bwt_fname, (char *)"rt");
        FILE *LCPfile = Fopen(lcp_fname, (char *)"rb");
        FILE *SAfile = Fopen(sa_fname, (char *)"rb");
        if (verbose)
            cout << "Output repeats type2: " << (string)(output) + ".rt2" << endl;
        FILE *OUTfile = Fopen((char *)((string)(output) + ".rt2").c_str(), (char *)"wt");

        // First Read
        N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
        if (!N)
        {
            cout << "Error reading files 1\n";
            Fclose(STRfile);
            Fclose(BWTfile);
            Fclose(LCPfile);
            Fclose(SAfile);
            Fclose(OUTfile);
            exit(1);
        }
        while (!feof(STRfile) && !feof(BWTfile) && !feof(LCPfile) && !feof(SAfile))
        {
            for (uint64_t i = 0; i < N - 1; i++)
            {
                // lcp increased and is a invalid repeat
                if (lcp[i] < lcp[i + 1] && (bwt[i] == bwt[i + 1] || MIN_LEN_REPEAT2 > lcp[i + 1]))
                    reset_pair(&pair);
                // lcp increased and is a valid repeat
                else if (lcp[i] < lcp[i + 1] && (bwt[i] != bwt[i + 1] && MIN_LEN_REPEAT2 <= lcp[i + 1]))
                {
                    pair.pos = sa[i + 1];
                    pair.lcp = lcp[i + 1];
#if BITSET
                    pair.B.reset();
                    pair.B.set(bwt[i] MODASCI);
                    pair.B.set(bwt[i + 1] MODASCI);
#else
                    pair.B = (__uint128_t)(1 << bwt[i] | 1 << bwt[i + 1]);
#endif
                }
                // there is a possible repeat saved pos and lcp != 0
                else if (pair.pos && pair.lcp)
                {
                    // lcp equals
                    if (pair.lcp == lcp[i + 1])
                    {
#if BITSET // bwt[i+1] is not set
                        if (!B.test(bwt[i + 1] MODASCI))
                            B.set(bwt[i + 1] MODASCI);
#else
                        if (!(pair.B & (__uint128_t)(1 << bwt[i + 1])))
                            pair.B |= (__uint128_t)1 << bwt[i + 1];
#endif
                        else // Reset
                            reset_pair(&pair);
                    }
                    // lcp decreased
                    else
                    {
                        Fseek(STRfile, pair.pos, SEEK_SET);
                        if (fread(Repeat2, sizeof(char), pair.lcp, STRfile) != pair.lcp)
                        {
                            cout << "Repeat2 oversized from: " << pair.pos << ", can't read lcp: " << pair.lcp << ", doesn't exists\n";
                            exit(1);
                        }
                        Repeat2[pair.lcp] = '\0';
                        fprintf(OUTfile, "%s\n", Repeat2);
                        reset_pair(&pair);
                    }
                }
            }
            int auxbwt = bwt[N - 1];
            N = readFiles(N, &bwt, BWTfile, &lcp, LCPfile, &sa, SAfile);
            if (N)
            {
                if (pair.lcp < lcp[0] && (auxbwt == bwt[0] || MIN_LEN_REPEAT2 > lcp[0]))
                    reset_pair(&pair);
                else if (pair.lcp < lcp[0] && (auxbwt != bwt[0] && MIN_LEN_REPEAT2 <= lcp[0]))
                {
                    pair.pos = sa[0];
                    pair.lcp = lcp[0];
#if BITSET
                    pair.B.reset();
                    pair.B.set(auxbwt MODASCI);
                    pair.B.set(bwt[0] MODASCI);
#else
                    pair.B = (__uint128_t)(1 << auxbwt | 1 << bwt[0]);
#endif
                }
                else if (pair.pos && pair.lcp)
                {
                    if (pair.lcp == lcp[0])
                    {
#if BITSET
                        if (!B.test(bwt[0] MODASCI))
                            B.set(bwt[0] MODASCI);
#else
                        if (!(pair.B & (__uint128_t)(1 << bwt[0])))
                            pair.B |= (__uint128_t)1 << bwt[0];
#endif
                        else
                            reset_pair(&pair);
                    }
                    else
                    {
                        Fseek(STRfile, pair.pos, SEEK_SET);
                        if (fread(Repeat2, sizeof(char), pair.lcp, STRfile) != pair.lcp)
                        {
                            cout << "Repeat2 oversized from: " << pair.pos << ", can't read lcp: " << pair.lcp << ", doesn't exists\n";
                            exit(1);
                        }
                        Repeat2[pair.lcp] = '\0';
                        fprintf(OUTfile, "%s\n", Repeat2);
                        reset_pair(&pair);
                    }
                }
            }
            else
            {
                cout << "Error reading file 2\n";
                Fclose(STRfile);
                Fclose(BWTfile);
                Fclose(LCPfile);
                Fclose(SAfile);
                Fclose(OUTfile);
                exit(1);
            }
        }
        if (pair.pos && pair.lcp) // Remain pair is a repeat
        {
            Fseek(STRfile, pair.pos, SEEK_SET);
            if (fread(Repeat2, sizeof(char), pair.lcp, STRfile) != pair.lcp)
            {
                cout << "Repeat oversized from: " << pair.pos << ", can't read lcp: " << pair.lcp << ", doesn't exists\n";
                exit(1);
            }
            Repeat2[pair.lcp] = '\0';
            fprintf(OUTfile, "%s\n", Repeat2);
        }

        // HEAP: FREE
        free(bwt);
        free(lcp);
        free(sa);
        free(Repeat2);

        if (!feof(BWTfile) || !feof(LCPfile) || !feof(SAfile))
            cout << "Remaining data in someone file\n";
        // Files close
        Fclose(STRfile);
        Fclose(BWTfile);
        Fclose(LCPfile);
        Fclose(SAfile);
        Fclose(OUTfile);

        time_stop(t_time, c_clock);
        cout << "Mem: " << malloc_count_peak() << " peak, " << malloc_count_current() << " current\n";
    }

    cout << "\n____Finishing repeats.log___\n";
    fclose(LOG);
    return 0;
}
