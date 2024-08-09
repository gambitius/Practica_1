import numpy as np
import matplotlib.pyplot as plt


#gain
#800V 20mV
g1 = [953721.3308,1084527.613]
#800V 100mV
g2 = [930126.3538,1034409.722]
#1000V 20mV
g3 = [4203819.123,4537715.825]
#1000V 100mV 
g4 = [4053920.125,4426229.508]
#1200V 20mV 
g5 = [1156531.688,0]
#1200V 100mV
g6 = [12808031.33,13064846.04]

#error gain
#800V 20mV
eg1 = [993243/2,1161123/2]
#800V 100mV
eg2 =[967026/2,1078132/2]
#1000V 20mV
eg3 = [4482581/2,4858381/2]
#1000V 100mV
eg4 = [4260330/2,4701236/2]
#1200V 20mV
eg5 = [1194602/2,0]
#1200V 100mV
eg6 = [13473486/2,13814424/2]

#pe
#800V 20mV
p1 = [29.20140202,25.2529301]
#800V 100mV
p2 = [148.9045004,134.8595213]
#1000V 20mV
p3 = [6.425704629,4.826216723]
#1000V 100mV
p4 = [31.94438864,25.84027778]
#1200V 20mV
p5 = [9.635490418,0]
#1200V 100mV
p6 = [9.735102671,8.156429832]

#error pe
#800V 20mV 
ep1 = [0.987839072,1.042398689]
#800V 100mV
ep2 = [3.960910033,3.79486892]
#1000V 20mV
ep3 = [0.223543291,0.195261374]
#1000V 100mV
ep4 = [1.017269933,1.034557394]
#1200V 20mV
ep5 = [0.372678532,0]
#1200V 100mV
ep6 = [0.325478697,0.270717453]

# Datos
ganancias = g5  # Ponemos la ganancia en una lista
errores_ganancia = eg5  # Ponemos el error de la ganancia en una lista
pe = p5
epe = ep5

# Índices del eje x (1 y 2 en este caso)
x_values = [1,2]

# Graficar
plt.errorbar(x_values[:len(ganancias)], ganancias, yerr=errores_ganancia, fmt='o', capsize=5, label='Ganancia con error')

# Ajustar límites de los ejes para que los puntos estén más cerca del centro
plt.xlim(0.5, 2.5)
plt.ylim(min(ganancias) - max(errores_ganancia), max(ganancias) + max(errores_ganancia))

#Etiquetas y título
plt.xlabel('Configuración de PMT')
plt.ylabel('Ganancia')
plt.title('Ganancia para 1200V con un peack de 100mV')

plt.xticks(x_values)  # Asegura que las etiquetas del eje x sean 1 y 2
plt.legend()
plt.grid(True)  # Añadir una cuadrícula para mayor claridad
plt.show()

# Graficar2
plt.errorbar(x_values[:len(pe)], pe, yerr=epe, fmt='o', capsize=5, label='<PE> con error')

plt.xlim(0.5, 2.5)
plt.ylim(min(pe) - max(epe), max(pe) + max(epe))

# Etiquetas y título
plt.xlabel('Configuración de PMT')
plt.ylabel('Cantidad Fotoelectrones')
plt.title('Cantidad de Fotoelectrons para 1200V con un peack de 20mV')
plt.xticks(x_values)  # Asegura que las etiquetas del eje x sean 1 y 2
plt.legend()
plt.grid(True)  # Añadir una cuadrícula para mayor claridad
plt.show()