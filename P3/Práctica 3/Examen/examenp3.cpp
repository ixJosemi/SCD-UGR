// APELLIDOS: López Sánchez
// NOMBRE: José Luis

#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"


#define clientes 9
#define proceso_intermedio  10

#define Pagar       0
#define Acabar      1

using namespace std;

// ---------------------------------------------------------------------

void Cola_intermedia()
{
	int		peticion,
			emisor,
			cajas_ocupadas = 0;
   MPI_Status status ;
   
   while(1)
   {  
		if(cajas_ocupadas < 3)
			MPI_Probe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		else
			MPI_Probe( MPI_ANY_SOURCE, Acabar, MPI_COMM_WORLD, &status );

		emisor = status.MPI_SOURCE;
		MPI_Recv( NULL, 0, MPI_INT, emisor, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

		if(status.MPI_TAG == Pagar)
		{
			cajas_ocupadas++;
			MPI_Ssend( NULL, 0, MPI_INT, emisor, Pagar, MPI_COMM_WORLD );
			cout << "Cliente " << emisor << " pagando." << endl << flush;  
		}
		else if(status.MPI_TAG == Acabar)
		{
			cajas_ocupadas--;
			cout << "Cliente " << emisor << " ha terminado de pagar. " << endl << flush;  
		}
   }
}
   
// ---------------------------------------------------------------------

void Cliente(int n_cliente)
{
   int         value,
               peticion = 1 ; 
   MPI_Status  status ;
 
   while(1)
   {
		// Cliente comprando
		usleep( 1000U * (100U+(rand()%900U)) );
		
		// Solicitar pago
      	MPI_Ssend( NULL, 0, MPI_INT, proceso_intermedio, Pagar, MPI_COMM_WORLD );

		// Puede entrar en caja
		MPI_Recv ( NULL, 0,    MPI_INT, proceso_intermedio, Pagar, MPI_COMM_WORLD, &status );
      	cout << "Cliente " << n_cliente << " pagando. " << endl << flush ;
		usleep( 1000U * (100U+(rand()%900U)) );
      
		// Compra completada
		MPI_Ssend( NULL, 0, MPI_INT, proceso_intermedio, Acabar, MPI_COMM_WORLD );
		usleep( 1000U * (100U+(rand()%900U)) );
   }
}
// ---------------------------------------------------------------------

int main(int argc, char *argv[]) 
{
   int rank,size; 
   
   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &size );
   
   // inicializa la semilla aleatoria:
   srand ( time(NULL) );
   
   // comprobar el número de procesos con el que el programa 
   // ha sido puesto en marcha (debe ser 3)
   if ( size != 11 ) 
   {
      cout<< "El numero de procesos debe ser 11 "<<endl;
      return 0;
   } 
   
   // verificar el identificador de proceso (rank), y ejecutar la
   // operación apropiada a dicho identificador
   if ( rank <= clientes ) 				// Si menor o igual 9
		Cliente(rank);
   else if ( rank == proceso_intermedio ) 	// Si igual que 10
		Cola_intermedia();

   
   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
