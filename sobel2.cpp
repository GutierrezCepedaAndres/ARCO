
/*

#############################################################################
##                 Arquitectura de Computadores                            ##
##    Grupo: -Andres Gutierrez Cepeda       Curso: 2018 / 2019             ##
##			 -Mario Cobo Gomez											   ##
#############################################################################

*/
// Prof.: 4,5 de 10



#include <QtGui/QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>
#include <math.h>

#define COLOUR_DEPTH 4


/*
Se declaran los dos tipos de kernel
*/
int weight[3][3] = 	{{ 1,  2,  1 },
					 { 0,  0,  0 },
					 { -1,  -2,  -1 }
					};

int height[3][3] =  {{ -1,  0,  1 },
           { -2,  0,  2 },
           { -1,  0,  1 }
          };



/*
###############################################################################
#   Nombre del metodo: SobelBasico									          #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: *srcImage, *dstImage 							  #
#   Valor retornado: Tiempo de ejecucion del metodo         				  #
#   Descripcion: Se realiza el producto de convolucion, se calcula el         #
#   valor del nuevo pixel a traves de los valores de sus vecinos.			  #
###############################################################################
*/
// Prof.: *dstImage es la salida de la función (no es entrada)
double SobelBasico(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValue, ii, jj, blue;

  for (ii = 1; ii < srcImage->height() - 1; ii++) {  	// Recorremos la imagen pixel a pixel, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      // Aplicamos el kernel weight[3][3] al pixel y su entorno
      pixelValue = 0;
      for (int i = -1; i <= 1; i++) {					// Recorremos el kernel weight[3][3]
          for (int j = -1; j <= 1; j++) {
			blue = qBlue(srcImage->pixel(jj+j, ii+i));	// Sintaxis pixel: pixel(columna, fila), es decir pixel(x,y)
            pixelValue += weight[i + 1][j + 1] * blue;	// En pixelValue se calcula el componente y del gradiente   
          }
      }

      if (pixelValue > 255) pixelValue = 255;
      if (pixelValue < 0) pixelValue = 0;
	
      dstImage->setPixel(jj,ii, QColor(pixelValue, pixelValue, pixelValue).rgba());	// Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: SobelLocal									          #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: *srcImage, *dstImage  							  #
#   Valor retornado: Tiempo de ejecucion				                      #
#   Descripcion: Se ejecuta el primero de los kernels pero manteniendo en     #
#   unas matrices auxiliares los valores ya obtenidos de la imagen,           #
#   unicamente se obtienen los 3 nuevos valores vecinos.         			  #
###############################################################################
*/

double SobelLocal(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValue, ii, jj, blue;
  int MatrizPixels [3][3];
  int MatrizPixelsAux [3][3];

  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen pixel a pixel, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      // Aplicamos el kernel weight[3][3] al pixel y su entorno
      pixelValue = 0;
      if(ii == 1 && jj==1){  /* en caso de encontrarse en la primera iteracion del bucle no habra ningun valor guardado en las matrices, por ello se ejecuta como en el sobel basico*/
        for (int i = -1; i <= 1; i++) {         // Recorremos el kernel weight[3][3]
            for (int j = -1; j <= 1; j++) {
        blue = qBlue(srcImage->pixel(jj+j, ii+i));  // Sintaxis pixel: pixel(columna, fila), es decir pixel(x,y)
              pixelValue += weight[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
              MatrizPixels[1+i][1+j]=srcImage->pixel(ii+i,jj+j);	// Prof.: mal
              MatrizPixelsAux[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
            }
        }
      }else{
      	/* De esta manera se guardan en las matrices auxiliares los valores ya registrados de los pixels adyacentes, teniendo que acceder unicamente a los 3 nuevos vecinos*/
        MatrizPixels[0][0] = MatrizPixelsAux[0][1];
        MatrizPixels[1][0] = MatrizPixelsAux[1][1];
        MatrizPixels[2][0] = MatrizPixelsAux[2][1];
        MatrizPixels[0][1] = MatrizPixelsAux[0][2];
        MatrizPixels[1][1] = MatrizPixelsAux[1][2];
        MatrizPixels[2][1] = MatrizPixelsAux[2][2];
        // Prof.: ¿¡!?        
        MatrizPixelsAux[0][0] = MatrizPixels[0][1];
        MatrizPixelsAux[1][0] = MatrizPixels[1][1];
        MatrizPixelsAux[2][0] = MatrizPixels[2][1];
        MatrizPixelsAux[0][1] = MatrizPixels[0][2];
        MatrizPixelsAux[1][1] = MatrizPixels[1][2];
        MatrizPixelsAux[2][1] = MatrizPixels[2][2];
        
        /*Se calcula unicamente los nuevos vecinos, los viejos ya se tienen localizados en las matrices*/
        for(int k = -1; k <= 1; k++){
          MatrizPixelsAux[k+1][2] = srcImage->pixel(jj+k, ii+k);
          MatrizPixels[k+1][2] = srcImage->pixel(jj+k, ii+k);
          // Prof.: demasiadas lecturas en la imagen
        }      

        for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
        blue = qBlue(MatrizPixels[1+j][1+i]);
            pixelValue += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
          }
        }

      }

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255; /*Los unicos valores que se permiten de pixel son o negro o blanco*/
      // Prof.: comentario anterior falso
      if (new_value < 0) new_value = 0;
  
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: SobelLocalParallel									  #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: *srcImage, *dstImage 							  #
#   Valor retornado: Tiempo de ejecucion				                      #
#   Descripcion: Se paraleliza el metodo anterior pero controlando las        #
#   variables que deben de ser privadas para el control de la seccion		  #
#   critica.											                      #
###############################################################################
*/

double SobelLocalParallel(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValue, ii, jj, blue;
  int MatrizPixels [3][3];
  int MatrizPixelsAux [3][3];

  #pragma omp parallel for private(ii,jj, blue, pixelValue, MatrizPixels,MatrizPixelsAux)
  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen pixel a pixel, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      // Aplicamos el kernel weight[3][3] al pixel y su entorno
      pixelValue = 0;
      if(ii == 1 && jj==1){  
        for (int i = -1; i <= 1; i++) {         // Recorremos el kernel weight[3][3]
            for (int j = -1; j <= 1; j++) {
        blue = qBlue(srcImage->pixel(jj+j, ii+i));  // Sintaxis pixel: pixel(columna, fila), es decir pixel(x,y)
              pixelValue += weight[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
              MatrizPixels[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
              MatrizPixelsAux[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
            }
        }
      }else{
        MatrizPixels[0][0] = MatrizPixelsAux[0][1];
        MatrizPixels[1][0] = MatrizPixelsAux[1][1];
        MatrizPixels[2][0] = MatrizPixelsAux[2][1];
        MatrizPixels[0][1] = MatrizPixelsAux[0][2];
        MatrizPixels[1][1] = MatrizPixelsAux[1][2];
        MatrizPixels[2][1] = MatrizPixelsAux[2][2];
        MatrizPixelsAux[0][0] = MatrizPixels[0][1];
        MatrizPixelsAux[1][0] = MatrizPixels[1][1];
        MatrizPixelsAux[2][0] = MatrizPixels[2][1];
        MatrizPixelsAux[0][1] = MatrizPixels[0][2];
        MatrizPixelsAux[1][1] = MatrizPixels[1][2];
        MatrizPixelsAux[2][1] = MatrizPixels[2][2];

        for(int k = -1; k <= 1; k++){
          MatrizPixelsAux[k+1][2] = srcImage->pixel(jj+k, ii+k);
          MatrizPixels[k+1][2] = srcImage->pixel(jj+k, ii+k);
        }      

        for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
        blue = qBlue(MatrizPixels[1+j][1+i]);
            pixelValue += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
          }
        }

      }

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
  
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: SobelCompleto								          #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: *srcImage, *dstImage  				              #
#   Valor retornado: Tiempo de ejecucion.				                      #
#   Descripcion: En esta ocasion en lugar de ejecutar un unico kernel se      #
#   ejecutan tanto el de altura como el de anchura para obtener una imagen	  #
#   mucho mas trabajada.											          #
###############################################################################
*/

double SobelCompleto(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValueX,pixelValueY, ii, jj, blue;
  int MatrizPixels [3][3];
  int MatrizPixelsAux [3][3];

  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen pixel a pixel, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      // Aplicamos el kernel weight[3][3] al pixel y su entorno
      pixelValueX = 0; /*Se declaran dos variables de pixel value uno para cada tipo de ejecucion de kernel*/
      pixelValueY = 0;
      if(ii == 1 && jj==1){  
        for (int i = -1; i <= 1; i++) {         // Recorremos el kernel weight[3][3]
            for (int j = -1; j <= 1; j++) {
        blue = qBlue(srcImage->pixel(jj+j, ii+i));  // Sintaxis pixel: pixel(columna, fila), es decir pixel(x,y)
              pixelValueX += weight[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente mediante el uso del kernel de anchura
              pixelValueY += height[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente mediante el uso del kernel de altura
              MatrizPixels[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
              MatrizPixelsAux[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
            }
        }
      }else{
        MatrizPixels[0][0] = MatrizPixelsAux[0][1];
        MatrizPixels[1][0] = MatrizPixelsAux[1][1];
        MatrizPixels[2][0] = MatrizPixelsAux[2][1];
        MatrizPixels[0][1] = MatrizPixelsAux[0][2];
        MatrizPixels[1][1] = MatrizPixelsAux[1][2];
        MatrizPixels[2][1] = MatrizPixelsAux[2][2];
        MatrizPixelsAux[0][0] = MatrizPixels[0][1];
        MatrizPixelsAux[1][0] = MatrizPixels[1][1];
        MatrizPixelsAux[2][0] = MatrizPixels[2][1];
        MatrizPixelsAux[0][1] = MatrizPixels[0][2];
        MatrizPixelsAux[1][1] = MatrizPixels[1][2];
        MatrizPixelsAux[2][1] = MatrizPixels[2][2];

        for(int k = -1; k <= 1; k++){
          MatrizPixelsAux[k+1][2] = srcImage->pixel(jj+k, ii+k);
          MatrizPixels[k+1][2] = srcImage->pixel(jj+k, ii+k);
        }      

        for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
        blue = qBlue(MatrizPixels[1+j][1+i]);
            pixelValueX += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente mediante el kernel de anchura
            pixelValueY += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente mediante el kernel de altura
          }
        }

      }

      int new_value = sqrt((pixelValueX*pixelValueX)+(pixelValueY*pixelValueY)); /*Se calcula el nuevo valor del pixel mediante la operacion
      	que se puede ver en las diapositivas, mediante la raiz cuadrada de la suma de los dos valores del gradiente al cuadrado*/
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
  
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}


/*
###############################################################################
#   Nombre del metodo: SobelCompletoParallel								  #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: *srcImage, *dstImage  							  #
#   Valor retornado: Tiempo de ejecucion del metodo             			  #
#   Descripcion: Se paraleliza el metodo anterior controlando la seccion      #
#   critica mediante el uso de privatizar las variables.			          #
###############################################################################
*/

double SobelCompletoParallel(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValueX,pixelValueY, ii, jj, blue;
  int MatrizPixels [3][3];
  int MatrizPixelsAux [3][3];


  #pragma omp parallel for private(ii,jj, blue, pixelValueX,pixelValueY, MatrizPixels,MatrizPixelsAux)
  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen pixel a pixel, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      // Aplicamos el kernel weight[3][3] al pixel y su entorno
      pixelValueX = 0;
      pixelValueY = 0;
      if(ii == 1 && jj==1){  
        for (int i = -1; i <= 1; i++) {         // Recorremos el kernel weight[3][3]
            for (int j = -1; j <= 1; j++) {
        blue = qBlue(srcImage->pixel(jj+j, ii+i));  // Sintaxis pixel: pixel(columna, fila), es decir pixel(x,y)
              pixelValueX += weight[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
              pixelValueY += height[i + 1][j + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
              MatrizPixels[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
              MatrizPixelsAux[1+i][1+j]=srcImage->pixel(ii+i,jj+j);
            }
        }
      }else{

        MatrizPixels[0][0] = MatrizPixelsAux[0][1];
        MatrizPixels[1][0] = MatrizPixelsAux[1][1];
        MatrizPixels[2][0] = MatrizPixelsAux[2][1];
        MatrizPixels[0][1] = MatrizPixelsAux[0][2];
        MatrizPixels[1][1] = MatrizPixelsAux[1][2];
        MatrizPixels[2][1] = MatrizPixelsAux[2][2];
        
        MatrizPixelsAux[0][0] = MatrizPixels[0][1];
        MatrizPixelsAux[1][0] = MatrizPixels[1][1];
        MatrizPixelsAux[2][0] = MatrizPixels[2][1];
        MatrizPixelsAux[0][1] = MatrizPixels[0][2];
        MatrizPixelsAux[1][1] = MatrizPixels[1][2];
        MatrizPixelsAux[2][1] = MatrizPixels[2][2];
        

        for(int k = -1; k <= 1; k++){
          MatrizPixelsAux[k+1][2] = srcImage->pixel(jj+k, ii+k);
          MatrizPixels[k+1][2] = srcImage->pixel(jj+k, ii+k);
        }      

        for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
        blue = qBlue(MatrizPixels[1+j][1+i]);
            pixelValueX += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
            pixelValueY += weight[j + 1][i + 1] * blue;  // En pixelValue se calcula el componente y del gradiente
          }
        }

      }

      int new_value = sqrt((pixelValueX*pixelValueX)+(pixelValueY*pixelValueY));
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
  
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

/*
###############################################################################
#   Nombre del metodo: main									                  #
#   Fecha de creacion: 24/10/18      										  #
#   Version: 1.0.0      													  #
#   Argumentos de entrada: argc, *argv										  #
#   Valor retornado: vacio                                   				  #
#   Descripcion: Realiza la ejecucion del programa, el calculo de los tiempos #
#   y la comparacion de las imagenes.										  #
###############################################################################
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    if (argc != 2) {printf("Vuelva a ejecutar. Uso: <ejecutable> <archivo imagen> \n"); return -1;} 
    QPixmap qp = QPixmap(argv[1]);
    if(qp.isNull()) { printf("no se ha encontrado la imagen\n"); return -1;}
	    
    QImage image = qp.toImage();
    QImage sobelImage(image);
    
    
    double computeTime = SobelBasico(&image, &sobelImage);
    printf("tiempo Sobel básico: %0.9f segundos\n", computeTime);
    // Prof.: falta SobelParallel

    QImage seqImage(image); // Prof.: ¡no uśais seq Image en la llamada a SobelLocal!
    computeTime = SobelLocal(&image, &sobelImage);
    printf("tiempo Sobel local: %0.9f segundos\n", computeTime);

    QImage auxImage(image);
    computeTime = SobelLocalParallel(&image, &sobelImage);
    printf("tiempo Sobel Local Parallel: %0.9f segundos\n", computeTime);
	// Prof.: el siguiente if no compara resultados de ninguna función
    if (auxImage == seqImage) printf("Algoritmo secuencial y paralelo dan la misma imagen\n");
	else printf("Algoritmo secuencial y paralelo dan distinta imagen\n");    

	seqImage = image;
    computeTime = SobelCompleto(&image, &sobelImage);
    printf("tiempo Sobel Completo: %0.9f segundos\n", computeTime);

    auxImage = image;	
    computeTime = SobelCompletoParallel(&image, &sobelImage);
    printf("tiempo Sobel Parallel Completo: %0.9f segundos\n", computeTime);

    if (auxImage == seqImage) printf("Algoritmo secuencial y paralelo dan la misma imagen\n");
	else printf("Algoritmo secuencial y paralelo dan distinta imagen\n");    

    QPixmap pixmap = pixmap.fromImage(sobelImage);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);
	// Prof.: solo se muestra una imagen y no es correcta
	// Prof.: no hay comentarios de resultados
    view.show();
    return a.exec();
}
