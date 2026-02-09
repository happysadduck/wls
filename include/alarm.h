# ifndef ALARM_H
# define ALARM_H

# include"config.h"

# if USE_FILE
# define RAISE_ERROR(message, system) \
printf("[ERROR] line%d: %s\n", system->line_cnt, message);\
system->error_cnt++
# define SHOW_INFO(message, system) \
printf("[INFO] line%d %s\n", system->line_cnt, message)
# else
# define RAISE_ERROR(message, system) \
printf("[ERROR] %s\n", message);\
system->error_cnt++
# define SHOW_INFO(message, system) \
printf("[INFO] %s\n", message)
# endif

# define CHECK_FACT(new_fact) \
if(!new_fact){\
printf("[SEVERER ERROR]: fact pool used up\n");\
return -1;\
}

# define CHECK_NAME(new_name) \
if(!new_name){\
printf("[SEVERER ERROR]: name pool used up\n");\
return -1;\
}

# define CHECK_CONTEXT(new_context) \
if(!new_context){\
printf("[SEVERER ERROR]: too much nesting\n");\
return -1;\
}

# define CHECK_PROP(new_prop) \
if(!new_prop){\
printf("[SEVERER ERROR]: prop pool used up\n");\
return -1;\
}

# endif
