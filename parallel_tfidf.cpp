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
#include <cctype>
#include <cmath>

using namespace std;
using namespace boost::filesystem;

int main(int argc, char* argv[])
{
  int pid; //For rank of current process
  int no_of_process; //To find the total number of processes
  int size; //Size of processes to be allocated for each process.

  //Initializing the MPI environment
  MPI::Init ( argc, argv );

  //Getting the number of processes
  no_of_process = MPI::COMM_WORLD.Get_size();
  //Handling if run as a single application.
  if(no_of_process<2){
    cout<<"\n ERROR: You'll need atleast 2 processes to run this application.\n\n";
    MPI_Finalize();
    return 0;
  }

  //Get the process ID
  pid = MPI::COMM_WORLD.Get_rank();

  // Process ID 0 => Initial Process
  if(pid==0){
  	  queue<string> que;
      que.push("dwarak");

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
        //cout<<"\n\n Parent Queue:";
        //cout<<"\n"<<dir+"/"+dirp->d_name<<endl;

        // BoostFileSystem Declaration
        // path file_path(dir+"/"+dirp->d_name);
        // if (is_directory(file_path)) //Push elements into the queue only if they are directories
        que.push(dir+"/"+string(dirp->d_name)); //If only this statement is present, we push all the files into the queue
      }
      closedir(dp);


      while(!que.empty()){



        // ======== FUNCTION TO PRINT QUEUE VALUES ========

        queue<string> que3;
        que3=que;
        cout<<"\n\n PARENT Queue : "<<endl;
        //Temp function to print the value of the queue
        while(!que3.empty()){
          cout<<que3.front()<<endl;
          que3.pop();
        }

        /*
        exit(0);
        */

        //cout<<"\n ####### PARENT : QUEUE SIZE BEFORE SENDING"<<que.size();

        //Send To Process
        int i=0;
        size=1; //By default, allocating one directory per process
        string buf; //Buffer to send the folders to the subordinate processes
        if(que.size()>(no_of_process-1)){
          size=ceil((float)que.size()/(no_of_process-1));
        }

        //cout<<"\n PARENT: Que size ="<<que.size();
        //cout<<"\n\n  PARENT : No of processes total ="<<no_of_process;
        //cout<<"\n PARENT : Size determined by parent = "<<size;

        /************* PARENT SENDER PROCESS ***********************/
        /************* ===================== ***********************/
        while(!que.empty() && i<=no_of_process-1){
            int j=0;
            buf="";
            while(j<size && !que.empty()){
              buf+=que.front();
              que.pop();
              buf+=";";
              j++;
            }

            //cout<<"\n\n RAW DATA SENT BY PARENT to Child"<<i+1<<": "<<buf;

         // MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
            MPI::COMM_WORLD.Send(buf.c_str(), buf.length(), MPI::CHAR, i+1, i+1);
            i++;
        }



        //cout<<"\n\n PARENT: THE Value of i = "<<i;

        /************* PARENT RECEIVER PROCESS ***********************/
        /************* ======================= ***********************/
        while(i>0){
          cout<<"\n\n Process 0 Waiting to receive from child";
	        MPI::Status status;



	        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
          // MPI::COMM_WORLD.Probe(1, 1, status);

          //cout<<"\n Parent : Child sender process values : \n";


	        int l = status.Get_count(MPI::CHAR);
	        char *buf = new char[l];
	        const auto sender = status.Get_source();
	        const auto tag = status.Get_tag();

          //cout<<"\n Sender : "<<sender;
          //cout<<"\n Tag : "<<tag;

	        // MPI::COMM_WORLD.Recv(buf, l, MPI::CHAR, sender, tag, status);
        //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
          MPI::COMM_WORLD.Recv(buf, l, MPI::CHAR, sender, tag, status);

	        string fname(buf, l);
	        delete [] buf;
	        vector<string> fnames;
	        boost::split(fnames, fname, boost::is_any_of(";"));

          //cout<<"\n\n PARENT: VALUES RECEIVED FROM CHILD"<<sender<<" THAT ARE PUSHED INTO QUEUE :----------------------\n";
          //cout<<"\n RECEIVED VECTOR SIZE : "<<fnames.size();

	        for(int k=0;k<fnames.size();k++){
	          //cout<<"\n"<<fnames[k];
            if(fnames[k].length())
            que.push(fnames[k]);
	        }
	        i--;
	    }
        //recieve from process end and add to queue end

        // name_of_files.push_back(dir->path().filename().string());
        // no_of_files++;

        //cout<<"\n ################### PARENT : after pushing received values, queue size =  "<<que.size();

        //cout<<"\n\n################### PARENT : after pushing received values,Temporary function to print the values"<<endl;
        //Temp function to print the value of the queue

        /*
        que3=que;
        while(!que3.empty()){
          cout<<que3.front()<<endl;
          que3.pop();
        }

        */

        // cout<<"\n ################### PARENT : value of i after pushing received values = "<<i;

      }


      /************* IF QUEUE EMPTY, SEND QUIT MESSAGE TO SLAVES ***********************/
      /************* =========================================== ***********************/
      if(que.empty()){
        //MPI::COMM_WORLD.Abort(2);
        // MPI_Finalize();
        // exit(1);
        for(int rank_values=1;rank_values<no_of_process;rank_values++){
          string exit_message = "EXIT NOW";
          //MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
          MPI::COMM_WORLD.Send(exit_message.c_str(), exit_message.length(), MPI::CHAR, rank_values, rank_values);
        }
        cout<<"\n\n\n\n\n $$$$$$$$$$$$$$$$$$$$$$$$$$ PARENT: QUEUE EMPTY. BUBYE!\n ";
        MPI_Finalize();
        return 0;
      }


    } //END OF PROCESS 0


// FOR SUBORDINATE PROCESSES
  else{


  //File Keyword Mapper Table
  unordered_map<string,vector<pair<string, int> > >  file_keyword_table;

  while(1){
        cout<<"\n\n Child"<<pid<<": Waiting for something to happen";
  			queue<string> que2;
  			string res="";
          //recieve from parent
  			MPI::Status status2;
        //MPI::COMM_WORLD.Probe(from,tag,status);
	        MPI::COMM_WORLD.Probe(0, pid, status2);
	        int l2 = status2.Get_count(MPI::CHAR);
	        char *buf2 = new char[l2];
	        const auto sender = status2.Get_source(); // Useless, since all the info is gonna be sent only the source ?
	        const auto tag = status2.Get_tag();
	        MPI::COMM_WORLD.Recv(buf2, l2, MPI::CHAR, sender, tag, status2);
          cout<<"\n Child"<<pid<<": Received stuff from parent";
	        string fname2(buf2, l2);
	        delete [] buf2;
	        vector<string> fnames2;

          //Exit if parent sends EXIT NOW Message
          if(fname2.compare("EXIT NOW")==0){
            cout<<"\n\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ CHILD"<<pid<<" QUITTING NOW.\n";
            MPI_Finalize();
            return 0;
            // exit(1);
          }

          //cout<<"\n Child : ACTUAL RAW DATA THAT I GOT FROM PARENT : "<<fname2;

	        boost::split(fnames2, fname2, boost::is_any_of(";"));

          // cout<<"\n Child: Queue status before pushing values";
          // while(!que2.empty()){
          //   cout<<que2.front();
          //   que2.pop();
          // }

          //cout<<"\nChild: Got these values from parent";

          //cout<<"\nChild: NUMBER OF VALUES I GOT FROM PARENT : "<<fnames2.size();

	        for(int i=0;i<fnames2.size();i++){
            //cout<<"\n"<<fnames2[i];
	          if(fnames2[i].length())
            que2.push(fnames2[i]);
	        }
  		  //recieve from parent end

          //cout<<"\n\n Child:  After pushing, the queue size is : "<<que2.size();


          //TEMPORARY FUNCTION TO PRINT QUEUE VALUES
          queue<string> que3;
          que3=que2;
          cout<<"\n\n CHILD"<<pid<<" Queue : "<<endl;
          //Temp function to print the value of the queue
          while(!que3.empty()){
            cout<<que3.front()<<endl;
            que3.pop();
          }



          while(!que2.empty()){

            //cout<<"\n Child : checking for "<<que2.front();

		        string dir2 = que2.front();
		        que2.pop();
		        DIR *dp;
		        struct dirent *dirp;


            // cout<<"\n\n hey there!";
            // exit(1);

            path file_path(dir2);

            //If the queue element is a file
            if(!(is_directory(file_path))) {
              // cout<<"\n dei mandaya!"<<dir2<<" is a file";
              file_keyword_table.insert(make_pair<string,vector<pair<string, int> > >(string(dir2),getKeywords(dir2)));
              cout<< "\n Child" << pid << " has entries for :\n";
              for( auto& x: file_keyword_table){
                cout<<x.first<<"\n";
              }
            }

            //The element in the queue is a directory
            else {

            //Traversing the directory in the queue

		        if((dp = opendir(dir2.c_str())) == NULL) {
  		        cout << "Error(" << errno << ") opening " << dir2 << endl;
  		        return errno;
		        }

		        while ((dirp = readdir(dp)) != NULL) {
		          if(((string)dirp->d_name).compare(".")==0||((string)dirp->d_name).compare("..")==0){
		            continue;
		          }

		          //cout<<dirp->d_name<<endl;

              //BoostFileSystem Declaration
              //path file_path2(dir2+"/"+string(dirp->d_name));
              //if (is_directory(file_path2)) {
                //cout<<"\n\nChild"<<pid<<": The file path to be pushed = "<<dir2<<"/"<<dirp->d_name;
                res+=dir2+"/"+string(dirp->d_name)+";"; //Push elements into the queue
              //}

		        }
		        closedir(dp);
            //cout<<"\n Child:  I am reaching until this place \n";
            }
		    }

        //cout<<"\n ***************** CHILD : Value TO BE SENT  : "<<res;

      //cout<<"\n Child : I am also reaching here!\n";
      //cout<<"\n Child: About to send to parent process of rank 0";
		  //Sending it back to parent process - rank 0
			MPI::COMM_WORLD.Send(res.c_str(), res.length(), MPI::CHAR, 0, pid);

      //cout<<"\n\n YAY! : Value sent from Child!";

			//send to parent end
	}
}



  //Terminating MPI Environment
  MPI::Finalize ( );

  return 0;
}
