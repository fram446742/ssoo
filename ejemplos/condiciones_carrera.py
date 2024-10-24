import threading
import time

# Recurso compartido
recurso_compartido = 0

def incrementar():
    global recurso_compartido
    for _ in range(100000):
        recurso_compartido += 1

# Crear varios hilos que incrementan el recurso
hilos = []
for _ in range(2):
    hilo = threading.Thread(target=incrementar)
    hilos.append(hilo)
    hilo.start()

# Esperar a que todos los hilos terminen
for hilo in hilos:
    hilo.join()

print("Valor final del recurso compartido (condición de carrera):", recurso_compartido)
