// Prof.: 7,8 de 8
/*

#############################################################################
##                 Arquitectura de Computadores                            ##
##    Grupo: -Andres Gutierrez Cepeda       Curso: 2018 / 2019             ##
##			 -Mario Cobo Gomez											   ##
#############################################################################

*/

/*
Tras ejecutar el codigo podemos observar ciertos datos que nos llama la
atencion de esta ejecucion.
De entre todas las secciones criticas la que mas tarda con diferencia es
critical. Ya que hasta que no termina, no inicia otro acceso.
La seccion de locks es la segunda que mas tarda. 
Esto es debido a la necesidad de crear locks para proteger el calculo 
del histograma y de su destruccion una vez finalizado su uso.
La seccion del atomic pose un tiempo similar al secuencial.
Con diferencia el que consigue reducir mas el tiempo es el reduction
disminuyendolo hasta casi una 3ª parte del original.
*/


#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define COLOUR_DEPTH 4

/*
###############################################################################
#   Nombre del metodo: imprimirHistograma									  #
#   Fecha de creacion: 8/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: histgr[] 										  #
#   Valor retornado: Vacio Imprime por pantalla el histograma				  #
#   Descripcion: Se trata de un metodo auxiliar que imprime por pantalla      #
#   el histograma con sus valores.											  #
###############################################################################
*/

void imprimirHistograma(int histgr[]){
	printf("#################################################################################################################################\n");
    
    for(int i = 0; i < 256; i++){
        printf("%d ",histgr[i]);
  }
   printf("\n#################################################################################################################################");
  
}

/*
###############################################################################
#   Nombre del metodo: compararHistogramas									  #
#   Fecha de creacion: 8/10/18												  #
#   Version: 1.0.0															  #
#   Argumentos de entrada: histgr[], histgrInicial[]						  #
#   Valor retornado: Un boolean que puede ser true o false					  #
#   Descripcion: Metodo auxiliar que compara el histograma obtenido en el     #
#   codigo secuencial con el histograma de la seccion critica correspondiente.#
#   Si ambos histogramas coincide devuelve true, si no, false.				  #
###############################################################################
*/

bool compararHistogramas(int histgr[], int histgrInicial[]){
	bool igual = true; //Prof.: algoritmo poco eficiente
    for(int i = 0; i < 256; i++){
        if(histgr[i] == histgrInicial[i]){
        igual=true;
        }else{
        	igual = false;
        }
  }
  return igual;
}

/*
###############################################################################
#   Nombre del metodo: createLocks											  #
#   Fecha de creacion: 8/10/18												  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: lock[]   										  #
#   Valor retornado: No devuelve nada										  #
#   Descripcion: Metodo auxiliar de la seccion de locks, el cual se encarga   #
#   de inicializar los locks. 												  #
###############################################################################
*/

void createLocks(omp_lock_t lock[]){
	for(int i = 0; i < 256; i++){
    	omp_init_lock(&lock[i]);
	}
}

/*
###############################################################################
#   Nombre del metodo: destroyLocks											  #
#   Fecha de creacion: 8/10/18												  #
#   Version: 1.0.0															  #
#   Argumentos de entrada: lock[]											  #
#   Valor retornado: No devuelve nada										  #
#   Descripcion: Metodo auxiliar de la seccion de locks, el cual se encarga   #
#   de destruir los locks.   												  #
###############################################################################
*/

void destroyLocks(omp_lock_t lock[]){
  for(int i = 0; i < 256; i++){
    omp_destroy_lock(&lock[i]);
  }
}

/*
###############################################################################
#   Nombre del metodo: computeGraySequential 								  #
#   Fecha de creacion: 8/10/18												  #
#   Version: 1.0.0															  #
#   Argumentos de entrada: QImage, *image, histgrInicial[]					  #
#   Valor retornado: Devuelve la diferencia entre el tiempo al iniciar la     #
#   seccion y el tiempo al finalizarla.										  #
#   Descripcion:Se realiza el histograma base de la imagen para poder         #
#   compararla con el resto de secciones criticas.                            #
#  	Este Segmento de codigo es el utilizado en la anterior práctica.          #
#   Este lo usaremos como base para poder calcular el tiempo de ejecucion del #
#   programa y poder comprobar si aumenta o disminuye su tiempo de ejecucion  #
#   en funcion de la seccion critica empleada.						          #
###############################################################################
*/

double computeGraySequential(QImage *image, int histgrInicial[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    histgrInicial[gray]+=1;
    *rgbpixel = QColor(gray, gray, gray).rgba();  //Prof.: sobra
  }
    printf("Histograma de la imagen \n\n\n"); 
    imprimirHistograma(histgrInicial);
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: computeGrayParallelCritical					    	  #
#   Fecha de creacion: 8/10/18	    										  #
#   Version: 1.0.0  														  #
#   Argumentos de entrada: QImage,*image, histgr[]							  #
#   Valor retornado: Devuelve la diferencia entre el tiempo al iniciar la     #
#   seccion y el tiempo al finalizarla.										  #
#   Descripcion: Se calcula el histograma generado debido al uso del parallel #
#   for y el critical en el código secuencial.                                #
#   Una vez detectada la seccion critica como la generacion del histograma    #
#	la controlaremos mediante la funcion critical.							  #
###############################################################################
*/

double computeGrayParallelCritical(QImage *image,int histgr[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for num_threads(  omp_get_num_procs() )       
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    *rgbpixel = QColor(gray, gray, gray).rgba();
    #pragma omp critical
    {	//Prof.: llaves innecesarias
      histgr[gray]+=1;
    }


  }

  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: computeGrayParallelAtomic					    	  #
#   Fecha de creacion: 8/10/18	    										  #
#   Version: 1.0.0  														  #
#   Argumentos de entrada: QImage,*image, histgr[]							  #
#   Valor retornado:Devuelve la diferencia entre el tiempo al iniciar la      #
#   seccion y el tiempo al finalizarla.					    				  #
#   Descripcion: Se calcula el histograma generado debido al uso del parallel #
#   for y el atomic en el código secuencial.                                  #
#   Sobre el codigo secuencial inicializaremos el parallel for antes de bucle #
#   para asi paralelizar el bucle.											  #
#   Dentro del bucle inicializaremos detectamos como seccion critica el       #
#   calcula del histograma y lo controlamos mediante la funcion atomic.       #
###############################################################################
*/

double computeGrayParallelAtomic(QImage *image, int histgr[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for num_threads(  omp_get_num_procs() )       
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    *rgbpixel = QColor(gray, gray, gray).rgba();
    #pragma omp atomic
    histgr[gray]+=1;
	
  }

  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: computeGrayParallelLocks 					    	  #
#   Fecha de creacion: 8/10/18	    										  #
#   Version: 1.0.0  														  #
#   Argumentos de entrada: QImage,*image, histgr[]							  #
#   Valor retornado: Devuelve la diferencia entre el tiempo al iniciar la     #
#   seccion y el tiempo al finalizarla.										  #
#   Descripcion: Se calcula el histograma generado debido al uso del parallel #
#   for y de locks en el código secuencial. Inicializaremos el objeto lock y  #
#   crearemos los locks para cada objeto lock.                                #
#   Dentro del bucle activaremos el lock cuando se va a modificar una parte   #
#	del histograma para que no se modifique la misma a la vez.                #
#   Tras el bucle for tendremos que destruir los locks creados.               #
###############################################################################
*/

double computeGrayParallelLocks(QImage *image, int histgr[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  omp_lock_t lock[256];
  createLocks(lock);
#pragma omp parallel for num_threads(  omp_get_num_procs() )       
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    *rgbpixel = QColor(gray, gray, gray).rgba(); //Prof.: sobra
    omp_set_lock(&lock[gray]);
    histgr[gray]++;
    omp_unset_lock(&lock[gray]);
  }
  destroyLocks(lock);
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: computeGrayParallelReduction					    	  #
#   Fecha de creacion: 8/10/18	    										  #
#   Version: 1.0.0  														  #
#   Argumentos de entrada: QImage,*image, histgr[]							  #
#   Valor retornado: Devuelve la diferencia entre el tiempo al iniciar la     #
#   seccion y el tiempo al finalizarla.                                       #
#   Descripcion: Se calcula el histograma generado al controlar la seccion    #
#	critica con reduction, para ello identificamos la operacion como una suma #
#	que se realiza a la hora de incrementar el valor del histograma           #
#	comprendido entre las posiciones 0 a 256.                                 #
###############################################################################
*/

double computeGrayParallelReduction(QImage *image, int histgr[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for num_threads(  omp_get_num_procs() ) reduction(+:histgr[0:256])       
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    histgr[gray]+=1;
    *rgbpixel = QColor(gray, gray, gray).rgba();  //Prof.: sobra
  }
  return omp_get_wtime() - start_time; 

}

/*
###############################################################################
#   Nombre del metodo: main                         				    	  #
#   Fecha de creacion: 8/10/18	    										  #
#   Version: 1.0.0  														  #
#   Argumentos de entrada: argc,*argv[]         							  #
#   Valor retornado: En caso de fallo se devuelve -1, si funciona bien 0      #
#   Descripcion: Se inicializa las variables histograma e histograma inicial  #
#   con las cuales calcularemos la escala de grises de la imagen.             #
#   Calcularemos el tiempo que necesita el sistema para cada control de       #
#   sección critica, utilizando como base el calculo la seccion secuencial    #
#   la cual no posee ninguna  seccion critica.                                #
#   Para verificar el correcto funcionamiento de las secciones criticas       #
#   compararemos el histograma inicial que fue calculado de manera secuencial #
#   con el histograma que se calculará en funcion de la seccion crítica que   #
#   corresponda.                                                              #
#   Al final de cada comparación, inicializaremos el histograma a 0.          #
###############################################################################
*/

int main(int argc, char *argv[])
{

	int histgrInicial[256];
    int histgr[256];
    memset(histgr, 0, sizeof(histgr));
    memset(histgrInicial, 0, sizeof(histgrInicial));

    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_1080p.bmp"); // ("c:\\test_1080p.bmp");
    if(qp.isNull())
    {
        printf("image not found\n");
		return -1;
    }
    
    QImage image = qp.toImage();
    
    //Secuencial
    QImage seqImage(image);
    double computeTime = computeGraySequential(&seqImage, histgrInicial);
    printf("\n------>  Tiempo Secuencial: %0.9f seconds\n", computeTime);


    //PAralelo Critical
    QImage auxImage(image);
    computeTime = computeGrayParallelCritical(&auxImage, histgr);
    printf("\n------>  Tiempo paralelo con Control Critical: %0.9f seconds\n", computeTime);    

	if (compararHistogramas(histgr,histgrInicial)) printf("\nAlgoritmo secuencial y paralelo critical dan el mismo histograma\n");
	else printf("\n\n\nAlgoritmo secuencial y paralelo critical dan distinto histograma\n");
   	
   	memset(histgr, 0, sizeof(histgr));


   	//Paralelo Atomic
    computeTime = computeGrayParallelAtomic(&auxImage, histgr);
    printf("\n------>  Tiempo paralelo con Control Atomic: %0.9f seconds\n", computeTime); 

    if (compararHistogramas(histgr,histgrInicial)) printf("\nAlgoritmo secuencial y paralelo Atomic dan el mismo histograma\n");
	else printf("\n\n\nAlgoritmo secuencial y paralelo Atomic dan distinto histograma\n");

	memset(histgr, 0, sizeof(histgr));


	//Paralelo Locks
    computeTime = computeGrayParallelLocks(&auxImage, histgr);
    printf("\n------>  Tiempo paralelo con Control Locks: %0.9f seconds\n", computeTime);     
	
	if (compararHistogramas(histgr,histgrInicial)) printf("\nAlgoritmo secuencial y paralelo Locks dan el mismo histograma\n");
	else printf("\n\n\nAlgoritmo secuencial y paralelo Locks dan distinto histograma\n");

	memset(histgr, 0, sizeof(histgr));


	//Paralelo Reduction
    computeTime = computeGrayParallelReduction(&auxImage, histgr);
    printf("\n------>  Tiempo paralelo con Control Reduction: %0.9f seconds\n", computeTime); 
       
    if (compararHistogramas(histgr,histgrInicial)) printf("\nAlgoritmo secuencial y paralelo Reduction dan el mismo histograma\n");
	else printf("\n\n\nAlgoritmo secuencial y paralelo Reduction dan distinto histograma\n");

    QPixmap pixmap = pixmap.fromImage(auxImage);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);
           
    view.show();
    return a.exec();
}
