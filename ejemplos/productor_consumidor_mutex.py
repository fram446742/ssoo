import threading
import time
import random

# Tamaño del buffer
BUFFER_SIZE = 5
buffer = []  # Buffer compartido
mutex = threading.Lock()  # Mutex para controlar el acceso al buffer
full = threading.Condition(mutex)  # Condición para manejar el buffer lleno
empty = threading.Condition(mutex)  # Condición para manejar el buffer vacío

def productor(id):
    while True:
        item = random.randint(1, 100)  # Generar un número aleatorio
        with empty:  # Esperar si el buffer está lleno
            while len(buffer) == BUFFER_SIZE:
                empty.wait()
            buffer.append(item)  # Colocar el item en el buffer
            print(f"Productor {id} produjo: {item}")
            full.notify()  # Notificar a los consumidores que hay un nuevo item
        time.sleep(random.uniform(0.1, 1))  # Esperar un tiempo aleatorio

def consumidor(id):
    while True:
        with full:  # Esperar si el buffer está vacío
            while len(buffer) == 0:
                full.wait()
            item = buffer.pop(0)  # Tomar un item del buffer
            print(f"Consumidor {id} consumió: {item}")
            empty.notify()  # Notificar a los productores que hay espacio en el buffer
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

# Esperar a que los hilos terminen (esto es solo para evitar que el script se detenga)
for hilo in productores:
    hilo.join()

for hilo in consumidores:
    hilo.join()
