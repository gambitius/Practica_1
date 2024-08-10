**Práctica 1**

En este repositorio se encuentran los códigos utilizados para la realización de mediciones y cálculos relacionados a la experiencia realizada en el 
instituto milenio SAPHIR el año 2024, el cual correspondió al análisis de eficiencia de un tubo fotomultiplicador (PMT) al utilizar dos configuraciones para el mismo modelo.

Las mediciones realizadas para cada evento del PMT fueron adquiridas por un osciloscopio en un archivo formato ".csv". 
Dichos archivos contienen un aproximado de 6000 puntos para cada evento, por lo que mediante la implementación del código **csvRead.cpp**, es posible realizar una visualización de todos los eventos capturados.
Luego de implementar este código, fue requerido utilizar **chargeHisto.cpp**, el cual permite seleccionar e integrar la región del peack de la señal y obtener propiedades tales como la media y la desviación estándar.

El código **gainandpe.py** permite visualizar la relación comparativa entre la ganancia y el número promedio de fotoelectrones para ambas configuraciones de PMT, lo cual será necesario para determinar cual configuración posee una mayor eficiencia en comparación al otro.
Mientras que el código **saphir.ipynb** permite visualizar la caracterización del LED y realizar una gráfica que permitirá comparar la ganancia obtenida con la esperada por el fabricante.
