# Laboratorio 1 (MyBash)#

## Objetivos

Este proyecto tiene como propósito implementar un intérprete de línea de comandos sencillo (shell) utilizando mecanismos de concurrencia y comunicación de UNIX. Además, busca promover el uso de buenas prácticas de programación, incluyendo:

- **Concurrencia y Comunicación en UNIX**: Utilización de primitivas que permiten la concurrencia y la comunicación entre procesos.
- **Reflejar la Arquitectura de UNIX**: Comprender cómo un intérprete de línea de comandos refleja las primitivas de comunicación y concurrencia del sistema operativo.
- **Buenas Prácticas de Programación**: Implementación de un código claro y estructurado, utilizando:
  - Tipos Abstractos de Datos (TAD)
  - Pruebas unitarias (unit testing)
  - Pruebas de caja cerrada (black box testing)
  - Programación defensiva
  - Uso de herramientas para debugging de memoria y programas

## Test
Se han implementado varias pruebas para garantizar el correcto funcionamiento de los distintos módulos del proyecto. Puedes correr las siguientes pruebas:

- **Pruebas del módulo \`command.c\`** (incluyendo \`scommand\` y \`pipeline\`):
  \`\`\`$
  make test-command
  \`\`\`

- **Pruebas del módulo \`parsing.c\`**:
  \`\`\`$
  make test-parsing
  \`\`\`

- **Pruebas generales** para todos los módulos:
  \`\`\`$
  make test
  \`\`\`

- **Pruebas de manejo de memoria** en los módulos:
  \`\`\`$
  make memtest
  \`\`\`

## Compilación

Para compilar el proyecto, ejecuta el siguiente comando en la terminal:

\`\`\`$
make
\`\`\`

Esto generará un archivo ejecutable que podrás correr con:

\`\`\`$
./mybash
\`\`\`

## Aclaraciones (decisiones de diseño):
- Si se redirecciona (se pone < > ) sin especificar el archivo, el comando se ejecuta normalmente, pero sin redireccionar (osea se imprime por stdout). 
- Si se hace un Pipe, pero solo se pone el primer comando (ej ls | ) no tira error, pero tampoco pasa nada.
  

