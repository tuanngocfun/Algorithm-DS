#include <iostream>
#include <string>
#include "Hashtable.hpp"

using namespace std;
using namespace Hashtable;

int main()
{
    cout.setf(ios_base::boolalpha);

    Hashtable_Chaining<string> table_1 = {"Hello", "World!", "Data", "structure", "Algorithm"};
    table_1.insert("and");
    cout << table_1.search("World") << endl;
    table_1.display();

    cout << endl;
    system("pause");
    cout << endl;

    Hashtable_Probing<string> table_2 = {"Hello", "World!", "Data", "structure", "Algorithm"};
    table_2.insert("and");
    cout << table_2.search("World!") << endl;
    table_2.display();

    return 0;
}
