#include <iostream>
#include <thread>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template < int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// Monitor Estanco
class Estanco : public HoareMonitor
{
private:
  int ingr_comun;
  bool hay_ingrediente;

  CondVar producir;
  CondVar fumador[3];

public:
  Estanco(); // Constructor
  void obteneringrediente(int miIngrediente); // Metodo para obtener ingrediente
  void poneringrediente(int ingrediente); // Metodo para poner ingrediente
  void esperarrecogidaingrediente(); // Metodo para esperar ingrediente
};

Estanco::Estanco()
{
  ingr_comun = -1;
  hay_ingrediente = false;

  for(int i = 0; i < 3; i++)
    fumador[i] = newCondVar();

  producir = newCondVar();
}

void Estanco::obteneringrediente(int miIngrediente)
{
  if(ingr_comun != miIngrediente)
    fumador[miIngrediente].wait(); // Se espera al ingrediente correspondiente

    hay_ingrediente = false; // Recogemos el ingrediente
    producir.signal(); // Se indica que ya se puede fumar
}

void Estanco::poneringrediente(int ingrediente)
{
  // Colocamos el ingrediente y lo comunicamos
  ingr_comun = ingrediente;
  hay_ingrediente = true;

  cout << "El estanquero pone el ingrediente " << ingrediente << endl;
  fumador[ingr_comun].signal(); // Llamamos al fumador que necesita dicho ingrediente
}

void Estanco::esperarrecogidaingrediente()
{
  if(hay_ingrediente)
    producir.wait(); // Si hay un ingrediente esperamos a que se retire para poner otro
}

//----------------------------------------------------------------------
// funcion que produce un ingrediente aleatorio entre 0 y 2

int producir()
{
    return aleatorio<0, 2>();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(MRef<Estanco> estanco )
{
    while( true ){
      int ingr = producir();
      estanco->poneringrediente(ingr);
      estanco->esperarrecogidaingrediente();
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
   cout << "Fumador " << num_fumador << "  :"
        << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << " : termina de fumar, comienza espera de ingrediente." << endl;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador

void funcion_hebra_fumador(MRef<Estanco> estanco, int num_fumador )
{
    while( true )
    {
      estanco->obteneringrediente(num_fumador);
      fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

int main()
{
  // creamos el monitor
  MRef <Estanco> estanco = Create<Estanco>();

  // creamos y lanzamos las hebras
  thread estanquero(funcion_hebra_estanquero, estanco),
         fum1(funcion_hebra_fumador, estanco, 0),
         fum2(funcion_hebra_fumador, estanco, 1),
         fum3(funcion_hebra_fumador, estanco, 2);

  // esperamos a que terminen las hebras
  fum1.join();
  fum2.join();
  fum3.join();
  estanquero.join();

  return 0;
}
