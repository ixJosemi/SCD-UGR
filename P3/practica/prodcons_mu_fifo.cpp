#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

// Atributos
const int etiq_productor  = 1, // Ojo, no es el numero del proceso, sino la etiqueta
          etiq_consumidor = 2, // Idem que la anterior
          etiq_buffer     = 5, // Numero del proceso
          ITERS           = 20,
          TAM             = 10,
          productores     = 5,
          consumidores    = 4;

int num_procesos_esperado = 10;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
// ---------------------------------------------------------------------
// ptoducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int num_productor)
{
   static int contador = num_productor ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(int rank)
{
   for ( unsigned int i= 0 ; i < 4 ; i++ )
   {
      // producir valor
      int valor_prod = producir(rank);
      // enviar valor
      cout << "Productor " << rank << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, etiq_buffer, etiq_productor, MPI_COMM_WORLD);
   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
}
// ---------------------------------------------------------------------

void funcion_consumidor(int rank)
{
   int         peticion = 1, valor_rec;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < 5; i++ ) // 4 consumidores, cada uno consume 5 datos = 20 datos
   {
      // Pide los datos al buffer
      MPI_Ssend( &peticion,  1, MPI_INT, etiq_buffer, etiq_consumidor, MPI_COMM_WORLD);

      // Recibe los datos del buffer cuando la peticion ha sido aceptada
      MPI_Recv ( &valor_rec, 1, MPI_INT, etiq_buffer, 0, MPI_COMM_WORLD, &estado);
      cout << "Consumidor " << rank << " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        value[TAM] , 
              val,
              peticion , 
              primera_libre = 0,
              primera_ocupada = 0,
              num_celdas_ocupadas = 0,
              opcion ;
   MPI_Status status ;
   
   for( unsigned int i=0 ; i < ITERS*2 ; i++ )
   {  
      if ( num_celdas_ocupadas == 0 )      // el consumidor no puede consumir
         opcion = 0 ;        
      else if (num_celdas_ocupadas == TAM) // el productor no puede producir
         opcion = 1 ;           
      else               // ambas guardas son ciertas
         opcion = MPI_ANY_SOURCE;

      // leer 'status' del siguiente mensaje (esperando si no hay)
      MPI_Recv(&val, 1, MPI_INT, opcion, 0, MPI_COMM_WORLD, &status);

      // calcular la opcion en función del origen del mensaje
      if ( status.MPI_SOURCE == etiq_productor ) 
        opcion = 0 ; 
      else
        opcion = 1 ;
      
      switch(opcion)
      {
        case 0:
          value[primera_libre] = val;
          primera_libre = (primera_libre + 1) % TAM;
          num_celdas_ocupadas++;
          cout << "Buffer recibe " << value[primera_libre] << " de Productor " << status.MPI_SOURCE << endl << flush;  
          break;
        case 1:
          val = value[primera_ocupada];
          primera_ocupada = (primera_ocupada + 1) % TAM;
          num_celdas_ocupadas--;
          cout << "Buffer envía " << value[primera_ocupada] << " a Consumidor " << status.MPI_SOURCE << endl << flush;  
          MPI_Ssend( &val, 1, MPI_INT, etiq_consumidor, 0, MPI_COMM_WORLD);
          break;
      }     
   }
}   

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int rank, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( rank < etiq_buffer )
         funcion_productor(rank);
      else if ( rank == etiq_buffer )
         funcion_buffer();
      else
         funcion_consumidor(rank);
   }
   else
   {
      if ( rank == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
