// Prof.: 9 de 10

/*

#############################################################################
##                 Arquitectura de Computadores                            ##
##    Grupo: -Andres Gutierrez Cepeda       Curso: 2018 / 2019             ##
##			 -Mario Cobo Gomez											   ##
#############################################################################

*/



#include <QtGui/QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define N 5
const int M=N/2;
int matrizGauss[N][N] =	{{ 1, 4, 6, 4,1 },
						 { 4,16,24,16,4 },
						 { 6,24,36,24,6 },
						 { 4,16,24,16,4 },
						 { 1, 4, 6, 4,1}};
int vectorGauss[N] = { 1, 4, 6, 4, 1 };

int alto, ancho;
#define max(num1, num2) (num1>num2 ? num1 : num2)
#define min(num1, num2) (num1<num2 ? num1 : num2)



double naive_matriz(QImage* image, QImage* result) {
    
  int h, w, i,j;
  int red, green, blue;
  int mini, supi,minj, supj;
  QRgb aux;  
  double start_time = omp_get_wtime();

  for (h = 0; h < alto; h++)
    
    for (w = 0; w < ancho ; w++) {

    /* Aplicamos el kernel (matrizGauss dividida entre 256) al entorno de cada pixel de coordenadas (w,h)
     Corresponde multiplicar la componente matrizGauss[i,j] por el pixel de coordenadas (w-M+j, h-M+i)
     Pero ha de cumplirse 0<=w-M+j<ancho y 0<=h-M+i<alto. Por tanto: M-w<=j<ancho+M-w y M-h<=i<alto+M-h
     Además, los índices i y j de la matriz deben cumplir 0<=j<N, 0<=i<N
     Se deduce:  máx(M-w,0) <= j < mín(ancho+M-w,N); máx(M-h,0) <= i < mín(alto+M-h,N)*/
    red=green=blue=0;
    mini = max((M-h),0); minj = max((M-w),0);         // Ver comentario anterior
    supi = min((alto+M-h),N); supj = min((ancho+M-w),N);      // Íd.
    for (i=mini; i<supi; i++)
        for (j=minj; j<supj; j++) {
      aux = image->pixel(w-M+j, h-M+i);
      red += matrizGauss[i][j]*qRed(aux);
      green += matrizGauss[i][j]*qGreen(aux);
      blue += matrizGauss[i][j]*qBlue(aux);
    };

    red /= 256; green /= 256; blue /= 256;
    result->setPixel(w,h, QColor(red, green, blue).rgba());
      
  }
  
  return omp_get_wtime() - start_time; 
}

/*
###############################################################################
#   Nombre del metodo: aplicar_vect_vertical								  #
#   Fecha de creacion: 11/11/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: image, red, green, blue							  #
#   Valor retornado: nada								       				  #
#   Descripcion: Se recorre la imagen desde el limite superior izquierda      #
#	hasta el limite inferior derecha, de manera que se toma el pixel          #
#	correspondiente a la columna sobre la que se tiene que aplicar de manera  #
#	vertical el vector gausiano. Una vez se toma el pixel sus valores de      #
#	rojo, verde y azul se multiplican por el vector gauss y se almacenan      #
#	dichos valores en su posicion correspondiente de cada vector dinamico.	  #
###############################################################################
*/

void aplicar_vect_vertical(QImage* image,int *red,int *green,int *blue) {
  int h, w, i;
  int mini, supi;
  QRgb aux;  

  for (h = 0; h < alto; h++){
    
    for (w = 0; w < ancho ; w++) {

    mini = max((M-h),0);   // Prof.:    mejor entre los dos for
    supi = min((alto+M-h),N); // Prof.: íd. 

    for (i=mini; i<supi; i++){

      aux = image->pixel(w, h-M+i);
      red [h*ancho+w] += vectorGauss[i]*qRed(aux);
      green [h*ancho+w]+= vectorGauss[i]*qGreen(aux);
      blue [h*ancho+w]+= vectorGauss[i]*qBlue(aux);
    };
      
   }
  }    
} 

/*
###############################################################################
#   Nombre del metodo: aplicar_vect_horizontal								  #
#   Fecha de creacion: 11/11/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: result, red, green, blue 						  #
#   Valor retornado: vacio     				 								  #
#   Descripcion: Se declaran tres enteros correspondientes a cada valor de    #
#	rojo, verde y azul del pixel que se va a asignar a la nueva imagen        #
#	se recorren los vectores dinamicos de la misma manera que en el           #
#	metodo anterior se recorria la imagen, pero en esta ocasion en lugar de   #
#	almacenarse el valor en los vectores dinamicos se almacenan en los enteros#
#	para asi asignar sus valores al pixel, el valor que se asigna a cada      #
#	entero sera el resultado de multiplicar el valor almacenado en el vector  #
#	por el vector gauss horizontal, el valor resultante se divide entre       #
#	256 y se almacena en la imagen resultante.		                          #
###############################################################################
*/

void aplicar_vect_horizontal(QImage* result,int *red,int *green,int *blue) {
  int h, w, j;
  int redAux, greenAux, blueAux;
  
  int minj, supj; 
 
  for (h = 0; h < alto; h++){
    
    for (w = 0; w < ancho ; w++) {
   
    minj = max((M-w),0);  
    supj = min((ancho+M-w),N);   
    redAux=blueAux=greenAux=0;
   
    for (j=minj; j<supj; j++){
     
      redAux += vectorGauss[j]*(red[h*ancho+w-M+j]);
      greenAux += vectorGauss[j]*(green[h*ancho+w-M+j]);
      blueAux += vectorGauss[j]*(blue[h*ancho+w-M+j]); // Prof.: el mismo índice se calcula 3 veces
    };
      redAux /= 256; greenAux /= 256; blueAux /= 256;
      result->setPixel(w,h, QColor(redAux, greenAux, blueAux).rgba());
   }
  }   
}


/*
###############################################################################
#   Nombre del metodo: aplicar_vect_vertical_parall							  #
#   Fecha de creacion: 11/11/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: image, red, green, blue							  #
#   Valor retornado: nada								       				  #
#   Descripcion: Igual que el metodo anterior pero paralelizado y realizando  #
#	el control de la seccion critica.										  #
###############################################################################
*/

void aplicar_vect_vertical_parall(QImage* image,int *red,int *green,int *blue) {
  int h, w, i;
  int mini, supi;
  QRgb aux;  

  #pragma omp parallel for private(w, i, mini, supi, aux)
  for (h = 0; h < alto; h++){
    
    for (w = 0; w < ancho ; w++) {

    mini = max((M-h),0);       
    supi = min((alto+M-h),N);  

    for (i=mini; i<supi; i++){

      aux = image->pixel(w, h-M+i);
      red [h*ancho+w] += vectorGauss[i]*qRed(aux);
      green [h*ancho+w]+= vectorGauss[i]*qGreen(aux);
      blue [h*ancho+w]+= vectorGauss[i]*qBlue(aux);
    };
      
   }
  }    
} 

/*
###############################################################################
#   Nombre del metodo: aplicar_vect_horizontal								  #
#   Fecha de creacion: 11/11/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: result, red, green, blue 						  #
#   Valor retornado: vacio     				 								  #
#   Descripcion: Igual que el metodo anterior pero paralelizado y realizando  #
#	el control de seccion critica.					                          #
###############################################################################
*/

void aplicar_vect_horizontal_parall(QImage* result,int *red,int *green,int *blue) {
  int h, w, j;
  int redAux, greenAux, blueAux;
  
  int minj, supj; 
 
  #pragma omp parallel for private(w, j, minj, supj, redAux, greenAux, blueAux) 
  for (h = 0; h < alto; h++)
    
    for (w = 0; w < ancho ; w++) {
   
    minj = max((M-w),0);  
    supj = min((ancho+M-w),N);   
    redAux=blueAux=greenAux=0;
   
    for (j=minj; j<supj; j++){

      redAux += vectorGauss[j]*(red[h*ancho+w-M+j]);
      greenAux += vectorGauss[j]*(green[h*ancho+w-M+j]);
      blueAux += vectorGauss[j]*(blue[h*ancho+w-M+j]);
    };
      
      redAux /= 256; greenAux /= 256; blueAux /= 256;

      #pragma omp critical /*Debido a lo comentado en la clase de teoria al formarse una seccion critica en esta parte del codigo
      hay que controlarla mediante critical, ya que la privatizacion de variables no es suficiente.*/
      result->setPixel(w,h, QColor(redAux, greenAux, blueAux).rgba());
   }
     
}


/*
###############################################################################
#   Nombre del metodo: separa_vectores								          #
#   Fecha de creacion: 11/11/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: image, matrGaussImage, opcion					  #
#   Valor retornado: Tiempo de ejecucion del metodo         				  #
#   Descripcion: Se realiza la declaracion de los distintos vectores dinamicos#
#	se les asigna su espacio de memoria y se inicializan a 0				  #
#	a su vez se realiza la llamada a los dos metodos que permiten la          #
#	realizacion del ejercicio.												  #
###############################################################################
*/

double separa_vectores(QImage* image,QImage* matrGaussImage,int opcion) {
  
  double start_time = omp_get_wtime();
  int *red, *green, *blue;
  red=(int *)malloc(alto*ancho*sizeof(int));
  green=(int *)malloc(alto*ancho*sizeof(int));
  blue=(int *)malloc(alto*ancho*sizeof(int));

  memset(red,0,sizeof(int)*alto*ancho);
  memset(green,0,sizeof(int)*alto*ancho);
  memset(blue,0,sizeof(int)*alto*ancho);

  switch (opcion){
    	case 1:	
		    aplicar_vect_vertical(image, red,green,blue);
  			aplicar_vect_horizontal(matrGaussImage,red,green,blue);
    	break;

    	case 2:
    		aplicar_vect_vertical_parall(image, red,green,blue);
    		aplicar_vect_horizontal_parall(matrGaussImage,red,green,blue);
    	break;

    	case 3:
    		aplicar_vect_vertical(image, red,green,blue);
    		aplicar_vect_horizontal_parall(matrGaussImage,red,green,blue);
    	break;

    	case 4:
    		aplicar_vect_vertical_parall(image, red,green,blue);
    		aplicar_vect_horizontal(matrGaussImage,red,green,blue);
    	break;
    }



    return omp_get_wtime() - start_time;     
} 





int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    int opcion;

    if (argc != 2) {printf("Vuelva a ejecutar. Uso: <ejecutable> <archivo imagen>  \n"); return -1;} 
    QPixmap qp = QPixmap(argv[1]);
    if(qp.isNull()) { printf("image not found\n"); return -1;}
    
    do{
    	printf("Elija la opcion de separa vectores: \n1-No paralelizado.\n2-Paralelizado\n3-Horizontal Paralelizado\n4-Vertical paralelizado\n");
    	scanf("%d",&opcion);
    }while (opcion<1||opcion>4);


    QImage image = qp.toImage();
    
    alto = image.height(); ancho = image.width();
    QImage ImageAux(image);
  
    double computeTime = naive_matriz(&image, &ImageAux);
    printf("naive_matriz time: %0.9f seconds\n", computeTime);

    QImage matrGaussImage(image);
   
    computeTime = separa_vectores(&image,&matrGaussImage,opcion);
	printf("separa vectores time: %0.9f seconds\n", computeTime);
		    
    
    
    if (ImageAux == matrGaussImage) printf("Algoritmo native y separado dan la misma imagen\n");
    else printf("Algoritmo native y separado dan distinta imagen\n");    
    
    QPixmap pixmap = pixmap.fromImage(matrGaussImage);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);

    view.show();
    return a.exec();
}
