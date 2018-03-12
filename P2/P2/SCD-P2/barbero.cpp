#include <iostream>
#include <cassert>
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

// Monitor Barberia
class Barberia : public HoareMonitor
{
private:
  CondVar silla;
  CondVar sala_espera;
  CondVar barbero;

public:

  Barberia(); // Constructor
  void cortarpelo(int i); // Metodo para cortar el pelo al cliente i
  void siguientecliente(); // Metodo para llamar al siguiente cliente
  void fincliente(); // Metodo para indicar el fin de un cliente
};

Barberia::Barberia()
{
  silla = newCondVar();
  sala_espera = newCondVar();
  barbero = newCondVar();
}

void Barberia::cortarpelo(int i )
{
  // Si el barbero esta ocupado, el cliente espera
  if(!silla.empty())
  {
    cout << "La silla esta ocupada, el cliente se queda en la sala de espera" << endl;
    sala_espera.wait();
  }

  // Se llama al cliente y se ocupa la silla
  barbero.signal();
  cout << "Se ocupa la silla. El barbero corta el pelo al cliente " << i << endl;
  silla.wait();
}

void Barberia::siguientecliente()
{
  // Si la silla y la sala de espera estan vacias
  if(sala_espera.empty() && silla.empty())
  {
    cout << "La sala de espera y la silla estan vacias. El barbero se pone a dormir" << endl;
    barbero.wait(); // El barbero espera (se duerme)
  }

  // El barbero llama al cliente
  cout << "El barbero llama al siguiente cliente" << endl;
  sala_espera.signal();
}

void Barberia::fincliente()
{
  cout << "El barbero termina de pelar al cliente" << endl;
  silla.signal(); // Se avisa de que la silla esta libre
}

// Funcion que simula la espera del cliente fuera de la barberia
void esperarfuerabarberia(int num_cliente)
{
  // calcular milisegundos de la espera aleatoria fuera de la barberia
  chrono::milliseconds duracion_espera(aleatorio<0, 2000>());

  // se informa de la espera

  // espera bloqueada un tiempo igual a duracion_espera milisegundos
  this_thread::sleep_for(duracion_espera);
}

// Funcion que simula cortar el pelo
void cortarpeloacliente()
{
  // calcular milisegundos del tiempo de cortar el pelo a un Cliente
  chrono::milliseconds duracion_corte(aleatorio<0, 2500>());

  // Espera bloqueada un tiempo igual a duracion_corte
  this_thread::sleep_for(duracion_corte);
}

// Funcion de la hebra cliente
void cliente (MRef<Barberia> barberia, int num_cliente)
{
  while (true)
  {
    barberia->cortarpelo(num_cliente);
    esperarfuerabarberia(num_cliente);
  }
}

// Funcion de la hebra barbero
void barbero(MRef<Barberia> barberia)
{
  while(true)
  {
    barberia->siguientecliente();
    cortarpeloacliente();
    barberia->fincliente();
  }
}

int main(int argc, char ** argv)
{
  if(argc != 2){
    cerr << "USO: num_clientes" << endl;
    exit(0);
  }

  int num_clientes = atoi(argv[1]);

  MRef <Barberia> barberia = Create<Barberia>();

  thread hebra_barbero, clientes[num_clientes];

  hebra_barbero = thread(barbero, barberia);

  for(int i = 0; i < num_clientes; i++)
    clientes[i] = thread(cliente, barberia, i);

  for(int i = 0; i < num_clientes; i++)
    clientes[i].join();

  hebra_barbero.join();
}
