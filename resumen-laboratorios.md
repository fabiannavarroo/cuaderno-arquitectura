# Arquitectura de computadores · Guía de los laboratorios

Material de estudio en español basado en `lab1(2).zip`, `lab2.zip` y `Texto pegado(1).txt`. Revisado el 12 de octubre de 2026.

## 1. Qué está documentado y qué falta

**No hay medidas de las ejecuciones de estos laboratorios en los adjuntos.** En lab1, `AC_LAB1/output/` está vacía. En lab2 hay un enunciado y tres programas, sin tablas finales, trazas ni capturas de sus ejecuciones. Las figuras del PDF ilustran el entorno y el camino de datos; no constituyen un informe de resultados de los tres programas.

Esta guía distingue tres categorías:

- **Documentado:** contenido explícito de los scripts, código y enunciado.
- **Deducido:** valores o comportamiento que se obtienen razonando sobre el código, bajo los supuestos indicados. No son resultados medidos.
- **Pendiente de medir:** tiempos, consumo energético, ciclos, CPI, IPC y número exacto de detenciones en las configuraciones del laboratorio.

El TXT sí contiene 65 preguntas y un solucionario teórico. Se conserva en la web con notas de revisión. Las diez presentaciones que menciona no están adjuntas; las respuestas que dependen de ellas no pueden contrastarse aquí.

## 2. Lab1 · Objetivo y funcionamiento del programa

**Idea que debes saber explicar:** estudiar paralelismo de hilos, tiempo de ejecución y el límite impuesto por una parte secuencial. Es paralelismo a nivel de hilo (TLP), no el pipeline del lab2.

`benchmark.c` almacena una matriz de N × N elementos en un vector de `double`. Inicializa cada elemento con `(i % 100) / 100.0`. Luego aplica repetidamente la operación `sin(temporal) * cos(temporal) + 1.0`.

```text
./benchmark N iteraciones fraccion_secuencial numero_hilos
./benchmark 2000 100 0.10 4
```

Los cuatro argumentos son: dimensión N, repeticiones por elemento, fracción de elementos procesados de forma secuencial y número de hilos POSIX. El ejemplo aparece configurado en los scripts 3 y 4; no implica que haya sido ejecutado.

**Orden real de trabajo:** asignación e inicialización → inicio del cronómetro total → fase secuencial → creación y ejecución de hilos → espera con `pthread_join` → fin del cronómetro total → impresión y liberación de memoria.

La fase secuencial procesa los primeros `floor(N² × f)` elementos. La fase paralela procesa el resto con intervalos contiguos que no se solapan. El reparto usa cociente y resto: los primeros hilos reciben un elemento extra si la división no es exacta. No se necesita un mutex para escribir estos elementos porque cada posición pertenece a un único hilo y los metadatos permanecen disponibles hasta los `join`.

Para N=2000, iteraciones=100 y f=0.10, **deducido del código**:

- 4.000.000 elementos; 400.000 secuenciales y 3.600.000 paralelos.
- 400.000.000 repeticiones de la carga en total, repartidas en 40.000.000 secuenciales y 360.000.000 paralelas.
- Con 4 hilos: 900.000 elementos paralelos por hilo. Con 8: 450.000.
- Memoria de la matriz: 32.000.000 bytes, aproximadamente 30,52 MiB, suponiendo `sizeof(double)=8`; hay pequeñas asignaciones adicionales para hilos y argumentos.

Doblar N cuadruplica N², el almacenamiento de la matriz y el trabajo si las iteraciones no cambian. Doblar las iteraciones duplica el trabajo de cálculo, pero no el tamaño de la matriz. El programa no imprime la matriz ni una suma de comprobación: su CSV informa de tiempos, no verifica numéricamente el resultado completo.

## 3. Lab1 · Compilar y entender cada comando

Los scripts están orientados a Linux y a un clúster con Slurm. `lscpu`, `free` y `perf` no son comandos equivalentes disponibles por defecto en macOS.

```bash
# Dentro de AC_LAB1, en el entorno Linux del laboratorio:
mkdir -p output
gcc -O3 benchmark.c -o benchmark -lpthread -lm
./benchmark 2000 100 0.10 4
```

| Comando u opción | Qué hace y qué debes explicar |
| --- | --- |
| `gcc` | Compila y enlaza el programa C. |
| `-O3` | Activa optimización; mantener la misma opción al comparar. |
| `-o benchmark` | Nombre del ejecutable generado. |
| `-lpthread` | Enlaza la biblioteca de hilos POSIX utilizada por `pthread_create` y `pthread_join`. |
| `-lm` | Enlaza la biblioteca matemática para `sin` y `cos`. |
| `hostname` | Muestra el equipo donde se está ejecutando el comando. |
| `lscpu` | Describe CPU, arquitectura, sockets, núcleos e hilos de hardware. |
| `grep -E` | Filtra la salida con expresiones regulares extendidas. |
| `free -h` | Muestra memoria y swap con unidades legibles. |
| `sbatch script3.sh` | Envía el script a la cola de Slurm; no equivale a ejecutarlo inmediatamente. |
| `squeue -u "$USER"` | Consulta trabajos del usuario; añadido como apoyo de estudio. |
| `salloc` | Reserva recursos para una sesión o comando. |
| `srun` | Lanza un paso de trabajo en los recursos asignados. |
| `perf stat -e ...` | Ejecuta el programa y recoge los eventos solicitados. |

**Distingue tres cosas:** hilos de software que crea el programa, CPUs lógicas que ofrece la máquina y CPUs que reserva Slurm. Un hilo de software no garantiza un núcleo físico dedicado. Los hilos de SMT de un núcleo tampoco equivalen a dos núcleos completos.

## 4. Lab1 · Qué hace cada script

| Archivo | Configuración documentada | Qué resultado se recogería |
| --- | --- | --- |
| `script1.sh` | Partición `avignon`, 1 nodo, 1 tarea, límite de 5 min. | Nombre de nodo, ID de trabajo, CPU y RAM. |
| `script2.sh` | Igual, pero partición `gpus`. | Información de CPU y RAM del nodo, sin medir GPU. |
| `script_local.sh` | Sin directivas Slurm. | Información del equipo donde se ejecuta; ID Slurm solo si existe en el entorno. |
| `script3.sh` | `avignon`, 4 CPUs por tarea; N=2000, 100 iteraciones, f=0.10, 4 hilos. | CSV de tiempos del benchmark. |
| `script4.sh` | Mismos parámetros que script3. | CSV más `task-clock` y `duration_time` de perf. |
| `script5_local.sh` | Compila, exporta parámetros, usa 8 hilos, invoca `salloc` y energía de paquete. | Pretende medir `power/energy-pkg/`; la ejecución en el nodo y los recursos deben revisarse. |

**Partición `gpus` no significa cálculo en GPU.** `script2.sh` solo consulta información; `benchmark.c` usa CPU con pthreads, sin CUDA ni otro código GPU.

Las líneas `#SBATCH` las interpreta `sbatch`. Si haces `bash script3.sh`, para Bash son comentarios y no reservan recursos. `--nodes=1` pide un nodo; `--ntasks=1` una tarea/proceso; `--cpus-per-task=4` reserva cuatro CPUs para esa tarea. El comentario «p. ej. 8 cores» no cambia el valor real 4.

`%j` en las rutas de salida se sustituye por el identificador del trabajo. Los scripts 1 y 2 generan `info_nodo_ID.out/.err`; los scripts 3 y 4, `speedup_ID.out/.err`. El CSV va a stdout; perf normalmente escribe estadísticas en stderr, por lo que en script4 se consultaría también `.err`.

```bash
# Ejemplos de envío, no ejecuciones realizadas en esta revisión:
sbatch script1.sh
sbatch script2.sh
sbatch script3.sh
sbatch script4.sh
```

**Revisión concreta del script5:** no solicita explícitamente ocho CPUs por tarea y no usa `srun`. Una reserva con `salloc` no garantiza por sí sola que el comando se lance en el nodo reservado. Esta es una alternativa propuesta, no el comando original ni una ejecución verificada en el clúster:

```bash
salloc -p avignon -N 1 -n 1 -c 8 --time=00:05:00 \
  srun -n 1 -c 8 perf stat -a -e power/energy-pkg/ \
  ./benchmark 2000 100 0.10 8
```

Antes de interpretar la medida hay que confirmar el nodo de ejecución y que el evento existe y está permitido. Los nombres de partición proceden de los adjuntos; su disponibilidad depende del clúster.

## 5. Lab1 · Leer el CSV y las medidas de perf

La salida del programa tiene siete campos, en este orden:

```text
N,Iteraciones,Fraccion_Secuencial,Hilos,Tiempo_Sec,Tiempo_Par,Tiempo_Total
```

Los tiempos están en segundos. `Tiempo_Sec` mide solo el bucle secuencial. `Tiempo_Par` incluye la asignación de estructuras de hilos, reparto, creación, ejecución y `join`. `Tiempo_Total` abarca ambas fases y pequeños intervalos adicionales. La asignación e inicialización de la matriz, la impresión y los `free` quedan fuera del cronómetro total.

`clock_gettime(CLOCK_MONOTONIC, ...)` mide tiempo transcurrido con un reloj que no sufre los saltos de ajuste del reloj civil. No suma tiempos de CPU de todos los hilos. La función de diferencia convierte nanosegundos a segundos mediante `1e9`.

**No confundas estas medidas:**

- `task-clock`: tiempo acumulado de actividad de CPU de las tareas/hilos medidos; puede superar el tiempo de pared porque varios hilos trabajan a la vez.
- `duration_time`: duración transcurrida de la ejecución medida por perf. Consulta las unidades de la salida; no la dividas por `task-clock` sin convertirlas.
- `task-clock / tiempo_de_pared`, con unidades iguales: aproxima CPUs utilizadas de media; no es el speedup.
- `power/energy-pkg/`: energía del paquete de CPU según el contador disponible. `-a` solicita medida global del sistema; puede incluir trabajo ajeno, actividad de otros núcleos y consumo de fondo. No es automáticamente energía exclusiva del programa ni del ordenador completo.

El tiempo externo de perf no tiene exactamente los mismos límites que `Tiempo_Total`: incluye partes de la vida del proceso que el cronómetro interno excluye. No deben coincidir obligatoriamente.

Para comparar energía usa julios; para potencia media, `P_media = E / t` en vatios. Más vatios no implica necesariamente más julios si la ejecución termina antes. **No hay valores de perf ni energía documentados en este ZIP.**

## 6. Lab1 · Amdahl, speedup y comparación justa

`S(h)=T(1)/T(h)` compara el mismo trabajo en un hilo y h hilos. `Eficiencia(h)=S(h)/h`. Si S<1, la versión paralela tarda más. El script llamado `speedup` imprime un único tiempo: no calcula el speedup ni proporciona la referencia de un hilo.

Si f es la fracción de tiempo secuencial de la ejecución de referencia, el modelo ideal de Amdahl es:

```text
S_ideal(h) = 1 / (f + (1-f)/h)
S_max = 1/f, si f > 0
```

**Cálculo teórico, no resultado observado:** para f=0.10, con 4 hilos S≈3,08; con 8, S≈4,71; el límite con infinitos recursos es 10. La fracción que recibe el programa es una fracción de elementos; se aproxima a una fracción de trabajo porque aplica la misma carga por elemento, pero no garantiza un 10 % exacto del tiempo real.

Una comparación útil mantiene N, iteraciones, compilación, máquina y condiciones iguales, y cambia solo los hilos. Reserva recursos suficientes, mide también con un hilo, repite y utiliza una estadística consistente (por ejemplo, mediana), guardando dispersión y parámetros. No compares un caso de 4 hilos en un nodo con otro de 8 en una máquina diferente como si solo hubiera cambiado el paralelismo.

Las desviaciones del modelo pueden explicarse por creación/sincronización de hilos, límites de CPUs reservadas, SMT, frecuencia, carga externa, cachés y memoria. No elijas una causa concreta sin evidencia. El código no comprueba el retorno de `pthread_create`/`pthread_join` ni valida todos los argumentos o posibles desbordamientos de N²; son limitaciones del benchmark proporcionado.

## 7. Lab1 · Preguntas abiertas para responder en voz alta

**¿Por qué un proceso puede aprovechar varios núcleos?** Porque crea hilos POSIX que el sistema puede planificar simultáneamente. `--ntasks=1` no limita el programa a un único hilo.

**¿Por qué cuatro hilos no garantizan cuatro veces menos tiempo?** La parte secuencial no se reparte y la fase paralela tiene costes de gestión. Amdahl da un modelo ideal; después hay que medir.

**¿Qué representa 0.10 en el comando?** El 10 % de elementos se procesa primero en el hilo principal. No es «10 % de CPUs» ni una medida exacta del tiempo secuencial.

**¿Por qué usamos join?** Para esperar a que terminen todos los hilos antes de detener el cronómetro de la fase paralela y liberar memoria compartida.

**¿Por qué no aparecen condiciones de carrera al escribir la matriz?** Los intervalos son disjuntos. Dos hilos no actualizan el mismo elemento.

**¿Qué harías si ocho hilos son más lentos que cuatro?** Revisaría recursos reservados, máquina, repetibilidad, CPU/SMT, carga y sobrecostes. El número de hilos por sí solo no identifica la causa.

**¿Qué sabes del hardware de avignon y gpus con estos adjuntos?** Solo los nombres de las particiones y los comandos que lo consultarían. No están sus salidas; no se pueden dar modelos de CPU ni cantidades de RAM.

**¿Qué pasa con f=1 o f=0?** Con f=1, toda la carga útil es secuencial, aunque el programa todavía crea hilos para intervalos vacíos. Con f=0, toda la carga útil se reparte, pero permanecen sobrecostes y recursos finitos.

## 8. Lab2 · Objetivo, etapas y configuración

El enunciado `Lab2_CPUPipeline.pdf`, de David Expósito Singh, estudia ILP con Ripes. Actividad 1: abrir el entorno y cargar el apoyo. Actividades 2–4: ejecutar code1 en ciclo único, cinco etapas sin forwarding y cinco etapas con forwarding. Actividad 5: code2 y saltos. Actividad 6: code3, RAW y reordenamiento.

| Etapa | Responsabilidad |
| --- | --- |
| IF | Captar la instrucción usando el PC. |
| ID | Decodificar y leer registros fuente. |
| EX | Operación ALU, dirección de memoria o decisión/destino de salto en este modelo. |
| MEM | Acceso a memoria de datos si la instrucción lo requiere. |
| WB | Escritura del resultado en el banco de registros, cuando corresponde. |

Segmentar permite solapar instrucciones. No evita que una instrucción recorra su camino ni garantiza menor latencia individual. `CPI=ciclos/instrucciones ejecutadas` e `IPC=instrucciones ejecutadas/ciclos`; son recíprocos si usan el mismo conjunto y límites de medida. Una burbuja o instrucción anulada no equivale a una instrucción útil terminada.

Para comparar tiempo real también importa el periodo de reloj: `T=IC×CPI/frecuencia`. Un procesador de ciclo único puede tener CPI=1 y necesitar un ciclo largo. Un pipeline puede necesitar más ciclos para un programa corto y aun así permitir ciclos más breves. No hay frecuencia física medida en el material.

Selecciona **sin forwarding pero con detección de riesgos**, tal como la figura de la página 4. No lo confundas con un modelo sin detección de riesgos: ese puede dar registros incorrectos si no se añaden esperas. Para `mul` hace falta la extensión de multiplicación M. Anota modelo, ISA/extensiones, versión y estado inicial de registros antes de comparar.

## 9. Lab2 · code1: aritmética y forwarding

```asm
.text
main:
    li x5, 10
    li x6, 20
    add x7, x5, x6
    sub x8, x6, x5
    mul x9, x7, x8
```

**Resultado deducido para una ejecución correcta:** x5=10, x6=20, x7=30, x8=10, x9=300. Los tres modelos correctos deben conservar estos valores; cambia el calendario, no la intención del programa.

Dependencias RAW: ambas `li` producen operandos de `add` y `sub`; `add` produce x7 para `mul`, y `sub` produce x8 para `mul`. `add` y `sub` no dependen entre sí, pero comparten productores anteriores. No basta mirar instrucciones consecutivas para identificar todas las dependencias.

Sin forwarding, un consumidor debe esperar hasta poder leer el valor actualizado del banco de registros. El número exacto de ciclos depende de las reglas de lectura/escritura del modelo. Con forwarding, los resultados aritméticos pueden llegar a EX del consumidor antes del WB del productor; no se requieren por ese motivo las mismas esperas. No atribuyas una latencia de multiplicación universal a cualquier CPU.

`li` es una pseudoinstrucción. Para los inmediatos pequeños 10 y 20 puede traducirse a una instrucción real cada una. Verifica el listado ensamblado para contar instrucciones reales; `.text` y `main:` son directiva y etiqueta, no instrucciones ejecutadas.

**Ejemplo ideal para comprender el llenado, no medida de Ripes:** 5 instrucciones reales en 5 etapas, sin riesgos y con una etapa por ciclo, necesitan `5+5−1=9` ciclos; CPI=1,8 e IPC≈0,56 para ese programa corto. En régimen largo el CPI ideal se aproxima a 1. Los adjuntos no documentan los ciclos reales de code1.

## 10. Lab2 · code2: bucle y riesgo de control

```asm
.text
main:
    li x5, 0
    li x6, 3
loop:
    addi x5, x5, 1
    bne  x5, x6, loop
    add x7, x5, x5
    add x8, x5, x5
```

`bne` salta si los registros son distintos. El bucle incrementa x5 hasta 3. **Deducido:** el salto tiene resultados T, T, NT; termina con x5=3, x6=3, x7=6 y x8=6.

Hay 6 instrucciones escritas, pero 10 ejecuciones útiles si cada `li` se expande a una instrucción: 2 inicializaciones + 3 incrementos + 3 saltos + 2 sumas finales. Las instrucciones del camino incorrecto no deben contarse como trabajo útil terminado.

**Regla específica del PDF, página 6:** política por defecto «salto no tomado» y condición/destino calculados en EX. Los dos primeros saltos fallan la predicción y requieren anular trabajo del camino secuencial; el tercero acierta. También existe una RAW de `addi` a `bne` sobre x5, que el forwarding del modelo debe resolver según su camino de datos.

En un pipeline convencional de cinco etapas que resuelve en EX, puede haber instrucciones más jóvenes en ID e IF cuando se detecta un salto tomado. Esa explicación no es una medición de la penalización exacta de Ripes; hay que observar su traza y la convención de conteo. No traslades aquí la resolución en ID3 del ejercicio anterior de la conversación.

**Alternativa propuesta para este número fijo de iteraciones:** desenrollar el bucle.

```asm
li x5, 0
li x6, 3
addi x5, x5, 1
addi x5, x5, 1
addi x5, x5, 1
add x7, x5, x5
add x8, x5, x5
```

Elimina los saltos y conserva los registros finales para este caso. Las tres `addi` aún forman una cadena RAW. Cambiar la predicción no elimina todas las dependencias. Si el número de iteraciones fuese variable, esta sustitución fija ya no sería una solución general.

## 11. Lab2 · code3: carga, load-use y reordenamiento

```asm
.data
val: .word 42
.text
main:
    la x10, val
    lw x5, 0(x10)
    add x6, x5, x7
    addi x8, x9, 1
    addi x11, x12, 2
```

`.word 42` define un dato de memoria. `la` carga una dirección y puede expandirse en varias instrucciones reales; `lw` lee la palabra en esa dirección. RAW: la producción de x10 → `lw`, y la producción de x5 por `lw` → `add`.

**Resultado deducido:** x10 apunta a `val`, x5=42, x6=42+x7_inicial, x8=x9_inicial+1 y x11=x12_inicial+2, con la aritmética de la ISA. No se proporciona la dirección numérica de `val` ni los valores iniciales de x7, x9 y x12. Si esos tres registros se inicializan a cero, entonces x6=42, x8=1 y x11=2; es un supuesto explícito, no una captura de ejecución.

Para una operación ALU el dato se calcula al acabar EX. Para una carga, EX solo calcula la dirección; el dato se obtiene al acabar MEM. Por eso, en el pipeline clásico con forwarding, una `add` inmediatamente detrás de `lw` suele necesitar un stall: querría el dato en EX antes de que la carga lo tenga. El forwarding no crea datos que aún no existen.

Las dos `addi` finales son independientes de x5, x6 y x10. Una propuesta de reordenamiento para el estado final de este fragmento es:

```asm
la x10, val
lw x5, 0(x10)
addi x8, x9, 1
addi x11, x12, 2
add x6, x5, x7
```

Ocupa el espacio entre carga y consumidor con trabajo útil. Con forwarding suele evitar la espera load-use inmediata; sin forwarding permite que el dato se acerque a WB, pero no garantiza eliminar todas las detenciones: siguen importando la dependencia de `la`, su expansión y la convención del banco de registros. Hay que volver a medir, no restar automáticamente un número fijo de ciclos.

Mover instrucciones debe preservar dependencias, direcciones y efectos visibles. Aquí se propone para una carga válida y comparación de estado final; no establece que cualquier reordenamiento preserve el estado ante excepciones o E/S.

## 12. Lab2 · Cómo recoger resultados sin mezclar conceptos

Para cada caso: carga el programa, reinicia, comprueba registros iniciales, selecciona el modelo correcto, examina el ensamblado real, avanza por ciclos para explicar dependencias y ejecuta hasta el mismo punto final. Guarda ciclos, instrucciones ejecutadas, registros finales y traza. Los códigos proporcionados no incluyen un `ecall` de salida; documenta el criterio de parada y no añadas instrucciones solo en una de las comparaciones.

| Experimento del enunciado | Qué falta registrar |
| --- | --- |
| code1 · ciclo único | Ciclos, instrucciones reales ejecutadas, CPI, IPC, registros finales. |
| code1 · 5 etapas sin forwarding | Las mismas métricas y stalls por RAW. |
| code1 · 5 etapas con forwarding | Las mismas métricas y caminos de anticipación utilizados. |
| code2 · 5 etapas con forwarding | Métricas, tres resultados de salto, instrucciones anuladas y penalización. |
| code3 · sin/con forwarding | Métricas, estado inicial, stalls de cada dependencia y expansión de `la`. |
| Variantes propuestas de code2/code3 | Estado final equivalente y métricas recogidas con los mismos criterios. |

Si te dan C ciclos e I instrucciones ejecutadas, calcula `CPI=C/I` e `IPC=I/C`. Para un bucle cuenta instrucciones dinámicas, no solo líneas. Separa coste de llenado/vaciado, esperas RAW y anulaciones de control. No sumes dos penalizaciones que se solapan en el mismo ciclo sin comprobar la traza.

## 13. Lab2 · Preguntas abiertas frecuentes

**¿Forwarding elimina RAW?** Conserva la dependencia verdadera, pero permite entregar el resultado antes de WB y reduce las detenciones. No elimina una espera si el dato todavía no está disponible.

**¿Stall y flush son lo mismo?** Stall retiene una instrucción válida mientras espera; flush invalida instrucciones, por ejemplo las de un camino mal predicho. Las provoca lógica de riesgos/control y afectan las etapas correspondientes, no necesariamente toda la CPU por igual.

**¿Por qué deben coincidir los registros con y sin forwarding?** Porque cambia la implementación de la entrega de operandos, no el significado del programa. Una discrepancia obliga a revisar modelo, detección de riesgos y estado inicial.

**¿Por qué comparar CPI no basta para decir qué máquina es más rápida?** También cuentan frecuencia e instrucciones ejecutadas. En un programa corto pesa el llenado del pipeline.

**¿Cuántas veces falla la predicción no tomada en code2?** Dos, porque los resultados son T,T,NT. Esto se deduce del código; el coste exacto en ciclos no está medido.

**¿Por qué reordenar code3 sirve?** Acerca el uso del dato a su disponibilidad sin introducir un NOP: ejecuta instrucciones independientes mientras la carga avanza.

**¿Por qué el contador de instrucciones podría superar las líneas del programa?** Por bucles y pseudoinstrucciones expandidas. Las directivas no cuentan como instrucciones ejecutadas.

**¿Un NOP y una burbuja cuentan igual?** Ambos pueden ocupar tiempo sin trabajo útil; un NOP explícito es una instrucción del programa y una burbuja es un hueco introducido por el control. Revisa qué contabiliza la herramienta.

## 14. Puente con el TXT · Marcador y trampas

El lab2 usa un pipeline en orden de Ripes. El Marcador y Tomasulo del TXT son temas relacionados, pero no son los modelos configurados en este laboratorio.

| Riesgo en Marcador clásico | Dónde espera | Qué lo desbloquea |
| --- | --- | --- |
| WAW | IS | El escritor anterior completa WB. |
| RAW | RO | El productor escribe el operando en WB. |
| WAR | WB | El lector anterior completa RO. |
| Estructural en emisión | IS | Se libera la unidad funcional requerida. |

**IS bloqueante:** si una instrucción no puede emitirse, las posteriores no pueden adelantarla en emisión. Las ya emitidas pueden seguir avanzando. Un atasco en RO no bloquea por sí solo todas las emisiones posteriores. Los ciclos exactos dependen de cuándo el modelo permite usar un evento de ese ciclo.

Tomasulo renombra para evitar WAR/WAW por nombres, pero mantiene RAW reales. Las estaciones de reserva esperan operandos; el CDB difunde resultados. Un ROB en una organización moderna permite retirar en orden. Las dependencias de memoria no desaparecen al renombrar registros.

**Corrección de la pregunta 65 del TXT:** si antes el store usaba `0(t1)` y ahora se ejecuta `addi t1,t1,-8` primero, la dirección antigua se conserva con `8(t1)`, no con `-8(t1)`. Si el puntero antiguo era P, el nuevo es P−8; `(P−8)+8=P`. El solucionario original se mantiene visible con esta corrección destacada.

**Contexto que falta en la pregunta 64:** el TXT afirma «predecir no tomada, 0,38 ciclos/instrucción» para un reparto concreto del MIPS R4000. No están las diapositivas con ese reparto. Ese valor no puede verificarse ni trasladarse a Ripes. La fórmula de una contribución por saltos sería frecuencia_de_saltos × tasa_de_fallo × penalización, si no hay otros costes o solapamientos.

**Convenciones del TXT:** 52 usa 10⁹, no 109; 55 usa 0,9²; 61 usa 2¹⁰; 63 usa 2². Son exponentes mal representados al pegar el texto. En 62 se asume contador saturante habitual 00/01=NT y 10/11=T. En 63, `E×2^m×n=8192` bits cuenta los contadores del esquema indicado, sin incluir necesariamente todo el hardware. Gustafson compara trabajo escalado; su relación con weak scaling es conceptual, no una definición idéntica para cualquier carga.

## 15. Fuentes, atribución y última sesión de repaso

Fuentes de los adjuntos: `AC_LAB1/benchmark.c` y sus seis scripts; `Lab2_CPUPipeline.pdf` (páginas 2–7); `code1.asm`, `code2.asm`, `code3.asm`; `Texto pegado(1).txt`. No se ha ejecutado el benchmark del clúster ni simulado los programas para generar medidas adicionales.

El PDF original atribuye el material a **David Expósito Singh** y muestra licencia **CC BY-NC-SA**. El resumen del lab2 es una adaptación de estudio; se conserva esa atribución y la condición CC BY-NC-SA para esa adaptación. El original se incluye intacto en la carpeta material.

Documentación técnica consultada para contrastar comandos y modelos:

- [Slurm: salloc](https://slurm.schedmd.com/salloc.html) y [guía de inicio](https://slurm.schedmd.com/quickstart.html): reserva, envío y lanzamiento de tareas.
- [Manual de perf stat](https://man7.org/linux/man-pages/man1/perf-stat.1.html): eventos, salida y alcance global de `-a`.
- [Ripes: implementación de cinco etapas sin forwarding](https://github.com/mortbopet/Ripes/blob/master/src/processors/RISC-V/rv5s_no_fw/rv5s_no_fw.h): detección de riesgos y etapas del modelo.
- [Ripes: explicación del autor sobre modelos sin detección de riesgos](https://github.com/mortbopet/Ripes/issues/233): no garantizan resultados correctos ante riesgos.
- [Ripes online](https://ripes.dk/): entorno indicado por el enunciado; este enlace requiere conexión, la web de estudio funciona sin ella.

**Último repaso:** explica los cuatro argumentos de benchmark, los siete campos del CSV, diferencia reserva/hilos/núcleos, calcula Amdahl sin confundir teoría y medida; después recorre los tres ASM, identifica productores/consumidores, justifica T,T,NT, distingue EX de MEM y explica por qué reordenar puede ahorrar esperas. Si falta una cifra experimental, di qué medirías y cómo: esa es una respuesta técnica mejor que inventarla.
