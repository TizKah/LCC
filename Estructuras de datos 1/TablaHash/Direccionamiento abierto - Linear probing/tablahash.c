#include "tablahash.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/**
 Casillas en la que almacenaremos los datos de la tabla hash.
   */
typedef struct {
  void *dato;
  int eliminado;
} CasillaHash;

/**
 Estructura principal que representa la tabla hash.
 */
struct _TablaHash {
  CasillaHash *elems;
  unsigned numElems;
  unsigned capacidad;
  FuncionCopiadora copia;
  FuncionComparadora comp;
  FuncionDestructora destr;
  FuncionHash hash;
};

/**
 Crea una nueva tabla hash vacia, con la capacidad dada.
 */
TablaHash tablahash_crear(unsigned capacidad, FuncionCopiadora copia,
                          FuncionComparadora comp, FuncionDestructora destr,
                          FuncionHash hash) {

  // Pedimos memoria para la estructura principal y las casillas.
  TablaHash tabla = malloc(sizeof(struct _TablaHash));
  assert(tabla != NULL);
  tabla->elems = malloc(sizeof(CasillaHash) * capacidad);
  assert(tabla->elems != NULL);
  tabla->numElems = 0;
  tabla->capacidad = capacidad;
  tabla->copia = copia;
  tabla->comp = comp;
  tabla->destr = destr;
  tabla->hash = hash;

  // Inicializamos las casillas con datos nulos.
  for (unsigned idx = 0; idx < capacidad; ++idx) {
    tabla->elems[idx].dato = NULL;
    tabla->elems[idx].eliminado = 0;
  }

  return tabla;
}


void tablahash_resize(TablaHash tabla){
  unsigned int nueva_capacidad = tabla->capacidad*2;
  // Creamos una var auxiliar para guardar el array de elementos
  CasillaHash *old_array = tabla->elems;

  // 'Reseteamos' el array de elementos, con el doble de capacidad anterior
  tabla->capacidad = nueva_capacidad;
  tabla->numElems = 0;
  tabla->elems = malloc(sizeof(CasillaHash) * tabla->capacidad);

  // Inicializamos el nuevo arreglo de elementos de la tabla hash
  for(unsigned idx = 0; idx < tabla->capacidad; idx++){
    tabla->elems[idx].dato = NULL;
    tabla->elems[idx].eliminado = 0;
  }

  // Insertamos nuevamente sus elementos iniciales en el nuevo almacenamiento de la tabla hash
  for(unsigned int idx = 0; idx < nueva_capacidad/2; idx++){
    if(old_array[idx].dato!=NULL && !old_array[idx].eliminado)    
      tablahash_insertar(tabla, old_array[idx].dato);
    // En caso de existir un elemento, lo destruimos
    if(old_array[idx].dato!=NULL)
      tabla->destr(old_array[idx].dato);
  }

  // Liberamos la memoria del arreglo inicial
  free(old_array);
}

/**
 Retorna el numero de elementos de la tabla.
 */
int tablahash_nelems(TablaHash tabla) { return tabla->numElems; }

/**
 Retorna la capacidad de la tabla.
 */
int tablahash_capacidad(TablaHash tabla) { return tabla->capacidad; }

/**
 Destruye la tabla.
 */
void tablahash_destruir(TablaHash tabla) {

  // Destruir cada uno de los datos.
  for (unsigned idx = 0; idx < tabla->capacidad; ++idx)
    if (tabla->elems[idx].dato != NULL)
      tabla->destr(tabla->elems[idx].dato);

  // Liberar el arreglo de casillas y la tabla.
  free(tabla->elems);
  free(tabla);
}


/* 
  Maneja el caso de que se haya encontrado una colisión al momento de insertar un dato.
 */
void colision_manage(TablaHash tabla, int idx, void *dato){  
  // Tendremos que buscar la primer casilla libre a la que acudir para insertar el dato.
  // Es decir:
  // Que el dato sea ==NULL
  // Que el dato haya sido eliminado
  // Que hayamos encontrado un dato igual al que ingresar
  int first_deleted = idx;

  for(;tabla->elems[idx].eliminado ||
  (tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato)!=0);
  idx = (idx + 1) % tabla->capacidad)
  {
    if(tabla->elems[first_deleted].dato && !tabla->elems[idx].dato)
      first_deleted = idx;
  }
  
  // Si la casilla es no nula y el dato ya existía
  if(tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato)==0){
    tabla->destr(tabla->elems[idx].dato);
    tabla->elems[idx].dato = tabla->copia(dato);
    tabla->elems[idx].eliminado = 0;
  }
  // Si existe una casilla eliminada en el cluster    
  else if(tabla->elems[first_deleted].eliminado){
    tabla->elems[first_deleted].dato = tabla->copia(dato);
    tabla->elems[first_deleted].eliminado = 0;
    tabla->numElems++;
  } 
  // Encontramos una celda nula
  else{
    tabla->numElems++;
    tabla->elems[idx].dato = tabla->copia(dato);
    tabla->elems[idx].eliminado = 0;
  }
  
}

/**
 Inserta un dato en la tabla, o lo reemplaza si ya se encontraba.
 */
void tablahash_insertar(TablaHash tabla, void *dato) {

  // En caso que la tabla haya pasado el factor de carga óptimo
  if((float)tabla->numElems / (float)tabla->capacidad > FACTOR_CARGA_MAX)
    tablahash_resize(tabla);

  // Calculamos la posicion del dato dado, de acuerdo a la funcion hash.
  unsigned idx = tabla->hash(dato) % tabla->capacidad;
  // En caso de haber colisiones, haremos el linear probing
  colision_manage(tabla,idx, dato);
}

/**
 Retorna el dato de la tabla que coincida con el dato dado, o NULL si el dato
 buscado no se encuentra en la tabla.
 */
void *tablahash_buscar(TablaHash tabla, void *dato) {

  // Calculamos la posicion del dato dado, de acuerdo a la funcion hash.
  unsigned idx = tabla->hash(dato) % tabla->capacidad;

  // Paramos de buscar al encontrar una celda vacía o encontrar el dato.
  for(;tabla->elems[idx].eliminado || 
  (tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato) != 0);
  idx = (idx + 1) % tabla->capacidad);
    
  if(tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato)==0)
    return tabla->elems[idx].dato;

  return NULL;
}


/* 
  Función auxiliar para eliminar el dato de una casilla
 */
void delete_data(TablaHash tabla, int idx){
  tabla->numElems--;
  tabla->destr(tabla->elems[idx].dato);
  tabla->elems[idx].dato = NULL;
  tabla->elems[idx].eliminado = 1;
}

/**
 Elimina el dato de la tabla que coincida con el dato dado.
 */
int tablahash_eliminar(TablaHash tabla, void *dato) {
  /* 
    En esta función vamos primero a conseguir el índice del array de elementos en el que 
    debería estar el dato. 
    Luego evaluamos dos casos:
    1 - No existe el elemento:
        1.1 - La casilla inicial marcada por el índice está vacía.
        1.2 - Luego de evaluar todo el cluster, seguimos sin encontrar el elemento.

    2 - Existe el elemento:
        2.1 - Encontramos el elemento en la casilla inicial.
        2.2 - Lo encontramos dentro de un cluster:
            2.2.1 - Evaluamos dentro del cluster hasta encontrar el elemento.
  */

  // Calculamos la posicion del dato dado, de acuerdo a la funcion hash.
  unsigned idx = tabla->hash(dato) % tabla->capacidad;

  // Retornar si la casilla estaba vacia.
  if (tabla->elems[idx].dato == NULL && !tabla->elems[idx].eliminado)
    return 0;
  
  if (tabla->comp(tabla->elems[idx].dato, dato) == 0) {
    delete_data(tabla,idx);
    return 1;
  }
  else{
    for(;tabla->elems[idx].eliminado || 
    (tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato)!=0);
    idx = (idx + 1) % tabla->capacidad);
    
    if(tabla->elems[idx].dato && tabla->comp(tabla->elems[idx].dato,dato)==0){
      delete_data(tabla,idx);
      return 1;
    }
  }
  return 0;
}


