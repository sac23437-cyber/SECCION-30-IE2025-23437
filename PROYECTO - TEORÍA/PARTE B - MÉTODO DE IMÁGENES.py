import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def metodo_imagenes(angulo_grados, carga_posicion=(1, 0), carga_valor=1.0):
    """
    Grafica las cargas originales y sus imágenes según el método de imágenes.
    
    Parámetros:
    - angulo_grados: Ángulo φ en grados para el cálculo de N
    - carga_posicion: Tupla (x, y) con la posición de la carga original
    - carga_valor: Valor de la carga original
    """
    
    # Calcular N según la fórmula dada
    phi = angulo_grados
    N = (360 / phi) - 1
    
    # Verificar si N es entero
    if not N.is_integer():
        print(f"Advertencia: Para φ={phi}°, N={N:.2f} no es entero. Se usará N={int(np.round(N))}")
    N = int(np.round(N))
    
    print(f"Para un ángulo de {phi}°, se necesitan {N} cargas imagen")
    
    # Configurar la figura
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_aspect('equal')
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.grid(True)
    ax.axhline(0, color='black', linewidth=0.5)
    ax.axvline(0, color='black', linewidth=0.5)
    ax.set_title(f"Método de Imágenes: φ={phi}°, N={N}", pad=20)
    
    # Dibujar las placas conductoras (líneas en el ángulo dado)
    angulo_rad = np.deg2rad(phi)
    x_end = 2 * np.cos(angulo_rad)
    y_end = 2 * np.sin(angulo_rad)
    ax.plot([0, x_end], [0, y_end], 'k-', linewidth=2, label='Placa conductora')
    ax.plot([0, 0], [0, 0], 'k-', linewidth=2)  # Eje x como otra placa
    
    # Dibujar la carga original
    ax.plot(carga_posicion[0], carga_posicion[1], 'ro', markersize=10)
    ax.text(carga_posicion[0], carga_posicion[1] + 0.1, 
            f'q={carga_valor}', ha='center', va='bottom', color='red')
    
    # Calcular y dibujar las cargas imagen
    for i in range(1, N + 1):
        angulo_imagen = i * phi
        # Rotar la posición original por el ángulo de imagen
        rot_matrix = np.array([
            [np.cos(np.deg2rad(angulo_imagen)), -np.sin(np.deg2rad(angulo_imagen))], 
            [np.sin(np.deg2rad(angulo_imagen)), np.cos(np.deg2rad(angulo_imagen))]
        ])
        pos_imagen = rot_matrix @ np.array(carga_posicion)
        
        # Alternar el signo de la carga imagen
        carga_imagen = carga_valor * (-1)**i
        
        color = 'blue' if carga_imagen > 0 else 'green'
        ax.plot(pos_imagen[0], pos_imagen[1], 'o', color=color, markersize=10)
        ax.text(pos_imagen[0], pos_imagen[1] + 0.1, 
                f'q\'={carga_imagen}', ha='center', va='bottom', color=color)
    
    # Añadir leyenda y mostrar
    ax.legend()
    plt.tight_layout()
    plt.show()

# Solicitar el ángulo por consola
while True:
    try:
        angulo = float(input("Ingrese el ángulo en grados (entre 0 y 180): "))
        if 0 < angulo < 180:
            break
        else:
            print("Por favor ingrese un valor entre 0 y 180 grados.")
    except ValueError:
        print("Entrada inválida. Por favor ingrese un número.")

# Llamar a la función con el ángulo ingresado
metodo_imagenes(angulo)