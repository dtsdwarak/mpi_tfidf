#include "mpi.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	MPI::Init(argc, argv);
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	try {
		int rank = MPI::COMM_WORLD.Get_rank();
		cout << "I am " << rank << std::endl;
	}
	catch (MPI::Exception e) {
		
		cout << "MPI ERROR: " << e.Get_error_code()<< " - " << e.Get_error_string()<< std::endl;
	
	}
	
	MPI::Finalize();
	return 0;
}