import threading
import queue
import time
import random

# Crear una cola con un tamaño máximo
buffer = queue.Queue(maxsize=5)

def productor(id):
    while True:
        item = random.randint(1, 100)  # Generar un número aleatorio
        buffer.put(item)  # Colocar el item en el buffer
        print(f"Productor {id} produjo: {item}")
        time.sleep(random.uniform(0.1, 1))  # Esperar un tiempo aleatorio

def consumidor(id):
    while True:
        item = buffer.get()  # Tomar un item del buffer
        print(f"Consumidor {id} consumió: {item}")
        buffer.task_done()  # Indicar que el item ha sido procesado
        time.sleep(random.uniform(0.1, 1))  # Esperar un tiempo aleatorio

# Crear hilos de productores y consumidores
num_productores = 2
num_consumidores = 2

productores = []
consumidores = []

for i in range(num_productores):
    hilo_productor = threading.Thread(target=productor, args=(i,))
    productores.append(hilo_productor)
    hilo_productor.start()

for i in range(num_consumidores):
    hilo_consumidor = threading.Thread(target=consumidor, args=(i,))
    consumidores.append(hilo_consumidor)
    hilo_consumidor.start()

# Esperar a que los productores terminen (esto en un caso real no es necesario, pero aquí es para evitar que el script se detenga)
for hilo in productores:
    hilo.join()

for hilo in consumidores:
    hilo.join()
