#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

int main(){
    path p("sample");
    if(is_directory(p))
        cout<<"true";
    else
        cout<<"false";
    return 0;
}
