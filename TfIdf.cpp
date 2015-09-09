//-g++ -std=c++11 TfIdf.cpp -lboost_filesystem -lboost_system -lboost_regex
//#include "mpi.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include "tf.cpp"
using namespace std;

int main () {

  int no_of_files=0;
  vector<string> name_of_files;
  vector<vector<pair<string, int> >> res;
  vector<pair<string, int> > temp;
  for ( boost::filesystem::recursive_directory_iterator end, dir("sample");dir != end; ++dir ) {
<<<<<<< HEAD
    std::cout << dir->path().string() << "\n"; 
=======
    std::cout << dir->path().string() << "\n";
>>>>>>> 68fa29a3b3b0aabb39372fa22bfc1880c192b458
	temp=getKeywords(dir->path().string());
	res.push_back(temp);
	name_of_files.push_back(dir->path().filename().string());
	no_of_files++;
  }
  int count;
  int arr[no_of_files][no_of_files];
  for(int i=0;i<no_of_files;i++){
  	for(int j=0;j<no_of_files;j++){
  		arr[i][j]=0;
  	}
  }


  float percent;
  cout<<"what %age of keywords you want to match";
  cin>>percent;
  percent=percent/100;


  for(int i=0;i<res.size();i++){
  	unordered_map<string,int> findkeys(res[i].begin(),res[i].end());
  	for(int j=i+1;j<res.size();j++){
  		count=0;
  		float limit=res[j].size()*percent;
  		for(int k=0;k<limit;k++){
  			if(findkeys[res[j][k].first]==1){
  				count++;
  			}
  		}
  		arr[i][j]=count;
  		arr[j][i]=count;
  	}
  }


  for(int i=0;i<no_of_files;i++){
  	for(int j=0;j<no_of_files;j++){
  		cout<<arr[i][j]<<"  ";
  	}
  	cout<<endl;
  }

  for(int i=0;i<name_of_files.size();i++){
  	cout<<name_of_files[i]<<endl;
  }

  cout<<"enter the name of the file";
  string name;
  int pos;
  cin>>name;

  for(int i=0;i<name_of_files.size();i++){
  	if(name.compare(name_of_files[i])==0){
  		pos=i;
  		break;
  	}
  }

  for(int i=0;i<no_of_files;i++){
  	if(arr[pos][i]!=0){
  		cout<<name_of_files[i]<<" ";
  	}
  }
  cout<<endl;
  cout<<"enter the file name for which you want to find transitive closure";
  string fname;
  cin>>fname;

  for(int i=0;i<name_of_files.size();i++){
  	if(name.compare(name_of_files[i])==0){
  		pos=i;
  		break;
  	}
  }
  cout<<endl<<"the transitive closure of the file is";
  for(int i=0;i<no_of_files;i++){
  	if(arr[pos][i]!=0){
	  		cout<<name_of_files[i]<<" ";
	  		continue;
	  	}
  	for(int j=0;j<no_of_files;j++){
	  	if(arr[pos][j]!=0&&arr[j][i]!=0){
	  		cout<<name_of_files[i]<<" ";
	  		break;
	  	}
	  }
  }
  cout<<endl;

  return 0;
}
