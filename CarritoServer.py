import cv2
import os
import requests
import cv2
import time
from ultralytics import YOLO

# IP de la ESP32 CAM
# ESP - 2A5B60
ip = "http://192.168.185.110"
# ip = "http://192.168.0.100"

# IP de la ESP32 CAM
# ESP - 593DBC
stream_url = 'http://192.168.185.14'

# Seteamos la calidad de la camara
response = requests.get(stream_url + "/control?var=framesize&val=11")

# Cargamos los modelos de YOLO para tangram en vivo
model_en_vivo = YOLO("./yolo_recognizer/only_real.pt")

# Cargamos los modelos de YOLO para tangram pintado en el piso
model_en_piso = YOLO("./yolo_recognizer/only_images_v4.pt")

# Directorio donde se guardarán las imágenes
output_directory = 'uploads/'

# Verificamos de que el directorio de salida exista, si no, lo creamos
if not os.path.exists(output_directory):
    os.makedirs(output_directory)

# Función para posicionar el brazo en la posición inicial
def posicion_inicial():
    enviar_comando_brazo("s090")
    time.sleep(1)
    enviar_comando_brazo("s1100")
    time.sleep(1)
    enviar_comando_brazo("s2140")
    time.sleep(1)
    enviar_comando_brazo("s3110")
    time.sleep(1)
    enviar_comando_brazo("s480")
    time.sleep(1)
    enviar_comando_brazo("s5130")
    time.sleep(3)


# Función para recolectar las figura en base a la base
# que se envie desde parametro
def tomar_figuras(results):

    # Bodega a la que se llevaran las figuras
    base = results["bodega"]

    # Patron que se reconocio
    patron = results["patron"]

    # Posiciones de las figuras en las base 1
    base1 = [
        ["s5135","s076","s2130","s122"], # Triangulo naranja
        ["s380","s080","s0100","s2128","s121"], # Triangulo rosa
        ["s070","s092","s5150","s2134","s124"], # Trapecio azul
        ["s3110","s5150","s070","s086","s2142","s132"], # Triangulo rojo
        ["s0110","s2143","s132"], # Triangulo verde
        ["s3140","s075","s086","s2152","s140"], # Triangulo morado
        ["s3110","s5140","s080","s097","s2148","s136"] # Cuadrado amarillo
        ]
    
    # Posiciones de las figuras en las base 2
    base2 = [
        ["s5135","s073","s2146","s134"], # Triangulo naranja
        ["s0110","s2147","s135"], # Triangulo verde
        ["s5130","s070","s087","s2134","s128"], # Cuadrado amarillo
        ["s5135","s088","s096","s2130","s124"], # Triangulo morado
        ["s088","s099","s2142","s131"], # Triangulo rojo
        ["s5140","s380","s080","s093","s2152","s138"], # Trapecio azul
        ["s3110","s5135","s075","s084","s2152","s138"], # Triangulo rosa
        ]
    
    # Posiciones de las figuras en las base 3
    base3 = [
        ["s5145","s070","s080","s2132","s122"], # Triangulo verde
        ["s070","s084","s3130","s2149","s138"], # Triangulo naraja
        ["s5145","s070","s097","s2150","s140"], # Cuadrado amarillo
        ["s5130","s070","s3110","s095","s3110","s2136","s127"], # Triangulo marado
        ["s5145","s070","s3140","s0102","s2128","s124"], # Triangulo rojo
        ["s5145","s070","s3110","s0109","s2137","s127"], # Trapecio azul
        ["s5145","s070","s0111","s2150","s140"], # Triangulo rosa
        ]
    
    # Posiciones de las figuras en las base 4
    base4 = [
        ["s5119","s2148","s090","s0116","s136"], # Triangulo morado
        ["s5130","s080","s096","s2132","s124"], # Trapecio azul
        ["s070","s077","s2146","s133"], # Triangulo verde
        ["s070","s088","s2133","s125"], # Triangulo naranja
        ["s080","s0104","s2134","s126"], # Triangulo rosa
        ["s080","s096","s2153","s140"], # Triangulo rojo
        ["s070","s0107","s2143","s132"], # Cuadrado amarillo
        ]

    # Creamos un array con posiciones de las figuras de cada base
    bases = [base1, base2, base3, base4]

    # Recorremos el array de posiciones de la base seleccionada
    for figure in bases[patron - 1]:
        # Posicionamos para agarra la figura
        enviar_comando_brazo("s5130")

        # Movemos el brazo a la posicion de la figura
        for command in figure:
            enviar_comando_brazo(command)
            time.sleep(0.5)

        # Tomamos la figura
        enviar_comando_brazo("s580")
        time.sleep(2)

        # Subimos el brazo
        enviar_comando_brazo("s195")

        # Avanzamos hacia la base
        for i in range(base):
            enviar_comando_seguidor("adelante")
            time.sleep(1)

        # Bajamos la figura
        enviar_comando_brazo(figure[len(figure)-1])
        enviar_comando_brazo("s5130")

        # Subimos el brazo
        enviar_comando_brazo("s195")
        time.sleep(3)

        # Volvemos al inicio
        volver_desde_base(base)
    
    # Posicionamos el brazo en la posicion inicial
    posicion_inicial()

# Función para volver al punto inicial desde "X" base
def volver_desde_base(base):
    for i in range(base):
        enviar_comando_seguidor("atras")

# Función para recuperar el label o etiqueta obtenido
# por algun modelo de YOLO
def obtener_label(image, model):
    resultados = model.predict(image, imgsz=640)
    names = model.names

    for r in resultados:
        for c in r.boxes.cls:
            return names[int(c)]
    
    return

# Función para obtener que patron se reconocio
# en base a las fotos tomados por la camara
def reconocer_patron():

    # Carga las imágenes de referencia de los patrones
    pattern_images = [
        cv2.imread('./uploads/b1.jpg', cv2.IMREAD_COLOR),
        cv2.imread('./uploads/b2.jpg', cv2.IMREAD_COLOR),
        cv2.imread('./uploads/b3.jpg', cv2.IMREAD_COLOR),
        cv2.imread('./uploads/b4.jpg', cv2.IMREAD_COLOR),
    ]

    # Carga la imagen en la que deseas realizar la comparación
    image_to_compare = cv2.imread('./uploads/bi.jpg', cv2.IMREAD_COLOR)

    # Inicializa el index de la bodega y el patron
    bodega_index = 0
    patron_index = 0

    # Obtiene el label o etiqueda de la imagen inicial (patrón del tangram al inicio)
    b1_label = obtener_label(image_to_compare, model_en_vivo)

    # Array para guardar los resultados de los patrones encontrados en las bases
    pattern_results = []

    # Array con las opciones de las bases
    options = ["Base1", "Base2", "Base3", "Base4"]

    # Obtener el label de las fotos de las bases
    for pattern in pattern_images:
        label = obtener_label(pattern, model_en_piso)
        pattern_results.append(label)

    # Comparamos el label de las bases con el label de la imagen inicial
    # para obtener el numero de la bodega
    for i, label in enumerate(pattern_results):
        if label == b1_label:
            bodega_index = i
            pass

    # Comparamos el label de la imagen inicial con las opciones de las bases
    # para obtener el patron
    for i, option in enumerate(options):
        if b1_label == option:
            patron_index = i

    # Creamos un diccionario con los resultados
    results = {
        "bodega": bodega_index + 1,
        "patron": patron_index + 1
    }

    return results

# Función para tomar una foto con la ESP32 CAM y guardarla 
# con el nombre enviado desde parametro
def tomar_foto(name):
    time.sleep(1)

    # Inicia la captura de video desde la transmisión de la ESPCam
    cap = cv2.VideoCapture(stream_url + ":81/stream")

    if not cap.isOpened():
        print("Error al abrir la transmisión de video.")
    else:
        # Captura un fotograma del flujo de video
        ret, frame = cap.read()

        if not ret:
            print("Error al capturar el fotograma.")
        else:
            # Genera un nombre de archivo único basado en el tiempo
            file_name = name + '.jpg'

            # Ruta completa para guardar la imagen
            output_path = os.path.join(output_directory, file_name)

            # Guarda la imagen
            cv2.imwrite(output_path, frame)
            print('Imagen guardada en:', output_path)

        # Libera los recursos
        cap.release()

# Función para tomar las fotos de las bases
def rutina_fotos():
    time_to_foto = 1
    time_to_go = 1
    
    # Tomar foto de la bodega inicial
    time.sleep(time_to_foto)
    tomar_foto("bi")
    time.sleep(time_to_go)

    # Ir a la bodega 1
    enviar_comando_seguidor("adelante")

    # Tomar foto de la bodega 1
    time.sleep(time_to_foto)
    tomar_foto("b1")
    time.sleep(time_to_go)

    # Ir a la bodega 2
    enviar_comando_seguidor("adelante")

    # Tomar foto de la bodega 2
    time.sleep(time_to_foto)
    tomar_foto("b2")
    time.sleep(time_to_go)

    # Ir a la bodega 3
    enviar_comando_seguidor("adelante")

    # Tomar foto de la bodega 3
    time.sleep(time_to_foto)
    tomar_foto("b3")
    time.sleep(time_to_go)

    # Ir a la bodega 4
    enviar_comando_seguidor("adelante")

    # Tomar foto de la bodega 4
    time.sleep(time_to_foto)
    tomar_foto("b4")
    time.sleep(time_to_go)

# Función que realiza la rutina completa de reconocimiento y rec
def rutina_bodegas():

    # Tomar fotos de las bases
    rutina_fotos()

    # Obtener el patron inicial y su ubicación en la base
    patron = reconocer_patron()

    # Dar resultado
    print("La bodega seleccionada es: ")
    print(patron["bodega"])

    #Volver al inicio
    volver_desde_base(4)

    # Tomar las figuras y trasladarlas a la base
    tomar_figuras(patron)

# Función para enviar comandos al seguidor de linea
def enviar_comando_seguidor(comando):
    url = f"{ip}/seguidor?mov={comando}"
    response = requests.get(url).text
    print(response)
    print(f"Enviando comando carro: {comando}")
    return response

# Función para enviar comandos al brazo robotico
def enviar_comando_brazo(comando):
    # Se asume que el comando para el brazo es de la forma "snxxx"
    servo_num = int(comando[1])
    grados = int(comando[2:])
    url = f"{ip}/brazo?servo={servo_num}&grados={grados}"
    response = requests.get(url).text
    print(response)
    print(f"Enviando comando brazo: {comando}")
    time.sleep(0.5)

# Función principal
def main():
    # Mostar menú de comandos
    while True:
        comando = input("Ingrese un comando ('q' para salir): ")

        if comando == 'q':
            break
        # Ejecutar rutina de bodegas
        if comando.startswith('bodega'):
            rutina_bodegas()
        # Ejecutatar movimiento de seguidor de linea hacia adelante
        elif comando.startswith('sade'):
            enviar_comando_seguidor('adelante')
        # Ejecutatar movimiento de seguidor de linea hacia atras
        elif comando.startswith('sdet'):
            enviar_comando_seguidor('atras')
        else:
            print("Comando no válido. Intente nuevamente.")

if __name__ == "__main__":
    main()
