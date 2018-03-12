#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

using namespace std;

#define coger_tenedor 0
#define soltar_tenedor 1
#define sentado 2
#define levantado 3
#define camarero 10

void Camarero( int id, int nprocesos);
void Filosofo( int id, int nprocesos);
void Tenedor ( int id, int nprocesos);

// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
  int rank, size;

  srand(time(0));
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if( size!=11){
    if( rank == 0) 
    cout<<"El numero de procesos debe ser 11" << endl << flush ;
    MPI_Finalize( ); 
    return 0; 
  }

  if(rank == camarero)
    Camarero(rank, size);
  else{
    if ((rank%2) == 0)  
      Filosofo(rank,size); // Los pares son Filosofos 
    else 
      Tenedor(rank,size);  // Los impares son Tenedores
  }

  MPI_Finalize( );
  return 0;
}  
// ---------------------------------------------------------------------

void Camarero(int id, int nprocesos){
  int fi_comiendo = 0, filosofo;
  MPI_Status status;

  while(true){

    if (fi_comiendo < 4)
      MPI_Probe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status ); // Para leer el status del siguiente mensaje
    else
      MPI_Probe( MPI_ANY_SOURCE, levantado, MPI_COMM_WORLD, &status );  // Solo deja pasar a los mensajes que indiquen levantarse


    if (status.MPI_TAG == sentado){
      filosofo = status.MPI_SOURCE;
      MPI_Recv( NULL, 0, MPI_INT, filosofo, sentado, MPI_COMM_WORLD, &status);

      fi_comiendo++;

      cout << "CAMARERO: Filosofo " << filosofo << " puede sentarse." << endl << flush;
      MPI_Send(NULL, 0, MPI_INT, filosofo, sentado, MPI_COMM_WORLD);
    }

    if (status.MPI_TAG == levantado){
      filosofo = status.MPI_SOURCE;
      MPI_Recv( NULL, 0, MPI_INT, filosofo, levantado, MPI_COMM_WORLD, &status);
      
      fi_comiendo--;

      cout << "CAMARERO: Filosofo " << filosofo << " se ha levantado." << endl << flush;
    }

  }
}

void Filosofo( int id, int nprocesos )
{
    int izq = (id+1) % (nprocesos-1);
    int der = (id+nprocesos-2) % (nprocesos-1);
    MPI_Status status;
   
   while(1)
   {
      // Pide al camarero que lo siente (si hay sitio)
      cout << "Filosofo " << id << " pide sitio al camarero." << endl;
      MPI_Send(NULL, 0, MPI_INT, camarero, sentado, MPI_COMM_WORLD);

      // El camarero le responde cuando ya hay sitio
      MPI_Recv(NULL, 0, MPI_INT, camarero, sentado, MPI_COMM_WORLD, &status);

      // Solicita tenedor izquierdo
      cout << "Filosofo "<<id<< " solicita tenedor izq ..." << izq << endl;
      MPI_Ssend(NULL, 0, MPI_INT, izq, coger_tenedor, MPI_COMM_WORLD);

      // Solicita tenedor derecho
      cout <<"Filosofo "<<id<< " coge tenedor der ..." << der << endl;
      MPI_Ssend(NULL, 0, MPI_INT, der, coger_tenedor, MPI_COMM_WORLD);

      cout<<"Filosofo "<<id<< " COMIENDO"<<endl<<flush;
      sleep((rand() % 3)+1);  //comiendo

      // Suelta el tenedor izquierdo
      cout <<"Filosofo "<<id<< " suelta tenedor izq ..." << izq << endl;
      MPI_Ssend(NULL, 0, MPI_INT, izq, soltar_tenedor, MPI_COMM_WORLD);

      // Suelta el tenedor derecho
      cout <<"Filosofo "<<id<< " suelta tenedor der ..." << der << endl;
      MPI_Ssend(NULL, 0, MPI_INT, der, soltar_tenedor, MPI_COMM_WORLD);

      // Avisa al camarero de que se levanta
      MPI_Ssend(NULL, 0, MPI_INT, camarero, levantado, MPI_COMM_WORLD );

      // Piensa (espera bloqueada aleatorio del proceso)
      cout << "Filosofo " << id << " PENSANDO" << endl;

        // espera bloqueado durante un intervalo de tiempo aleatorio 
        // (entre una décima de segundo y un segundo)
        usleep( 1000U * (100U+(rand()%900U)) );

      }
}
// ---------------------------------------------------------------------

void Tenedor(int id, int nprocesos)
{
  int buf; 
  MPI_Status status; 
  int Filo;
  
  while( true )
  {
    // Espera un peticion desde cualquier filosofo vecino ...
    MPI_Recv( NULL, 0, MPI_INT, MPI_ANY_SOURCE, coger_tenedor, MPI_COMM_WORLD, &status);

    // Recibe la peticion del filosofo ...
    Filo = status.MPI_SOURCE;    
    cout << "Ten. " << id << " recibe petic. de " << Filo << endl;
    
    // Espera a que el filosofo suelte el tenedor...
    MPI_Recv( NULL, 0, MPI_INT, Filo, soltar_tenedor, MPI_COMM_WORLD, &status );
    cout << "Ten. " << id << " recibe liberac. de " << Filo << endl; 
  }
}
// ---------------------------------------------------------------------
