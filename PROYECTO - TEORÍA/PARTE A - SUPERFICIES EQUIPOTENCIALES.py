import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D

def calcular_potencial(x, y, a, b, V1, V2, n_terminos=5):
    V = 0
    for k in range(n_terminos):
        n = 2*k + 1 # usa los términos impares por simetría del problema 
        denominador = n * np.sinh(n * np.pi * a / b)
        seno = np.sin(n * np.pi * x / b)
        term_V1 = V1 * np.sinh(n * np.pi * (a - y) / b)
        term_V2 = V2 * np.sinh(n * np.pi * y / b)
        V += (4/np.pi) * (seno / denominador) * (term_V1 + term_V2)
    return V

def mostrar_graficas_completas(a, b, V1, V2, escenario_num):
    # Configuración común
    plt.close('all')
    
    # Crear malla de puntos (CORRECCIÓN: transpuesta de la malla)
    x = np.arange(0, a + 0.1, 0.1)
    y = np.arange(0, b + 0.1, 0.1)
    X, Y = np.meshgrid(x, y)
    
    # Calcular potencial (CORRECCIÓN: dimensión correcta)
    V = np.zeros_like(X)
    for i in range(X.shape[0]):
        for j in range(X.shape[1]):
            V[i,j] = calcular_potencial(X[i,j], Y[i,j], a, b, V1, V2)
    
    # --- Gráfico 2D ---
    plt.figure(1, figsize=(10, 8))
    levels = np.linspace(0, max(V1, V2), 20)
    CS = plt.contour(X, Y, V, levels=levels, colors='black', linewidths=0.8)
    plt.clabel(CS, inline=True, fontsize=8)
    plt.contourf(X, Y, V, levels=levels, cmap='viridis', alpha=0.7)
    plt.colorbar(label='Potencial (V)')
    plt.title(f'Escenario {escenario_num}: Equipotenciales\n{a}m × {b}m | V1={V1}V, V2={V2}V')
    plt.xlabel('x (m)')
    plt.ylabel('y (m)')
    plt.grid(True)
    plt.show(block=False)
    
    # --- Gráfico 3D ---
    plt.figure(2, figsize=(12, 8))
    ax = plt.axes(projection='3d')
    surf = ax.plot_surface(X, Y, V, cmap='viridis', edgecolor='none')
    ax.set_title(f'Escenario {escenario_num}: Potencial 3D\n{a}m × {b}m | V1={V1}V, V2={V2}V')
    ax.set_xlabel('x (m)')
    ax.set_ylabel('y (m)')
    ax.set_zlabel('Potencial (V)')
    plt.colorbar(surf, ax=ax, label='Potencial (V)')
    plt.show(block=False)
    
    # --- Tabla de Valores ---
    puntos_x = np.linspace(0, a, 5)
    puntos_y = np.linspace(0, b, 4)
    valores = np.zeros((len(puntos_y), len(puntos_x)))
    
    for i, yi in enumerate(puntos_y):
        for j, xi in enumerate(puntos_x):
            valores[i,j] = calcular_potencial(xi, yi, a, b, V1, V2)
    
    plt.figure(3, figsize=(10, 6))
    ax = plt.gca()
    ax.axis('off')
    
    # Crear tabla
    cell_text = [[f"{val:.1f}" for val in row] for row in valores]
    tabla = plt.table(cellText=cell_text,
                     rowLabels=[f"{yi:.1f}" for yi in puntos_y],
                     colLabels=[f"{xi:.1f}" for xi in puntos_x],
                     loc='center',
                     cellLoc='center')
    
    # Formato tabla
    tabla.auto_set_font_size(False)
    tabla.set_fontsize(12)
    tabla.scale(1, 2)
    
    # Color de celdas
    for i in range(len(puntos_y)):
        for j in range(len(puntos_x)):
            color = plt.cm.viridis(valores[i,j]/max(V1, V2))
            tabla[(i+1,j)].set_facecolor(color)
            tabla[(i+1,j)].set_alpha(0.6)
    
    plt.title(f'Escenario {escenario_num}: Valores de Potencial\n{a}m × {b}m | V1={V1}V, V2={V2}V', pad=30)
    plt.tight_layout()
    plt.show(block=False)
    
    input("\nPresiona Enter para continuar al siguiente escenario...")
    plt.close('all')

# Escenarios principales
escenarios = [
    {'a': 1, 'b': 1, 'V1': 100, 'V2': 50},
    {'a': 1, 'b': 2, 'V1': 100, 'V2': 50},
    {'a': 1, 'b': 3, 'V1': 100, 'V2': 50},
    {'a': 4, 'b': 1, 'V1': 100, 'V2': 50}
]

# Mostrar todos los escenarios
for i, esc in enumerate(escenarios, 1):
    print(f"\n{'='*60}")
    print(f"PROCESANDO ESCENARIO {i}: a={esc['a']}m, b={esc['b']}m")
    print(f"{'='*60}")
    mostrar_graficas_completas(esc['a'], esc['b'], esc['V1'], esc['V2'], i)

print("\n¡Todos los escenarios han sido procesados!")