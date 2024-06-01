#include "../include/mmu.h"

uint32_t direccion_logica_a_fisica(int direccion_logica)
{
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    return numero_pagina + desplazamiento;

}


