// mpic++ -std=c++11 parallel_tfidf.cpp -lboost_filesystem -lboost_system -lboost_regex
//mpirun -np 4 ./a.out
#include "boost/filesystem.hpp"
#include "mpi.h"
#include <iostream>
#include <queue>
#include "tf.cpp"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
using namespace std;

int main(int argc, char* argv[])
{
  int pid;
  int no_of_process;
  int flag=0;
  
  int no_of_files=0;
  vector<string> name_of_files;
  int size;

  //vector<vector<pair<string, int> >> res;
  //vector<pair<string, int> > temp;
  
  MPI::Init ( argc, argv );
//
//  Get the number of processes.
//
  
  no_of_process = MPI::COMM_WORLD.Get_size ( );
//
//  Get the individual process ID.
//
  pid = MPI::COMM_WORLD.Get_rank ( );
//
//  Process 0 prints an introductory message.
//
  if(pid==0){
  	  queue<string> que;
      que.push("sample");
      while(!que.empty()){
        
        string dir = que.front();
        que.pop();

        DIR *dp;
        struct dirent *dirp;
        if((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
        }

        while ((dirp = readdir(dp)) != NULL) {
          if(((string)dirp->d_name).compare(".")==0||((string)dirp->d_name).compare("..")==0){
            continue;
          }
          cout<<dir+"/"+dirp->d_name<<endl;
          que.push(dir+"/"+string(dirp->d_name));
        }
        closedir(dp);


        //send to process  
        int i=1;
        size=1;
        string buf="";
        if(que.size()>no_of_process){
          size=que.size()/no_of_process;  
        }
        
        while(!que.empty()&&i<=no_of_process){
            int j=0;
            while(j<size){
              buf+=que.front();
              que.pop();
              buf+=";";
              j++;
            }

            MPI::COMM_WORLD.Send(buf.c_str(), buf.length(), MPI::CHAR, pid+i, pid+i);
            i++;
        }
        //send to process end


        //recieve from process
        while(i>0){
	        MPI::Status status;
	        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
	        int l = status.Get_count(MPI::CHAR);
	        char *buf = new char[l];
	        const auto sender = status.Get_source();
	        const auto tag = status.Get_tag();
	        MPI::COMM_WORLD.Recv(buf, l, MPI::CHAR, sender, tag, status);
	        string fname(buf, l);
	        delete [] buf;
	        vector<string> fnames;
	        boost::split(fnames, fname, boost::is_any_of(";"));

	        for(int k=0;k<fnames.size();k++){
	          que.push(fnames[k]);
	        }
	        i--;
	    }
        //recieve from process end and add to queue end

        // name_of_files.push_back(dir->path().filename().string());
        // no_of_files++;
        
      }
    }
  
  else{
  			queue<string> que2;
  			string res="";
          //recieve from parent
  			MPI::Status status2;
	        MPI::COMM_WORLD.Probe(0, pid, status2);
	        int l2 = status2.Get_count(MPI::CHAR);
	        char *buf2 = new char[l2];
	        const auto sender = status2.Get_source();
	        const auto tag = status2.Get_tag();
	        MPI::COMM_WORLD.Recv(buf2, l2, MPI::CHAR, sender, tag, status2);
	        string fname2(buf2, l2);
	        delete [] buf2;
	        vector<string> fnames2;
	        boost::split(fnames2, fname2, boost::is_any_of(";"));

	        for(int i=0;i<fnames2.size();i++){
	          que2.push(fnames2[i]);
	        }
  		  //recieve from parent end
	        while(!que2.empty()){
		        string dir2 = que2.front();
		        que2.pop();
		        DIR *dp;
		        struct dirent *dirp;
		        if((dp = opendir(dir2.c_str())) == NULL) {
		        cout << "Error(" << errno << ") opening " << dir2 << endl;
		        return errno;
		        }

		        while ((dirp = readdir(dp)) != NULL) {
		          if(((string)dirp->d_name).compare(".")==0||((string)dirp->d_name).compare("..")==0){
		            continue;
		          }
		          cout<<dirp->d_name<<endl;
		          res+=dir2+"/"+string(dirp->d_name)+";";
		        }
		        closedir(dp);
		    }
		    
		    //send to parent

			MPI::COMM_WORLD.Send(res.c_str(), res.length(), MPI::CHAR, 0, pid);
			
			//send to parent end
	}

//
//  Terminate MPI.
//
  MPI::Finalize ( );
//
//  Terminate.
//

  return 0;
}