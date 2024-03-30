// Nivel 5
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int nionodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);


