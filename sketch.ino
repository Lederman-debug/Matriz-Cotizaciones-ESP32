#include <MD_MAX72xx.h>
#include <string.h>

// ====================================================
// CONFIGURACIÓN DE HARDWARE
// ====================================================
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW  
#define DATA_PIN   25
#define CLK_PIN    27
#define MAX_DEVICES 4   

#define CS1 26   
#define CS2 14   
#define CS3 32   
#define CS4 33   
#define CS5 15   
#define CS6 2    

MD_MAX72XX mx1 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS1, MAX_DEVICES);
MD_MAX72XX mx2 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS2, MAX_DEVICES);
MD_MAX72XX mx3 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS3, MAX_DEVICES);
MD_MAX72XX mx4 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS4, MAX_DEVICES);
MD_MAX72XX mx5 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS5, MAX_DEVICES);
MD_MAX72XX mx6 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS6, MAX_DEVICES);

#define INVERTIR_FILAS    false   
#define INVERTIR_COLUMNAS true    

#define FILAS_MATRIZ 8
#define COLS_MATRIZ 32
#define FILAS_MATRICES 3
#define COLS_MATRICES 2

MD_MAX72XX* matrices[FILAS_MATRICES][COLS_MATRICES] = {
  { &mx1, &mx2 },   
  { &mx3, &mx4 },   
  { &mx5, &mx6 }    
};

// ====================================================
// FUNCIONES DE CONTROL DE PÍXELES
// ====================================================

// Limpia todas las matrices del panel
void limpiarPantalla() {
  for (int f = 0; f < FILAS_MATRICES; f++) {
    for (int c = 0; c < COLS_MATRICES; c++) {
      matrices[f][c]->clear();
    }
  }
}

void setPuntoGlobal(int fila_global, int col_global, bool estado) {
  if (fila_global < 0 || fila_global >= 24 || col_global < 0 || col_global >= 64) return;
  
  int fila_matriz = fila_global / FILAS_MATRIZ;
  int col_matriz = col_global / COLS_MATRIZ;
  int fila_local = fila_global % FILAS_MATRIZ;
  int col_local = col_global % COLS_MATRIZ;

  int fila_real = INVERTIR_FILAS ? (7 - fila_local) : fila_local;
  int col_real = INVERTIR_COLUMNAS ? (31 - col_local) : col_local;

  matrices[fila_matriz][col_matriz]->setPoint(fila_real, col_real, estado);
}

// Dibuja texto respetando un margen izquierdo en LEDs y verificando límites
void dibujarTextoConMargen(int fila_inicio, int margen_leds, const char* texto, int spacing) {
  int x_actual = margen_leds; // Columna global actual
  
  // Usamos mx1 como referencia para obtener la fuente (todas las matrices comparten la misma por defecto)
  MD_MAX72XX &fuente = mx1;
  
  while (*texto && x_actual < 64) {
    uint8_t charBits[5];
    uint8_t ancho = fuente.getChar(*texto++, sizeof(charBits), charBits);

    // Limitar el dibujo al ancho del panel
    for (int col = 0; col < ancho && (x_actual + col) < 64; col++) {
      for (int fila = 0; fila < 7; fila++) {
        if (charBits[col] & (1 << fila)) {
          setPuntoGlobal(fila_inicio + fila, x_actual + col, true);
        }
      }
    }
    x_actual += ancho + spacing;
  }
}

// Dibuja texto centrado en una matriz individual (ej: mx3, mx4, etc.)
void dibujarTextoCentrado(MD_MAX72XX &mx, const char* texto) {
  int longitud = strlen(texto);
  int anchoTotal = 0;
  for (int i = 0; i < longitud; i++) {
    uint8_t temp[5];
    anchoTotal += mx.getChar(texto[i], sizeof(temp), temp) + 1;
  }
  anchoTotal--; // Quitar el último espacio extra

  int colInicio = (COLS_MATRIZ - anchoTotal) / 2;
  int x_ptr = colInicio;

  mx.clear();
  for (int i = 0; i < longitud; i++) {
    uint8_t charBits[5];
    uint8_t ancho = mx.getChar(texto[i], sizeof(charBits), charBits);
    for (int col = 0; col < ancho; col++) {
      for (int fila = 0; fila < 7; fila++) {
        if (charBits[col] & (1 << fila)) {
          int filaReal = INVERTIR_FILAS ? 7 - fila : fila;
          int colReal = INVERTIR_COLUMNAS ? (31 - (x_ptr + col)) : (x_ptr + col);
          mx.setPoint(filaReal, colReal, true);
        }
      }
    }
    x_ptr += ancho + 1;
  }
}

// ====================================================
// SETUP
// ====================================================

void setup() {
  // Inicializar cada matriz
  mx1.begin(); mx2.begin(); mx3.begin(); mx4.begin(); mx5.begin(); mx6.begin();

  // Configurar intensidad y limpiar todo el panel
  for (int f = 0; f < FILAS_MATRICES; f++) {
    for (int c = 0; c < COLS_MATRICES; c++) {
      for (int d = 0; d < MAX_DEVICES; d++) {
        matrices[f][c]->control(d, MD_MAX72XX::INTENSITY, 5);
      }
    }
  }
  limpiarPantalla();

  // EJEMPLO: "TASA DEL DIA" empezando exactamente a 2 LEDs de la esquina izquierda
  // Parámetros: (Fila inicial, Margen LEDs izquierda, Texto, Espacio entre letras)
  dibujarTextoConMargen(0, 0, "TASA DEL DIA", 1);

  // Textos centrados en las matrices inferiores
  dibujarTextoCentrado(mx3, "DOLAR");
  dibujarTextoCentrado(mx4, "300BS");
  dibujarTextoCentrado(mx5, "EURO");
  dibujarTextoCentrado(mx6, "600BS");
}

void loop() { }