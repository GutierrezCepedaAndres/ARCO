// Prof.: 1,9 de 2




//AUTORES DE LAS MODIFICACIONES DEL CODIGO

//ANDRÉS GUTIÉRREZ CEPEDA

//MARIO COBO GOMEZ





#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define COLOUR_DEPTH 4

double computeGraySequential(QImage *image) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    *rgbpixel = QColor(gray, gray, gray).rgba();
  }
  return omp_get_wtime() - start_time;  
}

double computeGrayParallel(QImage *image) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for        
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    *rgbpixel = QColor(gray, gray, gray).rgba();
  }
  return omp_get_wtime() - start_time;  
}

double computeGrayScanline(QImage *image) {
  double start_time = omp_get_wtime();
  int alto = image->height(); int ancho = image->width();
  int jj, gray; uchar* scan; QRgb* rgbpixel;  
  for (int ii = 0; ii < alto; ii++) {
    
    scan = image->scanLine(ii);
    for (jj = 0; jj < ancho; jj++) {
      
      rgbpixel = reinterpret_cast<QRgb*>(scan + jj * COLOUR_DEPTH);
      gray = qGray(*rgbpixel);
      *rgbpixel = QColor(gray, gray, gray).rgba();
    }
  }
  return omp_get_wtime() - start_time;    
}

double computeGrayScanlineParallel(QImage *image) {
  double start_time = omp_get_wtime();
  int alto = image->height(); int ancho = image->width();
  int jj, gray; uchar* scan; QRgb* rgbpixel; 

  #pragma omp parallel for private(scan,gray,jj,rgbpixel) // estas variables se declaran como variables privadas dado que ninguna de ellas esta declarada
  														  // en el interior del bucle paralelizado y son modificadas por las distintas hebras en ejecucion
  														  // lo cual puede generar un conflicto de modificacion de datos y no obtenerse el resultado deseado en la imagen
  for (int ii = 0; ii < alto; ii++) { // a su vez se puede paralelizar el segundo bucle for pero en este caso no se pide, por lo cual solo hemos paralelizado el bucle externo
    
    #pragma omp critical //esta es una seccion critica dado que no pueden acceder a la imagen varias hebras a la vez, la funcion atomic no puede usarse dado que no es una operación como tal
    scan = image->scanLine(ii);

    for (jj = 0; jj < ancho; jj++) {
      
      rgbpixel = reinterpret_cast<QRgb*>(scan + jj * COLOUR_DEPTH);
      gray = qGray(*rgbpixel);
      *rgbpixel = QColor(gray, gray, gray).rgba();
    }
  }
  return omp_get_wtime() - start_time;    
}

int main(int argc, char *argv[])
{
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
    
    QImage seqImage(image);
    double computeTime = computeGraySequential(&seqImage);
    printf("sequential time: %0.9f seconds\n", computeTime);

    QImage auxImage(image);
    computeTime = computeGrayParallel(&auxImage);
    printf("parallel time: %0.9f seconds\n", computeTime);
    
	if (auxImage == seqImage) printf("Algoritmo secuencial y paralelo dan la misma imagen\n");
	else printf("Algoritmo secuencial y paralelo dan distinta imagen\n");    

    seqImage = image;
    computeTime = computeGrayScanline(&seqImage);
    printf("scanline sequential time: %0.9f seconds\n", computeTime);
    
    // Prof.: falta la comparación de imágenes

    auxImage = image;														//Se realiza una llamada a los dos metodos recogiendo la comparativa de las imagenes y de los tiempos
    computeTime = computeGrayScanlineParallel(&auxImage);
    printf("scanline parallel time: %0.9f seconds\n", computeTime);
    
	if (auxImage == seqImage) printf("Algoritmo básico y 'scanline' dan la misma imagen\n");
	else printf("Algoritmo básico y 'scanline' dan distinta imagen\n");    
        
    QPixmap pixmap = pixmap.fromImage(auxImage);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);
           
    view.show();
    return a.exec();
}
