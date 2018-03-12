#include <iostream>
#include <cassert>
#include <thread>
#include <random>
#include <chrono>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

const int num_items = 40;
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

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

// Monitor ProdCons
class ProdCons : public HoareMonitor
{
private:
  static const int tam_vec = 10;
  int primera_libre, primera_ocupada;
  int buffer[tam_vec];

  CondVar puede_producir;
  CondVar puede_consumir;

public:
  ProdCons(); // Constructor
  void producir(int valor); // funcion llamada por el productor
  int consumir(); // funcion llamada por el consumidor
};

ProdCons::ProdCons(){
  primera_libre = 0;
  primera_ocupada = 0;
  puede_producir = newCondVar();
  puede_consumir = newCondVar();
}

void ProdCons::producir(int valor)
{
  if(primera_libre == tam_vec)
    puede_producir.wait();

  buffer[primera_libre] = valor;
  primera_libre++;
  puede_consumir.signal();

  primera_libre %= tam_vec;
}

int ProdCons::consumir()
{
  if(primera_libre == 0)
    puede_consumir.wait();

  int valor = buffer[primera_ocupada];
  primera_ocupada++;
  puede_producir.signal();

  primera_ocupada %= tam_vec;

  return valor;
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}

//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//**********************************************************************
// Funcion de la hebra productora para una pila acotada (LIFO)
//----------------------------------------------------------------------

void funcion_hebra_productora(MRef <ProdCons> prodcons, int num_prods)
{
  for(unsigned i = 0; i < num_items/num_prods; i++)
  {
    int valor = producir_dato();
    prodcons->producir(valor);
  }
}

//**********************************************************************
// Funcion de la hebra consumidora para una pila acotada (LIFO)
//----------------------------------------------------------------------

void funcion_hebra_consumidora(MRef <ProdCons> prodcons, int num_cons)
{
  for(unsigned i = 0; i < num_items/num_cons; i++)
  {
    int valor = prodcons->consumir( );
    consumir_dato(valor);
  }
}

//----------------------------------------------------------------------

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cerr << "USO: num_productores num_consumidores" << endl;
  }

  int num_productores = atoi(argv[1]);
  int num_consumidores = atoi(argv[2]);

  cout << "--------------------------------------------------------" << endl
       << "Problema de los productores-consumidores (solución LIFO)." << endl
       << "--------------------------------------------------------" << endl
       << flush ;

  MRef <ProdCons> prodcons = Create<ProdCons>();

  thread productores[num_productores], consumidores[num_consumidores];

  for(int i = 0; i < num_productores; i++)
    productores[i] = thread(funcion_hebra_productora, prodcons, num_productores);

  for(int i = 0; i < num_consumidores; i++)
    consumidores[i] = thread(funcion_hebra_consumidora, prodcons, num_consumidores);

  for(int i = 0; i < num_productores; i++)
    productores[i].join();

  for(int i = 0; i < num_consumidores; i++)
    consumidores[i].join();

  test_contadores();

  cout << "\n******FIN DEL PROGRAMA ******" << endl;

  return 0;
}
