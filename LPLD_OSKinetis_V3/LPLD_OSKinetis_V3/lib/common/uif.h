/**
 * @file uif.h
 * @version 3.0[By LPLD]
 * @date 2013-06-18
 * @brief �ṩ�򵥵Ľ����ӿ�
 *
 * ���Ľ���:�������޸�
 *
 * The commands, set/show parameters, and prompt are configured
 * at the project level
 *
 * ��Ȩ����:�����������µ��Ӽ������޹�˾
 * http://www.lpld.cn
 * mail:support@lpld.cn
 *
 * @par
 * ����������������[LPLD]������ά������������ʹ���߿���Դ���롣
 * �����߿���������ʹ�û��Դ���롣�����μ�����ע��Ӧ���Ա�����
 * ���ø��Ļ�ɾ��ԭ��Ȩ���������������ο����߿��Լ�ע���ΰ�Ȩ�����ߡ�
 * ��Ӧ�����ش�Э��Ļ����ϣ�����Դ���롢���ó��۴��뱾��
 * �������²���������ʹ�ñ��������������κ��¹ʡ��������λ���ز���Ӱ�졣
 * ����������������͡�˵��������ľ���ԭ�����ܡ�ʵ�ַ�����
 * ������������[LPLD]��Ȩ�������߲��ý�������������ҵ��Ʒ��
 */


#ifndef _UIF_H_
#define _UIF_H_

/********************************************************************/

/*
 * Function prototypes
 */
int8_t *get_line(int8_t *);

uint32_t get_value(int8_t *, int32_t *, int32_t);

void run_cmd(void);

int32_t make_argv(int8_t *, int8_t **);

void uif_cmd_help(int32_t, int8_t **);

void uif_cmd_set(int32_t, int8_t **);

void uif_cmd_show(int32_t, int8_t **);

/*
 * Maximum command line arguments
 */
#define UIF_MAX_ARGS    10

/*
 * Maximum length of the command line
 */
#define UIF_MAX_LINE    80

/*
 * The command table entry data structure
 */
typedef const struct {
    int8_t   *cmd;                    /* command name user types, ie. GO  */
    int32_t     min_args;               /* min num of args command accepts  */
    int32_t     max_args;               /* max num of args command accepts  */
    int32_t     flags;                  /* command flags (e.g. repeat)      */
    void (*func)(int32_t, int8_t **);     /* actual function to call          */
    int8_t   *description;            /* brief description of command     */
    int8_t   *syntax;                 /* syntax of command                */
} UIF_CMD;

/*
 * Prototype and macro for size of the command table
 */
extern UIF_CMD UIF_CMDTAB[];
extern const int32_t UIF_NUM_CMD;
#define UIF_CMDTAB_SIZE             (sizeof(UIF_CMDTAB)/sizeof(UIF_CMD))

#define UIF_CMD_FLAG_REPEAT         0x1

/*
 * Macros for User InterFace command table entries
 */
#ifndef UIF_CMD_HELP
#define UIF_CMD_HELP    \
    {"help",0,1,0,uif_cmd_help,"Help","<cmd>"},
#endif

#ifndef UIF_CMD_SET
#define UIF_CMD_SET \
    {"set",0,2,0,uif_cmd_set,"Set Config","<option value>"},
#endif

#ifndef UIF_CMD_SHOW
#define UIF_CMD_SHOW    \
    {"show",0,1,0,uif_cmd_show,"Show Config","<option>"},
#endif

/*
 * Macro to include all standard user interface commands
 */
#define UIF_CMDS_ALL    \
    UIF_CMD_HELP        \
    UIF_CMD_SET         \
    UIF_CMD_SHOW

/*
 * The set/show table entry data structure
 */
typedef const struct {
    int8_t   *option;
    int32_t     min_args;
    int32_t     max_args;
    void (*func)(int32_t, int8_t **);
    int8_t   *syntax;
} UIF_SETCMD;

/*
 * Prototype and macro for size of the table
 */
extern UIF_SETCMD UIF_SETCMDTAB[];
extern const int32_t UIF_NUM_SETCMD;
#define UIF_SETCMDTAB_SIZE      (sizeof(UIF_SETCMDTAB)/sizeof(UIF_SETCMD))

/*
 * Strings defined in uif.c that may be useful to external functions
 */
extern const int8_t HELPMSG[];
extern const int8_t INVARG[];
extern const int8_t INVALUE[];

/********************************************************************/

#endif /* _UIF_H_ */
