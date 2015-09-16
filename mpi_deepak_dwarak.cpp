// mpic++ -std=c++11 mpi_deepak_dwarak.cpp -o mpi_deepak_dwarak -lboost_filesystem -lboost_system -lboost_regex
// mpirun -np 6 ./mpi_deepak_dwarak 10 new-delhi

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
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <set>

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

  //argv[1] - PERCENT OF KEYWORDS REQUIRED ; argv[2] - FOLDER PATH
  if(!argv[1] || !argv[2]){
    cout<<"\n\n Parameter not provided. Quitting\n";
    MPI_Finalize();
    return 0;
  }

  //Get the process ID
  pid = MPI::COMM_WORLD.Get_rank();

  //Taking the Time value
  clock_t time_value = clock();

  // Process ID 0 => Initial Process
  if(pid==0){
  	  queue<string> que;
      que.push(string(argv[2],strlen(argv[2])));

      /********* INITIAL STRUCTURE TO HAVE SOME VALUES IN THE QUEUE ***************/

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

        que.push(dir+"/"+string(dirp->d_name)); //If only this statement is present, we push all the files into the queue
      }
      closedir(dp);

      /********* INITIAL STRUCTURE TO HAVE SOME VALUES IN THE QUEUE ***************/


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

        //Allocate work to processes equally.
        int i=0;
        size=1; //By default, allocating one directory per process
        string buf; //Buffer to send the folders to the subordinate processes
        if(que.size()>(no_of_process-1)){
          size=ceil((float)que.size()/(no_of_process-1));
        }

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

         // MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
            MPI::COMM_WORLD.Send(buf.c_str(), buf.length(), MPI::CHAR, i+1, i+1);
            i++;
        }

        /************* PARENT RECEIVER PROCESS ***********************/
        /************* ======================= ***********************/
        while(i>0){
          // cout<<"\n\n Process 0 Waiting to receive from child";
	        MPI::Status status;

          //Probe for values first
	        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);

	        int l = status.Get_count(MPI::CHAR);
	        char *buf = new char[l];
	        const auto sender = status.Get_source();
	        const auto tag = status.Get_tag();

        //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
          MPI::COMM_WORLD.Recv(buf, l, MPI::CHAR, sender, tag, status);

	        string fname(buf, l);
	        delete [] buf;
	        vector<string> fnames;
	        boost::split(fnames, fname, boost::is_any_of(";"));

	        for(int k=0;k<fnames.size();k++){
            if(fnames[k].length())
            que.push(fnames[k]);
	        }
	        i--;
	       }
       }


       vector<int> processes_with_files; //Vector to store only the files with ranks
       set<string> queue_values;
       vector<string> vec_queue_values;

      /************* IF QUEUE EMPTY, PROCEED TO QUERY PROCESSING ***********************/
      /************* =========================================== ***********************/
      if(que.empty()){


        //Message asking children to send their file availability
        string send_rank_message="SEND IF YOU HAVE";

        //Send message to children to send if they have files with them
        for(int rank_values=1;rank_values<no_of_process;rank_values++){
          //MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
          MPI::COMM_WORLD.Send(send_rank_message.c_str(), send_rank_message.length(), MPI::CHAR, rank_values, rank_values);
        }

        //Values for reception
        int rank_received[no_of_process];
        rank_received[0]=0; //Parent process - So excluding it.

        for(int rank_values=1;rank_values<no_of_process;rank_values++){
          //For probe status store
          MPI::Status status;
          //Probe for incoming values
          MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
          //Get source and tag
          const auto sender = status.Get_source();
          const auto tag = status.Get_tag();
          //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
          MPI::COMM_WORLD.Recv(&rank_received[sender], 1, MPI::INT, sender, tag, status);
        }

        //String for rank values to be sent to all the child processes
        string processes_with_files_str="";

        //Storing the rank of processes that have files
        for(int i=1;i<no_of_process;i++){
          if(rank_received[i]==1){
            processes_with_files_str+= to_string(i) + ";";
            processes_with_files.push_back(i);
          }
        }

        string process_list_message = "ABOUT TO SEND PROCESS VALUES";
        for(int i=0;i<processes_with_files.size();i++){
          //MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
          MPI::COMM_WORLD.Send(process_list_message.c_str(), process_list_message.length(), MPI::CHAR, processes_with_files[i], processes_with_files[i]);
          MPI::COMM_WORLD.Send(processes_with_files_str.c_str(), processes_with_files_str.length(), MPI::CHAR, processes_with_files[i], processes_with_files[i]);
          // cout<<"\n\n Parent has sent the value!\n";
        }


      }//End of queue empty condition

      int val_recv;

      //Expecting reply from all child processes
      for(int i=0;i<processes_with_files.size();i++){
        MPI::Status status;
        //Probe for incoming values
        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
        //Get source and tag
        auto sender = status.Get_source();
        auto tag = status.Get_tag();
        //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
        MPI::COMM_WORLD.Recv(&val_recv, 1, MPI::INT, sender, tag, status);
      }

      cout<<"\n\n Time taken to process data = "<<float(clock()-time_value)/CLOCKS_PER_SEC<<" seconds";

      while(1){

        int choice;
        string task_message;
        char whatfile[400];

        cout<<"\n\n Graph Processed. What do you want to do now? \n 1. Find all the files related to another file\n 2. Find the Transitive Closure of a file\n 3. Exit\n 4. Choice : ";
        cin>>choice;
        cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');

        switch(choice) {
          case 1: cout<<"\n Enter the file name : ";
                  cin.getline(whatfile,400);
                  task_message=string(whatfile,strlen(whatfile))+";Related Files";
                  // cout<<"\n"<<task_message;
                  time_value=clock();
                  break;
          case 2: cout<<"\n Enter the file name you wish to find the transitive closure for : ";
                  cin.getline(whatfile,400);
                  task_message=string(whatfile,strlen(whatfile))+";Transitive Closure;Just Tell";
                  queue_values.insert(whatfile);
                  vec_queue_values.push_back(whatfile);
                  time_value=clock();
                  break;
          case 3: task_message="EXIT NOW";

          default:;
        }

        for(int rank_values=1;rank_values<no_of_process;rank_values++){
          //MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
          MPI::COMM_WORLD.Send(task_message.c_str(), task_message.length(), MPI::CHAR, rank_values, rank_values);
        }

        if(choice==3){
          cout<<"\n PARENT : QUITTING. BYE!";
          break;
        }

        else if (choice==2){

          int send_flag=1;


          while(send_flag) {


          send_flag=0;

          char* char_value=NULL;
          int char_length;

          // cout<<"\n\n ********************************** Value Sent for Transitive closure!";

          for(int i=0;i<vec_queue_values.size();i++){


            MPI::Status status;
            //Probe for incoming values
            MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
            //Get source and tag
            char_length = status.Get_count(MPI::CHAR);
            char_value = new char[char_length];
            auto sender = status.Get_source();
            auto tag = status.Get_tag();
            //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
            // cout<<"\n\n Parent waiting to receive!\n\n ";
            MPI::COMM_WORLD.Recv(char_value, char_length, MPI::CHAR, sender, tag, status);

          }


          string recd_string(char_value,char_length);
          delete [] char_value;

          vector<string> recd_file_vector;
          string send_string_val="";

          //Clear the vector queue value
          vec_queue_values.clear();

          boost::split(recd_file_vector, recd_string, boost::is_any_of(";"));


          for(int i=0;i<recd_file_vector.size();i++){

            if(recd_file_vector[i].length()){

              if(queue_values.find(recd_file_vector[i])==queue_values.end()){
                send_flag=1;
                queue_values.insert(recd_file_vector[i]);
                vec_queue_values.push_back(recd_file_vector[i]);
                send_string_val += recd_file_vector[i] + ";";

              }
            }
          }

          send_string_val += "Transitive Closure;Find One";

          if(send_flag){

            for(int rank_values=1;rank_values<no_of_process;rank_values++){
              // cout<<"\n\n Sending value to "<<rank_values;
               MPI::COMM_WORLD.Send(send_string_val.c_str(), send_string_val.length(), MPI::CHAR, rank_values, rank_values);
            }
          }

          else{
            cout<<"\n\n Time taken to find transitive closure = "<<float(clock()-time_value)/CLOCKS_PER_SEC<<" seconds";
            cout<<"\n\n Connected File Names : \n";
            queue_values.erase(whatfile);
            for(auto x: queue_values){
              cout<<x<<"\n";
            }
            queue_values.clear();
            vec_queue_values.clear();
          }
        }

        }

        else{

          MPI::Status status;
          //Probe for incoming values
          MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
          //Get source and tag
          auto sender = status.Get_source();
          auto tag = status.Get_tag();
          //MPI::Comm::Recv(void* buf, int count, MPI::Datatype& datatype, int source, int tag, MPI::Status* status)
          // cout<<"\n\n Parent waiting to receive!\n\n ";
          MPI::COMM_WORLD.Recv(&val_recv, 1, MPI::INT, sender, tag, status);
          cout<<"\n\n Time taken to find related files = "<<float(clock()-time_value)/CLOCKS_PER_SEC<<" seconds";        }

      } // End of While Loop


    } //END OF PROCESS 0


// FOR SUBORDINATE PROCESSES
  else{


  //File Keyword Mapper Tables
  unordered_map<string,set<string > > file_keyword_table;
  unordered_map<string,set<string > > recd_file_keyword_table;
  map <pair<string,string>, int> similarity_matrix;

  while(1){
        cout<<"\n\n Child"<<pid<<": Waiting for something to happen";
  			queue<string> que2;
  			string res="";
          //recieve from parent
  			MPI::Status status2;

        // Probe and get the values to be received
        MPI::COMM_WORLD.Probe(0, pid, status2);
        int l2 = status2.Get_count(MPI::CHAR);
        char *buf2 = new char[l2];
        auto sender = status2.Get_source(); // Useless, since all the info is gonna be sent only the source ?
        auto tag = status2.Get_tag();


        MPI::COMM_WORLD.Recv(buf2, l2, MPI::CHAR, sender, tag, status2);

        // cout<<"\n Child"<<pid<<": Received stuff from parent";
        string fname2(buf2, l2);
        delete [] buf2;
        vector<string> fnames2;

          //Block to exit if parent sends EXIT NOW Message
          if(fname2.compare("EXIT NOW")==0){
            cout<<"\n\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ CHILD"<<pid<<" QUITTING NOW.\n";
            MPI_Finalize();
            return 0;
            // exit(1);
          }

          //If Transitive Closure if being asked for
          if(fname2.find("Transitive Closure")!=string::npos){


            if(fname2.find("Transitive Closure;Just Tell")!=string::npos){

              string act_file_name = fname2.substr(0, fname2.find(';'));

              string send_file_string = "";

              if(file_keyword_table.find(act_file_name)!=file_keyword_table.end()){

                for(auto p: similarity_matrix ){
                  if(p.first.first.compare(act_file_name)==0 && p.second!=0 ){
                    send_file_string += p.first.second + ";";
                  }
                }

                MPI::COMM_WORLD.Send(send_file_string.c_str(), send_file_string.length(), MPI::CHAR, 0, pid);

              }

            }

            else if(fname2.find("Transitive Closure;Find One")!=string::npos){

              vector<string> act_file_names;
              string send_file_string;
              boost::split(act_file_names, fname2, boost::is_any_of(";"));

              for(int i=0;i<act_file_names.size();i++){

                send_file_string="";

                if(act_file_names[i].length() && file_keyword_table.find(act_file_names[i])!=file_keyword_table.end()){

                  for(auto p: similarity_matrix ){
                    if(p.first.first.compare(act_file_names[i])==0 && p.second!=0 ){
                      send_file_string += p.first.second + ";";
                    }
                  }

                  MPI::COMM_WORLD.Send(send_file_string.c_str(), send_file_string.length(), MPI::CHAR, 0, pid);

                }

              }

            }

            continue;

          }

          //If Related Files is asked for
          if(fname2.find("Related Files")!=string::npos){
            string act_file_name = fname2.substr(0, fname2.find(';'));
            if(file_keyword_table.find(act_file_name)!=file_keyword_table.end()){
              cout<<"\n\n Related Files retrieved by "<<pid<<"\n";
              for(auto it : similarity_matrix){
                if(it.first.first.compare(act_file_name)==0 && it.second!=0)
                  cout<<it.first.second<<"\n";
              }
              // cout<<"\n\n Child"<<pid<<" is going to send!";
              MPI::COMM_WORLD.Send(&pid, 1, MPI::INT, 0, pid);
              // cout<<"\n Child"<<pid<<" has sent the value!";
            }
            continue;
          }

          // Block to notify parent if you have a file
          if(fname2.compare("SEND IF YOU HAVE")==0){
            //Sending file have value to Sender
            int have_value=0;
            if(!file_keyword_table.empty())
              have_value=1;
          //MPI::Comm::Send(const void* buf, int count, MPI::Datatype& datatype, int dest, int tag)
            MPI::COMM_WORLD.Send(&have_value, 1, MPI::INT, 0, pid);
            continue; // To go to Probe point
          }


          // Block for Inter Process Communication
          if((fname2.compare("ABOUT TO SEND PROCESS VALUES")==0)){


            MPI::COMM_WORLD.Probe(0, pid, status2);

            l2 = status2.Get_count(MPI::CHAR);
            char* buf3 = new char[l2];
            sender = status2.Get_source(); // Useless, since all the info is gonna be sent only the source ?
            tag = status2.Get_tag();

            MPI::COMM_WORLD.Recv(buf3, l2, MPI::CHAR, sender, tag, status2);

            fname2 = string(buf3,l2);
            delete[] buf3;

            //Clear fnames2 if it may have content already.
            fnames2.clear();

            //Splitting the values based ; and storing them as separate values
            boost::split(fnames2, fname2, boost::is_any_of(";"));

            //Vector of Process Values that have files
            vector<int> process_values;

            for(int i=0;i<fnames2.size();i++){
              if(fnames2[i].length()){
                process_values.push_back(stoi(fnames2[i]));
              }
            }


            string temp_value="";
            int myval;

            //Send values to child processes
            for(int i=0;i<process_values.size();i++){

              //If I am the one who's supposed to send
              if(pid==process_values[i]){

                //First getting the count of number of files that I have to send to all processes
                int file_table_size = file_keyword_table.size();

                //Send the file table size to all processes
                for(int j=0;j<process_values.size();j++){
                  if(j!=i){
                    MPI::COMM_WORLD.Send(&file_table_size,1,MPI::INT,process_values[j],process_values[j]);
                  }
                }

                //For each file in the unordered map, generate a string
                for( auto& x: file_keyword_table){
                      temp_value="";
                      // cout<<x.first<<"\n";
                      temp_value += x.first + "#";
                      for(set<string>::iterator it=x.second.begin();it!=x.second.end();it++){
                        temp_value += *it + ";";
                      }

                      //Send the string to child processes that are supposed to receive
                      for(int j=0;j<process_values.size();j++){
                        if(j!=i){
                          //Send the string next
                          MPI::COMM_WORLD.Send(temp_value.c_str(), temp_value.length(), MPI::CHAR, process_values[j], process_values[j]);
                        }
                      }
                }

              }

              //If I am supposed to receive from someone else
              else{

                int file_table_size;
                MPI::COMM_WORLD.Recv(&file_table_size,1,MPI::INT,process_values[i],pid);
                // cout<<"\n\n Child"<<pid<< ": Received values from "<<process_values[i]<<" and it is = "<<file_table_size;

                for(int j=0;j<file_table_size;j++){

                  MPI::COMM_WORLD.Probe(process_values[i], pid, status2);
                  l2 = status2.Get_count(MPI::CHAR);
                  char* buf4 = new char[l2];
                  // sender = status2.Get_source(); // Useless, since all the info is gonna be sent only the source ?
                  // tag = status2.Get_tag();


                  MPI::COMM_WORLD.Recv(buf4, l2, MPI::CHAR, process_values[i], pid);

                  string recd_string(buf4,l2);
                  delete [] buf4;

                  vector<string> recd_file_vector;
                  boost::split(recd_file_vector, recd_string, boost::is_any_of("#;"));

                  string file_name = recd_file_vector[0]; //File name
                  recd_file_vector.erase(recd_file_vector.begin()); //Removing the filename

                  set<string> recd_file_vector_strings(recd_file_vector.begin(),recd_file_vector.end());
                  recd_file_vector_strings.erase(recd_file_vector_strings.begin()); //Removing the empty space

                  recd_file_keyword_table.insert(make_pair<string,set<string > >(string(file_name),set<string>(recd_file_vector_strings)));
                  // cout<<"\n\n Child"<<pid<<" : Inserting filename = "<<file_name<<" received from Child"<<process_values[i];


                }

              } //End of else process

            }//End of IPC For Loop

            for(auto p: file_keyword_table){
              for(auto q: file_keyword_table ){
                if(p.first.compare(q.first)==0){
                  // cout<<"\nYes!";
                  similarity_matrix.insert(   make_pair<pair<string,string>,int>(   make_pair<string,string>(  string(p.first),string(q.first)  ),0   )   );
                }
                else if (   similarity_matrix.find(    make_pair<string,string>(string(p.first),string(q.first))    )   ==   similarity_matrix.end()       ) {
                  int similar=get_similarity(file_keyword_table.at(p.first),file_keyword_table.at(q.first));
                  similarity_matrix.insert(   make_pair<pair<string,string>,int>    (    make_pair<string,string>    (   string(p.first),string(q.first)   )    ,   int(similar)   )    );
                  similarity_matrix.insert(   make_pair<pair<string,string>,int>    (    make_pair<string,string>    (   string(q.first),string(p.first)   )    ,   int(similar)   )    );

              }
            }
          }


          for(auto p: file_keyword_table){
            for(auto q: recd_file_keyword_table){
              similarity_matrix.insert(   make_pair<pair<string,string>,int>    (    make_pair<string,string>    (   string(p.first),string(q.first)   )    ,   get_similarity(file_keyword_table.at(p.first),recd_file_keyword_table.at(q.first))   )    );
            }
          }

            //Send that process has processed the graph
            MPI::COMM_WORLD.Send(&pid, 1, MPI::INT, 0, pid);

            // To continue to Probe point once block gets executed
            continue;
          }


          //Split folder functions into separate names
	        boost::split(fnames2, fname2, boost::is_any_of(";"));

	        for(int i=0;i<fnames2.size();i++){
	          if(fnames2[i].length())
            que2.push(fnames2[i]);
	        }

          while(!que2.empty()){

		        string dir2 = que2.front();
		        que2.pop();
		        DIR *dp;
		        struct dirent *dirp;

            //Boost file path
            path file_path(dir2);

            //If the queue element is a file
            if(!(is_directory(file_path))) {
              file_keyword_table.insert(make_pair<string,set<string > >(string(dir2),getKeywords(dir2,atoi(argv[1]))));
              cout<< "\n Child" << pid << " has entries for :\n";
              for( auto& x: file_keyword_table){
                cout<<x.first<<"\n";
              }
            }

            //The element in the queue is a directory
            else {
              //Traversing the directory in the queue and pushing them onto buffer to send to master
  		        if((dp = opendir(dir2.c_str())) == NULL) {
    		        cout << "Error(" << errno << ") opening " << dir2 << endl;
    		        return errno;
  		        }

  		        while ((dirp = readdir(dp)) != NULL) {
  		          if(((string)dirp->d_name).compare(".")==0||((string)dirp->d_name).compare("..")==0){
  		            continue;
  	             }
                //Push elements into the queue
                res+=dir2+"/"+string(dirp->d_name)+";";

	            }
		          closedir(dp);
            }
		    }

        //Send buffer to child process
			  MPI::COMM_WORLD.Send(res.c_str(), res.length(), MPI::CHAR, 0, pid);


	} //End of while loop

} //End of child process

  //Terminating MPI Environment
  MPI::Finalize ( );
  return 0;
}
