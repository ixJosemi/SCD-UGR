#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Estanco.h"
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

Estanco::Estanco()
{
  ingr_comun = -1;
  hay_ingrediente = false;

  for(int i = 0; i < 3; i++)
    fumador[i] = newCondVar();

  producir = newCondVar();
}

void Estanco::obtenerIngrediente(int miIngrediente)
{
  while(true)
  {
    if(ingr_comun != miIngrediente)
      fumador[miIngrediente].wait();

      hay_ingrediente = false; // Recogemos el ingrediente
      cout << "Fumador " << miIngrediente << " fumando" << endl;
      producir.signal(); // Se indica que ya se puede fumar
  }
}

void Estanco::ponerIngrediente(int ingrediente)
{
  while(true)
  {
    // Colocamos el ingrediente y lo comunicamos
    ingr_comun = ingrediente;
    hay_ingrediente = true;

    cout << "El estanquero pone el ingrediente " << ingrediente << endl;
    fumador[ingr_comun].signal(); // Llamamos al fumador que necesita dicho ingrediente
  }
}

void Estanco::esperarRecogidaIngrediente()
{
  while(true)
  {
    if(hay_ingrediente)
      producir.wait(); // Si hay un ingrediente esperamos a que se retire para poner otro
  }
}
