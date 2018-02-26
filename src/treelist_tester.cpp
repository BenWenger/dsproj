
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "treelist.h"

using namespace std;

static const int iterations = 200;      // number of tests to perform
static const int testsize = 100;        // number of elements in each test

TreeList<int> buildTree(unsigned seed)
{
    srand(seed);

    TreeList<int>   x;
    for(int i = 0; i < testsize; ++i)
        x.insert( rand() );

    return x;
}

void eraseElement(TreeList<int>& x, int index)
{
    auto i = x.begin();
    while(index > 0)
    {
        ++i;
        --index;
    }
    x.erase(i);
}

int main()
{
    srand((unsigned)time(nullptr));

    std::vector<unsigned> seeds;
    seeds.reserve(iterations);
    for(int i = 0; i < iterations; ++i)
        seeds.push_back( rand() );

    for(auto& seed : seeds)
    {
        cout << "Beginning test with seed (" << setw(8) << setfill(' ') << seed << "):  ";
        try
        {
            auto x = buildTree(seed);
            x.validate();

            while(!x.empty())
            {
                eraseElement(x, rand() % x.size());
                x.validate();
            }

            cout << "SUCCESS!" << endl;
        }
        catch(std::exception& e)
        {
            cout << "FAILED: " << e.what() << endl;
            break;
        }
    }

    return 0;
}
