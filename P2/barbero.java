import java.util.Random;
import monitor.* ;

// ************************************************************************

class Barberia extends AbstractMonitor
{
 private Condition silla = makeCondition();
 private Condition sala_espera = makeCondition();
 private Condition barbero = makeCondition();

 //El constructor es el por defecto

 public void cortarPelo(){
  enter();
   if(!silla.isEmpty())
    sala_espera.await();

   barbero.signal();
   silla.await();
  leave();
 }

 public void siguienteCliente(){
  enter();
   if(silla.isEmpty() && sala_espera.isEmpty()){
    System.out.println("El barbero se pone a dormir");
    barbero.await();
   }

   System.out.println("Hay " + sala_espera.count() + " clientes esperando");
   System.out.println("El barbero LLAMA al siguiente");
   sala_espera.signal();
  leave();
 }

 public void finCliente(){
  enter();
   System.out.println("El barbero TERMINA de pelar");
   silla.signal();
  leave();
 }

}

// ************************************************************************

class Barbero implements Runnable
{
 private Barberia barberia;
 public Thread thr;

 public Barbero(Barberia p_barberia){
  barberia = p_barberia;
  thr = new Thread(this, "barbero");
 }

 public void run(){
  while(true){
   barberia.siguienteCliente();
   System.out.println("El barbero EMPIEZA a pelar");
   aux.dormir_max(2500);   //Mientras corta el pelo
   barberia.finCliente();
  }
 }
}

// ************************************************************************

class Cliente implements Runnable
{
 private Barberia barberia;
 public Thread thr;
 private int num_cliente;

 public Cliente(Barberia p_barberia, int numero){
  barberia = p_barberia;
  num_cliente = numero;
  thr= new Thread(this, "cliente " + num_cliente);
 }

 public void run(){
  while(true){
   System.out.println("El cliente " + num_cliente + " ENTRA a la barberia");
   barberia.cortarPelo();
   System.out.println("El cliente " + num_cliente + " SALE de la barberia");
   aux.dormir_max(2000);   //Cliente fuera de la barberia un tiempo
  }
 }
}

// ************************************************************************

class aux
{
 static Random genAlea = new Random() ;
 static void dormir_max( int milisecsMax ){
  try{
   Thread.sleep( genAlea.nextInt( milisecsMax ) ) ;
  }

  catch( InterruptedException e ){
   System.err.println("sleep interumpido en ’aux.dormir_max()’");
  }
 }
}

// ************************************************************************

class MainBarbero
{
 public static void main(String[] args)
 {
  if( args.length != 1 ) {
       System.err.println("Uso: num_clientes");
       return ;
     }

     // leer parametros, crear vectore y barberia
     Cliente[] cliente = new Cliente[Integer.parseInt(args[0])] ;   //Vector con los clientes
  Barberia barberia = new Barberia();

  // crear hebras
  Barbero barbero = new Barbero(barberia);
  for(int i = 0; i < cliente.length; i++)
     cliente[i] = new Cliente(barberia, i) ;

  // poner en marcha las hebras
  barbero.thr.start();
  for(int i = 0; i < cliente.length; i++)
   cliente[i].thr.start();

 }
}
