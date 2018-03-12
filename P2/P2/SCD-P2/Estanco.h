#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

class Estanco : public HoareMonitor
{
private:
  int ingr_comun;
  bool hay_ingrediente;

  CondVar producir;
  CondVar fumador[3];

public:
  Estanco();
  void obtenerIngrediente(int miIngrediente);
  void ponerIngrediente(int ingrediente);
  void esperarRecogidaIngrediente();
};
