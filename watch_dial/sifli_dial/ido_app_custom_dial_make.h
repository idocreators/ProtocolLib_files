#ifndef __APP_CUSTOM_DIAL_MAKE_H__
#define __APP_CUSTOM_DIAL_MAKE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



//////////////////////////////////////////////////////////////////////
#if 1
#define DEBUG_INFO(fmt, ...)   do { \
    printf("%s(%d):"fmt"\r\n", __func__, __LINE__, ##__VA_ARGS__);  \
}while(0)

#define LOG_INFO DEBUG_INFO
#endif


//////////////////////////////////////////////////////////////////////
struct ido_list_node {
    struct ido_list_node *next;                          /**< point to next node. */
    struct ido_list_node *prev;                          /**< point to prev node. */
};
typedef struct ido_list_node ido_list_node_t;                  /**< Type for lists. */
typedef struct ido_list_node ido_list_head_t;

#ifndef IDO_OFFSETOF
#define IDO_OFFSETOF(T, x)                          ((size_t) &((T *)0)->x)
#endif
#ifndef IDO_CONTAINER_OF
#define IDO_CONTAINER_OF(ptr, type, member)         ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif
#define ido_list_entry(node, type, member)          IDO_CONTAINER_OF(node, type, member)
// 遍历链表
#define ido_list_for_each(pos, head)                for (pos = (head)->next; pos != (head); pos = pos->next)
// 安全遍历整个链表
#define ido_list_for_each_safe(pos, n, head)        for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

// @brief 初始化链表
static inline void ido_list_head_init(ido_list_node_t* p_node)
{
    p_node->next = p_node->prev = p_node;
}
// @brief 将新节点添加在某个节点 node 前面
static inline void ido_list_insert_before(ido_list_node_t* p_new, ido_list_node_t* p_node)
{
    p_node->prev->next = p_new;
    p_new->prev = p_node->prev;
    p_node->prev = p_new;
    p_new->next = p_node;
}
// @brief 将新节点添加在某个节点 node 后面
static inline void ido_list_insert_after(ido_list_node_t* p_new, ido_list_node_t* p_node)
{
    p_node->next->prev = p_new;
    p_new->next = p_node->next;
    p_node->next = p_new;
    p_new->prev = p_node;
}
// @brief 删除链表节点
static inline void ido_list_del(ido_list_node_t *p_node)
{
    p_node->next->prev = p_node->prev;
    p_node->prev->next = p_node->next;
    p_node->next = p_node->prev = p_node;
}
// @brief 替换
static inline void ido_list_replace(ido_list_node_t *p_old, ido_list_node_t *p_new)
{
    p_new->next = p_old->next;
    p_new->next->prev = p_new;
    p_new->prev = p_old->prev;
    p_new->prev->next = p_new;
}
// @brief 判断链表是否为空
static inline int ido_list_is_empty(const ido_list_node_t *p_head)
{
    return (p_head->next == p_head);
}
// @brief 判断节点是否为最后一个节点
static inline int ido_list_is_last(const ido_list_node_t *p_list, const ido_list_node_t *p_head)
{
    return (p_list->next == p_head);
}






////////////////////////////////配置文件//////////////////////////////////////
#define IDO_GUI_FILE_MAGIC_VAL                  0x7f8e3d4c
typedef struct {
    uint32_t magic;                         // 魔数：0x7f8e3d4c
    uint32_t version;
    uint32_t size;
    uint16_t widget_count;
    uint16_t count;                         // 有效元素数量(不包含控件类型为4的控件数量)
    uint32_t id;
    uint32_t checksum;
    uint32_t flag;
} ido_gui_file_bin_head_t;

#define IDO_FILE_WIDGET_TYPE_NONE               0
#define IDO_FILE_WIDGET_TYPE_PREVIEW            1
#define IDO_FILE_WIDGET_TYPE_IMAGE              2
#define IDO_FILE_WIDGET_TYPE_IMAGE_SET          3
#define IDO_FILE_WIDGET_TYPE_IMAGE_SET_EX       4
#define IDO_FILE_WIDGET_TYPE_NUM_IMAGE_SET      5
#define IDO_FILE_WIDGET_TYPE_NUM_IMAGE_SET_EX   6
#define IDO_FILE_WIDGET_TYPE_MUL_LANG           7
#define IDO_FILE_WIDGET_TYPE_ROTATING_POINTER   8
#define IDO_FILE_WIDGET_TYPE_UTF8_DATA          9
#define IDO_FILE_WIDGET_TYPE_VIDEO              10
typedef uint8_t ido_file_widget_type_t;

#define IDO_DAIL_DATA_TYPE_NONE                 0
#define IDO_DAIL_DATA_TYPE_YEAR                 1
#define IDO_DAIL_DATA_TYPE_MONTH                2
#define IDO_DAIL_DATA_TYPE_DAY                  3
#define IDO_DAIL_DATA_TYPE_HOUR                 4
#define IDO_DAIL_DATA_TYPE_MINUTE               5
#define IDO_DAIL_DATA_TYPE_SECOND               6
#define IDO_DAIL_DATA_TYPE_WEEK                 7
#define IDO_DAIL_DATA_TYPE_AM_PM                8           // 0->AM, 1->PM
#define IDO_DAIL_DATA_TYPE_YMD                  9
#define IDO_DAIL_DATA_TYPE_HMS                  10
#define IDO_DAIL_DATA_TYPE_MD                   11
#define IDO_DAIL_DATA_TYPE_HM                   12
#define IDO_DAIL_DATA_TYPE_MS                   13
#define IDO_DAIL_DATA_TYPE_BLE_IS_LINK          14
#define IDO_DAIL_DATA_TYPE_BT_IS_LINK           15
#define IDO_DAIL_DATA_TYPE_APK_IS_LINK          16
#define IDO_DAIL_DATA_TYPE_NOT_DIS_OPEN         17
#define IDO_DAIL_DATA_TYPE_DAY_STEP             18
#define IDO_DAIL_DATA_TYPE_DAY_CALORIES         19
#define IDO_DAIL_DATA_TYPE_DAY_DISTANCE         20
#define IDO_DAIL_DATA_TYPE_DAY_ACTCNT           21
#define IDO_DAIL_DATA_TYPE_DAY_ACT_DURATION     22
#define IDO_DAIL_DATA_TYPE_STEP_PERCENT         23
#define IDO_DAIL_DATA_TYPE_CALORIES_PERCENT     24
#define IDO_DAIL_DATA_TYPE_DISTANCE_PERCENT     25
#define IDO_DAIL_DATA_TYPE_ACTCNT_PERCENT       26
#define IDO_DAIL_DATA_TYPE_ACT_DURATION_PERCENT 27
#define IDO_DAIL_DATA_TYPE_STEP_TARGET          28
#define IDO_DAIL_DATA_TYPE_CALORIES_TARGET      29
#define IDO_DAIL_DATA_TYPE_DISTANCE_TARGET      30
#define IDO_DAIL_DATA_TYPE_ACTCNT_TARGET        31
#define IDO_DAIL_DATA_TYPE_ACT_DURATION_TARGET  32
#define IDO_DAIL_DATA_TYPE_HRVAL_LAST           33
#define IDO_DAIL_DATA_TYPE_SPO2VAL_LAST         34
#define IDO_DAIL_DATA_TYPE_STRESSVAL_LAST       35
#define IDO_DAIL_DATA_TYPE_HRVAL                36
#define IDO_DAIL_DATA_TYPE_SPO2VAL              37
#define IDO_DAIL_DATA_TYPE_STRESSVAL            38
#define IDO_DAIL_DATA_TYPE_HRVAL_MAX            39
#define IDO_DAIL_DATA_TYPE_HRVAL_MIN            40
#define IDO_DAIL_DATA_TYPE_HR_RESTING           41
typedef uint16_t ido_dial_data_type_t;


typedef struct {
    uint32_t identify;
    uint32_t data_size;
    uint32_t checksum;                      // crc32
} ido_widget_image_info_t;

typedef struct {
    uint8_t size;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ido_widget_utf8_info_t;

// 大小必须是32byte
typedef struct {
    ido_file_widget_type_t wtype;
    uint8_t xdata;              // 是否存在外部数据
    ido_dial_data_type_t dtype;
    uint16_t x;
    uint16_t y;
    union {
        uint8_t reserve1[24];
        ido_widget_image_info_t img_info;
        ido_widget_utf8_info_t utf8_info;
    };
} ido_gui_file_widget_t;




// @brief 生成在线表盘
// @return  -1: 没有控件
//          -2: json文件加载失败
int32_t ido_app_custom_dial_make(const char* p_path);

/**
 * @brief 获取思澈表盘(.watch)文件占用空间大小，计算规则：
 * nor方案：对表盘所有文件以4096向上取整  -98平台对应的项目，IDW27,205G Pro,IDW28,IDS05，DR03等
 * nand方案：对表盘所有文件以2048向上取整 -99平台对应的项目，GTX12,GTX13,GTR1,TIT21
 * @param path .watch文件路径，包含文件名
 * @param platform 平台类型，目前有98(nor)，99(nand)平台
 * @return size 文件占用磁盘的实际大小，-1:失败，文件路径访问失败，-2:失败，申请内存失败，-3:失败，读取文件失败，-4:失败，输入平台类型不支持
 * */
int32_t get_sifli_watch_file_size(const char *path,int platform);

#ifdef __cplusplus
}
#endif
#endif /*__APP_CUSTOM_DIAL_MAKE_H__*/

