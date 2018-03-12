#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

#define soltar     0
#define coger      1
#define levantarse 3
#define sentarse   2

const int num_filosofos = 5 ,
          num_procesos  = 2*num_filosofos + 1,
          camarero = 10;


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

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % (num_procesos-1), //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1) ; //id. tenedor der.
  MPI_Status estado;

  while ( true )
  {

    // Solicita sentarse
 		cout << "Filosofo " << id << " solicita al Camarero sentarse" << endl;
 		MPI_Ssend(NULL, 0, MPI_INT, camarero, sentarse, MPI_COMM_WORLD);

    // El filosofo espera a que le digan que puede sentarse
    MPI_Recv(NULL, 0, MPI_INT, camarero, sentarse, MPI_COMM_WORLD, &estado);
 		cout << "Filosofo " << id << " se SIENTA" << endl;

    // Solicita tenedor izquierdo
    cout << "Filosofo " << id << " solicita tenedor izq ..." << id_ten_izq << endl;
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, coger, MPI_COMM_WORLD);

    // Solicita tenedor derecho
    cout << "Filosofo " << id << " solicita tenedor der ..." << id_ten_der << endl;
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, coger, MPI_COMM_WORLD);

    cout << "Filósofo " <<id << " comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout << "Filósofo " << id << " suelta ten. izq. " << id_ten_izq << endl;
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, soltar, MPI_COMM_WORLD);

    cout << "Filósofo " << id << " suelta ten. der. " << id_ten_der << endl;
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, soltar, MPI_COMM_WORLD);

    // Le dice al Camarero que se levanta para que anote el sitio libre
    cout << "Filosofo " << id << " le dice al Camarero que se levanta" << endl;
    MPI_Ssend(NULL, 0, MPI_INT, camarero, levantarse, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
    // Esperamos una peticion de cualquier filosofo que este a los lados del tenedor

    // ...... recibir petición de cualquier filósofo (completar)
    // ...... guardar en 'id_filosofo' el id. del emisor (completar)
    MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, coger, MPI_COMM_WORLD, &estado);
    id_filosofo = estado.MPI_SOURCE;
    cout <<"Ten. " << id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

    // ...... recibir liberación de filósofo 'id_filosofo' (completar)
    MPI_Recv(&id_filosofo, 1, MPI_INT, id_filosofo, soltar, MPI_COMM_WORLD, &estado);
    cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void Camarero( )
{
  int id_filosofo, tag, num_filosofos_mesa = 0;
  MPI_Status estado;

  while(true)
  {
    if(num_filosofos_mesa < 4)
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
    else
      MPI_Probe(MPI_ANY_SOURCE, levantarse, MPI_COMM_WORLD, &estado);

    tag = estado.MPI_TAG;

    if(tag == sentarse){ // Se le deja sentarse
      id_filosofo=estado.MPI_SOURCE;
      MPI_Recv( NULL, 0, MPI_INT, id_filosofo, sentarse, MPI_COMM_WORLD,&estado);
      num_filosofos_mesa++;

      MPI_Send( NULL, 0, MPI_INT, id_filosofo, sentarse, MPI_COMM_WORLD);
      cout << "Filosofo " << id_filosofo << " se sienta. Hay " << num_filosofos_mesa 
           << " filosofos sentados. " << endl;
    }
    else if(tag == levantarse){ // Se levanta
      id_filosofo = estado.MPI_SOURCE;
      MPI_Recv(NULL, 0, MPI_INT, id_filosofo, levantarse, MPI_COMM_WORLD, &estado);
      num_filosofos_mesa--;
      cout << "Filosofo " << id_filosofo << " se levanta. Hay " << num_filosofos_mesa
           << " filosofos sentados.  " << endl;
    }
  }
}
// ---------------------------------------------------------------------

// Punto de entrada al programa
int main(int argc, char** argv)
{
  int id_propio, num_procesos_actual ;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
  MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


  if ( num_procesos == num_procesos_actual )
  {
    // ejecutar la función correspondiente a 'id_propio'
    if ( id_propio % 2 == 0 )          // si es par
      funcion_filosofos( id_propio ); //   es un filósofo
    else                               // si es impar
      funcion_tenedores( id_propio ); //   es un tenedor
  }
  else
  {
    if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
    { cout << "el número de procesos esperados es:    " << num_procesos << endl
           << "el número de procesos en ejecución es: " << num_procesos_actual << endl
           << "(programa abortado)" << endl ;
    }
  }

  MPI_Finalize( );
  return 0;
}

// ---------------------------------------------------------------------
