***********************************************************************
** ███╗   ███╗██╗   ██╗    ███████╗██╗  ██╗███████╗██╗     ██╗       **
** ████╗ ████║╚██╗ ██╔╝    ██╔════╝██║  ██║██╔════╝██║     ██║       **
** ██╔████╔██║ ╚████╔╝     ███████╗███████║█████╗  ██║     ██║       **
** ██║╚██╔╝██║  ╚██╔╝      ╚════██║██╔══██║██╔══╝  ██║     ██║       **
** ██║ ╚═╝ ██║   ██║       ███████║██║  ██║███████╗███████╗███████╗  **
** ╚═╝     ╚═╝   ╚═╝       ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝  **
***********************************************************************
***********************************************************************
**** Pràctica 2 per *************************************
*       Josep Ferriol Font     **********************
*       Daniel García Vázquez  *******************
*       Biel Perelló Perelló   **************
*************************************
**** A la carpeta s'inclou *******
*   nivel1-6.c                   *
*   my_shell.c                   *
*   Makefile                     *
*   README.txt                   *
**********************************
**** Observacions per nivells ****
*   1. El prompt és un placeholder, als següents nivells s'implementa el definitiu.
*      Aquest nivell te el parse_args que detecta '\ ', però no va ser portat als nivells posteriors.
*   2. En un principi no tenia el cd avançat, però va ser backporteat.
*      Aquest cd avançat està disenyat per a ser usat amb cometes dobles o simples, però
*      no ambdues a la vegada, encara que parse_args() es bastant permisiu amb la combinació
*      de cometes i espais
*   3. internal_source va tenir bugs que es varen solucionar més endavant.
*   4. Debuguear des d'aquest nivell les senyals des de VSC no era posible segons la
*      configuració, d'aquest, era més rapid usar gdb
*   5. El tractament de procesos en segon pla tenia bugs que sense fg i bg varen passar
*      dessapercebuts fins al nivell 6
*   6. Es varen solucionar tots els bugs i es va actualitzar els DEBUG per ser
*      DEBUGn, canvi realitzat a tots els nivells.
*************************************************************