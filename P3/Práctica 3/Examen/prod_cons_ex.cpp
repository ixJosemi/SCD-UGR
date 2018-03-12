// APELLIDOS: López Sánchez
// NOMBRE: José Luis

#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"


#define Productor    3
#define Buffer       8
#define Consumidor   0
#define ConsumidorPares 1

#define ITERS       10
#define TAM          5

using namespace std;

// ---------------------------------------------------------------------

void productor(int n_productor)
{
   	int value ;
	MPI_Status status ;
   
	for ( unsigned int i=0; i < ITERS ; i++ )
	{ 
		value = rand()%10 ;
		cout << "Productor " << n_productor << " produce valor " << value << endl << flush ;

		// espera bloqueado durante un intervalo de tiempo aleatorio 
		// (entre una décima de segundo y un segundo)
		usleep( 1000U * (100U+(rand()%900U)) );

		// enviar 'value'
		MPI_Ssend( &value, 1, MPI_INT, Buffer, 0, MPI_COMM_WORLD );
	
		// Esperar hasta que el buffer le diga que se ha consumido el dato
		MPI_Recv ( NULL, 0, MPI_INT, Buffer, 0, MPI_COMM_WORLD, &status );
	}
}
// ---------------------------------------------------------------------

void buffer()
{
	int		value ,
			peticion ,
			siguiente_cons ,
			fuente_prod;
   MPI_Status status ;
   
   for( unsigned int i=0 ; i < ITERS*3 ; i++ )
   {  

	MPI_Recv( &value, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );
	fuente_prod = status.MPI_SOURCE;

	if(value%2 == 0)
	{
		MPI_Recv( &peticion, 1, MPI_INT, ConsumidorPares, 1, MPI_COMM_WORLD, &status );
        MPI_Ssend( &value, 1, MPI_INT, ConsumidorPares, 1, MPI_COMM_WORLD);
		cout << "Buffer envía " << value << " a Consumidor 1 (PARES)." << endl << flush;  
	}
	else
	{
 		MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status );
        MPI_Ssend( &value, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
		cout << "Buffer envía " << value << " a Consumidor " << status.MPI_SOURCE << " (IMPARES)." << endl << flush;  
	}

	// Avisamos al productor de que puede volver a producir
	MPI_Ssend( NULL, 0, MPI_INT, fuente_prod, 0, MPI_COMM_WORLD);

   }
}


   
// ---------------------------------------------------------------------

void consumidor(int n_consumidor)
{
   int         value,
               peticion = 1 ; 
   MPI_Status  status ;
 
   while(1)
   {
      MPI_Ssend( &peticion, 1, MPI_INT, Buffer, 1, MPI_COMM_WORLD ); 
      MPI_Recv ( &value, 1,    MPI_INT, Buffer, 1, MPI_COMM_WORLD, &status );
      cout << "Consumidor " << n_consumidor << " recibe valor " << value << " de Buffer " << endl << flush ;
      
      // espera bloqueado durante un intervalo de tiempo aleatorio 
      // (entre una décima de segundo y un segundo)
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
   if ( size != 9 ) 
   {
      cout<< "El numero de procesos debe ser 9 "<<endl;
      return 0;
   } 
   
   // verificar el identificador de proceso (rank), y ejecutar la
   // operación apropiada a dicho identificador
   if ( rank < Productor ) 		// Si menor que 3
      consumidor(rank);
   else 
		if ( rank == Buffer ) 	// Si igual que 8
			  buffer();
		else 
		  productor(rank);		// Si entre 3 y 7
   
   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
