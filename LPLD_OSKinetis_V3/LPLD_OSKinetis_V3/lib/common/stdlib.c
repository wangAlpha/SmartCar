/**
 * @file stdlib.c
 * @version 3.01[By LPLD]
 * @date 2013-11-4
 * @brief C标准代码库
 *
 * 更改建议:不建议修改
 *
 * 由标准C代码库移植，仅支持ASCII
 *
 * 版权所有:北京拉普兰德电子技术有限公司
 * http://www.lpld.cn
 * mail:support@lpld.cn
 *
 * @par
 * 本代码由拉普兰德[LPLD]开发并维护，并向所有使用者开放源代码。
 * 开发者可以随意修使用或改源代码。但本段及以上注释应予以保留。
 * 不得更改或删除原版权所有者姓名，二次开发者可以加注二次版权所有者。
 * 但应在遵守此协议的基础上，开放源代码、不得出售代码本身。
 * 拉普兰德不负责由于使用本代码所带来的任何事故、法律责任或相关不良影响。
 * 拉普兰德无义务解释、说明本代码的具体原理、功能、实现方法。
 * 除非拉普兰德[LPLD]授权，开发者不得将本代码用于商业产品。
 */
#include "stdlib.h"


/****************************************************************/
int isspace(int ch)
{
    if ((ch == ' ') || (ch == '\t')) {  /* \n ??? */
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/****************************************************************/
int isalnum(int ch)
{
    /* ASCII only */
    if (((ch >= '0') && (ch <= '9')) ||
        ((ch >= 'A') && (ch <= 'Z')) ||
        ((ch >= 'a') && (ch <= 'z'))) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/****************************************************************/
int isdigit(int ch)
{
    /* ASCII only */
    if ((ch >= '0') && (ch <= '9')) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/****************************************************************/
int isupper(int ch)
{
    /* ASCII only */
    if ((ch >= 'A') && (ch <= 'Z')) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/****************************************************************/
int strcasecmp(const int8_t *s1, const int8_t *s2)
{
    int8_t    c1, c2;
    int     result = 0;
    while (result == 0) {
        c1 = *s1++;
        c2 = *s2++;
        if ((c1 >= 'a') && (c1 <= 'z')) {
            c1 = (int8_t)(c1 - ' ');
        }
        if ((c2 >= 'a') && (c2 <= 'z')) {
            c2 = (int8_t)(c2 - ' ');
        }
        if ((result = (c1 - c2)) != 0) {
            break;
        }
        if ((c1 == 0) || (c2 == 0)) {
            break;
        }
    }
    return result;
}


/****************************************************************/
int strncasecmp(const int8_t *s1, const int8_t *s2, int n)
{
    int8_t    c1, c2;
    int     k = 0;
    int     result = 0;
    while (k++ < n) {
        c1 = *s1++;
        c2 = *s2++;
        if ((c1 >= 'a') && (c1 <= 'z')) {
            c1 = (int8_t)(c1 - ' ');
        }
        if ((c2 >= 'a') && (c2 <= 'z')) {
            c2 = (int8_t)(c2 - ' ');
        }
        if ((result = (c1 - c2)) != 0) {
            break;
        }
        if ((c1 == 0) || (c2 == 0)) {
            break;
        }
    }
    return result;
}

/****************************************************************/
unsigned long strtoul(int8_t *str, int8_t **ptr, int base)
{
    unsigned long rvalue;
    int c, err, neg;
    int8_t *endp;
    int8_t *startp;
    rvalue = 0;
    err = 0;
    neg = 0;
    /* Check for invalid arguments */
    if ((str == NULL) || (base < 0) || (base == 1) || (base > 36)) {
        if (ptr != NULL) {
            *ptr = str;
        }
        return 0;
    }
    /* Skip leading white spaces */
    for (startp = str; isspace(*startp); ++startp)
        ;
    /* Check for notations */
    switch (startp[0]) {
    case '0':
        if ((startp[1] == 'x') || (startp[1] == 'X')) {
            if ((base == 0) || (base == 16)) {
                base = 16;
                startp = &startp[2];
            }
        }
        break;
    case '-':
        neg = 1;
        startp = &startp[1];
        break;
    default:
        break;
    }
    if (base == 0) {
        base = 10;
    }
    /* Check for invalid chars in str */
    for (endp = startp; (c = *endp) != '\0'; ++endp) {
        /* Check for 0..9,Aa-Zz */
        if (!isalnum(c)) {
            err = 1;
            break;
        }
        /* Convert int8_t to num in 0..36 */
        if (isdigit(c)) {
            c = c - '0';
        }
        else {
            if (isupper(c)) {
                c = c - 'A' + 10;
            }
            else {
                c = c - 'a' + 10;
            }
        }
        /* check c against base */
        if (c >= base) {
            err = 1;
            break;
        }
        if (neg) {
            rvalue = (rvalue * base) - c;
        }
        else {
            rvalue = (rvalue * base) + c;
        }
    }
    /* Upon exit, endp points to the character at which valid info */
    /* STOPS.  No chars including and beyond endp are used.        */
    if (ptr != NULL) {
        *ptr = endp;
    }
    if (err) {
        if (ptr != NULL) {
            *ptr = str;
        }
        return 0;
    }
    else {
        return rvalue;
    }
}

/****************************************************************/
int strlen(const int8_t *str)
{
    int8_t *s = (int8_t *)str;
    int len = 0;
    if (s == NULL) {
        return 0;
    }
    while (*s++ != '\0') {
        ++len;
    }
    return len;
}

/****************************************************************/
int8_t *strcat(int8_t *dest, const int8_t *src)
{
    int8_t *dp;
    int8_t *sp = (int8_t *)src;
    if ((dest != NULL) && (src != NULL)) {
        dp = &dest[strlen(dest)];
        while (*sp != '\0') {
            *dp++ = *sp++;
        }
        *dp = '\0';
    }
    return dest;
}

/****************************************************************/
int8_t *strncat(int8_t *dest, const int8_t *src, int n)
{
    int8_t *dp;
    int8_t *sp = (int8_t *)src;
    if ((dest != NULL) && (src != NULL) && (n > 0)) {
        dp = &dest[strlen(dest)];
        while ((*sp != '\0') && (n-- > 0)) {
            *dp++ = *sp++;
        }
        *dp = '\0';
    }
    return dest;
}

/****************************************************************/
int8_t *strcpy(int8_t *dest, const int8_t *src)
{
    int8_t *dp = (int8_t *)dest;
    int8_t *sp = (int8_t *)src;
    if ((dest != NULL) && (src != NULL)) {
        while (*sp != '\0') {
            *dp++ = *sp++;
        }
        *dp = '\0';
    }
    return dest;
}

/****************************************************************/
int8_t *strncpy(int8_t *dest, const int8_t *src, int n)
{
    int8_t *dp = (int8_t *)dest;
    int8_t *sp = (int8_t *)src;
    if ((dest != NULL) && (src != NULL) && (n > 0)) {
        while ((*sp != '\0') && (n-- > 0)) {
            *dp++ = *sp++;
        }
        *dp = '\0';
    }
    return dest;
}

/****************************************************************/
int strcmp(const int8_t *s1, const int8_t *s2)
{
    /* No checks for NULL */
    int8_t *s1p = (int8_t *)s1;
    int8_t *s2p = (int8_t *)s2;
    while (*s2p != '\0') {
        if (*s1p != *s2p) {
            break;
        }
        ++s1p;
        ++s2p;
    }
    return (*s1p - *s2p);
}

/****************************************************************/
int strncmp(const int8_t *s1, const int8_t *s2, int n)
{
    /* No checks for NULL */
    int8_t *s1p = (int8_t *)s1;
    int8_t *s2p = (int8_t *)s2;
    if (n <= 0) {
        return 0;
    }
    while (*s2p != '\0') {
        if (*s1p != *s2p) {
            break;
        }
        if (--n == 0) {
            break;
        }
        ++s1p;
        ++s2p;
    }
    return (*s1p - *s2p);
}

/****************************************************************/
void *memcpy(void *dest, const void *src, unsigned n)
{
    int longs, bytes;
    uint32_t *dpl = (uint32_t *)dest;
    uint32_t *spl = (uint32_t *)src;
    uint8_t  *dpb, *spb;
    if ((dest != NULL) && (src != NULL) && (n > 0)) {
        bytes = (n & 0x3);
        longs = (n - bytes) >> 2;
        while (longs--) {
            *dpl++ = *spl++;
        }
        dpb = (uint8_t *)dpl;
        spb = (uint8_t *)spl;
        while (bytes--) {
            *dpb++ = *spb++;
        }
    }
    return dest;
}

/****************************************************************/
void *memset(void *s, int c, unsigned n)
{
    /* Not optimized, but very portable */
    uint8_t *sp = (uint8_t *)s;
    if ((s != NULL) && (n > 0)) {
        while (n--) {
            *sp++ = (uint8_t)c;
        }
    }
    return s;
}

/****************************************************************/
