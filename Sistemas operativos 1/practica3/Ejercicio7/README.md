a) Puede ocurrir un deadlock por ejemplo en el caso que no lleguen los dos elementos a un único fumador, sino separados en dos. El fumador 2 podría recibir fosforos y luego el fumador 1 recibir tabaco, generando el deadlock.

b) No es posible arreglar el deadlock únicamente reordenando los recursos. Ya que se genera un círculo de bloqueos
entre ellos. Sea el orden que sea. El deadlock se produce porque cada fumador está esperando un recurso que otro fumador tiene bloqueado y viceversa, formando así un ciclo de espera que impide que cualquier fumador avance.

c) En la implementación dada, la idea principal es ver si una vez agarrado un recurso, se posee el otro recurso necesario para no llevar al
deadlock. En caso que no se posea el otro recurso (es decir, el trywait nos da distinto de cero), devolvemos el recurso.
Si bien funciona correctamente, esto implica que cada fumador "devuelva" un recurso en caso de no tener lo que le falta. Lo ideal sería
enviar desde el agente los recursos de modo más correcto.

d) Un ejemplo sería:

    void agente() {
        while (1) {
            sem_wait(&otra_vez);
            
            int recurso1 = rand() % 3;
            int recurso2;
            do {
                recurso2 = rand() % 3;
            } while (recurso2 == recurso1);
    
            // Separamos por fumador.
            if (recurso1 == 0 && recurso2 == 1) {
                sem_post(&tabaco);
                sem_post(&papel);
            } else if (recurso1 == 0 && recurso2 == 2) {
                sem_post(&tabaco);
                sem_post(&fosforos);
            } else {
                sem_post(&papel);
                sem_post(&fosforos);
            }
        }
    }
