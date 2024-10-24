import threading
import time

# Crear un semáforo con un valor inicial de 2
semaforo = threading.Semaphore(2)

# Recurso compartido
recurso_compartido = 0

def acceder_recurso(thread_id):
    global recurso_compartido
    
    # Intentar adquirir el semáforo
    with semaforo:
        print(f"Hilo {thread_id} ha adquirido el semáforo.")
        # Simular trabajo con el recurso compartido
        temp = recurso_compartido
        time.sleep(1)  # Simulando trabajo
        recurso_compartido = temp + 1
        print(f"Hilo {thread_id} ha incrementado el recurso compartido a {recurso_compartido}.")

# Crear varios hilos
hilos = []
for i in range(5):
    hilo = threading.Thread(target=acceder_recurso, args=(i,))
    hilos.append(hilo)
    hilo.start()

# Esperar a que todos los hilos terminen
for hilo in hilos:
    hilo.join()

print("Acceso final al recurso compartido:", recurso_compartido)
