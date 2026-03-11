#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "ido_app_custom_dial_make.h"
#include "ido_cJSON.h"
// #include "debug.h"

//////////////////////////////////////////////////////////////////////
typedef struct {
    ido_list_node_t node;
    char filename[1024];
} file_node_t;

typedef struct {
    ido_list_head_t head;
    uint32_t file_count;
} file_list_t;

typedef struct {
    uint32_t name_offset;           // 文件名字偏移量
    uint32_t size;                  // 该文件大小
    uint16_t checksum;              // 该文件的校验码
    uint16_t reserve;               // 保留位
    uint32_t offset;                // 文件内容在该文件的偏移值
} file_head_t;

typedef struct {
    uint32_t magic;                 // 0xa1b2c8d9
    uint16_t size;                  // 该头部大小
    uint16_t project_no;            // 项目号
    uint16_t file_count;            // 文件数量
    uint16_t checksum;              // 头部以后数据内容校验和
    uint32_t data_size;             // 头部以后数据内容大小
    uint32_t name_size;             // 名字数据大小
    uint8_t reserve[12];             // 保留字节
} head_t;


//////////////////////////////////////////////////////////////////////
static uint32_t malloc_count = 0;
static void* app_malloc(size_t size)
{
    malloc_count++;
    return malloc(size);
}
static void app_free(void* p_ptr)
{
    free(p_ptr);
    malloc_count--;
}
static uint32_t identify_start_number;


static const unsigned char CRC16Hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const unsigned char CRC16Lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

// @brief CRC-16/MODBUS       x16+x15+x2+1
uint16_t crc16_x16x15x21(const uint8_t *p_data, uint32_t data_len, uint16_t crc)
{
    unsigned char ucCRCHi = 0xFF;
    unsigned char ucCRCLo = 0xFF;
    unsigned char iIndex;

    if (crc) {
        ucCRCHi = (unsigned char)((crc) >> 8);
        ucCRCLo = (unsigned char)(crc);
    }
    while (data_len--) {
        iIndex = ucCRCLo ^ (*(p_data++));
        ucCRCLo = (unsigned char)(ucCRCHi ^ CRC16Hi[iIndex]);
        ucCRCHi = CRC16Lo[iIndex];
    }
    return (uint16_t)(ucCRCHi << 8 | ucCRCLo);
}

// @brief 将整个文件加载到内存中
char* load_file_to_memory(const char* p_file_path, int* p_file_size)
{
    struct stat sb;
    if (p_file_size) {
        *p_file_size = 0;
    }
    if (-1 == stat(p_file_path, &sb) ) {
        DEBUG_INFO("stat file failed.");
        return NULL;
    }
    int flag = 0;
    char* p_buff = NULL;
    p_buff = (char*)app_malloc(sb.st_size + 1);
    assert(p_buff);
    memset(p_buff, 0, sb.st_size + 1);
    FILE *fd = fopen(p_file_path, "rb");
    if (NULL == fd) {
        DEBUG_INFO("Open %s Failed.", p_file_path);
        if (p_buff) app_free(p_buff), p_buff = NULL;
        return NULL;
    }
    int read_len = 0;  
    int cnt = sb.st_size / 512;
    if (sb.st_size % 512) {
        cnt++; 
        flag = 1;
    }
    read_len = fread(p_buff, 512, cnt, fd);
    if (1 == flag) ++read_len;
    if (read_len != cnt) {
        DEBUG_INFO("fread to file failed. read_len=%d, cnt=%d", read_len, cnt);
        if (p_buff) app_free(p_buff), p_buff = NULL;
        fclose(fd);
        return NULL;
    }
    fclose(fd);
    if (p_file_size) {
        *p_file_size = sb.st_size;
    }
    return p_buff;
}

// @brief 将数据加载到文件中
void load_memory_to_file(const char* p_buff, uint32_t buff_len, const char* p_filepath)
{
    int32_t ret;
    FILE *p_wfile = fopen(p_filepath, "wb");
    if (NULL == p_wfile) {
        DEBUG_INFO("Open %s Failed.", p_filepath);
        return ;
    }
    ret = fwrite(p_buff, buff_len, 1, p_wfile);
    if (ret != 1) {
        DEBUG_INFO("file write failed. ret=%d, buff_len=%d, p_file=%s", ret, buff_len, p_filepath);
    }
    fclose(p_wfile);
}

uint32_t crc32_ext(uint8_t const * p_data, uint32_t data_len, uint32_t crc)
{
    while (data_len--) {
        crc = crc ^ p_data[0];
        p_data++;
        for (uint32_t j=8; j>0; j--) {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}


//////////////////////////////////////////////////////////////////////
// @brief 加载json文件
ido_cJSON* app_custom_dial_config_file_parser(const char* p_path)
{
    char fpath[1024];
    ido_cJSON* p_json = NULL;
    char* p_json_data;
    snprintf(fpath, sizeof(fpath), "%s/dial_config.json", p_path);
    p_json_data = load_file_to_memory(fpath, NULL);
    if (p_json_data == NULL) {
        DEBUG_INFO("load file failed. ");
        return NULL;
    }
    p_json = ido_cJSON_Parse(p_json_data);
    if (p_json == NULL) {
        DEBUG_INFO("json parse failed. [%s]", ido_cJSON_GetErrorPtr());
    } 
    app_free(p_json_data);
    return p_json;
}

// @brief 拷贝缩略图文件到对应的目录下
static void copy_preview_file(ido_gui_file_widget_t* p_widget, const char* p_path, const char* p_dial_path, const char* filename, const char* dial_name)
{
    char fpath[1024];
    int32_t file_size = 0;
    char* p_file_data;
    
    snprintf(fpath, sizeof(fpath), "%s/%s", p_path, filename);
    p_file_data = load_file_to_memory(fpath, &file_size);
    if (p_file_data == NULL) {
        DEBUG_INFO("read file failed. fpath=%s", fpath);
        return;
    }
    p_widget->img_info.data_size = file_size;
    p_widget->img_info.checksum = crc32_ext(p_file_data, file_size, 0xffffffff);
    
    snprintf(fpath, sizeof(fpath), "%s/%s/watchface/installer/%s_tn.bin", p_path, p_dial_path, dial_name);
    remove(fpath);
    load_memory_to_file(p_file_data, file_size, fpath);
    app_free(p_file_data);
}

// @brief 拷贝图片文件到对应的目录下
static void copy_image_file(ido_gui_file_widget_t* p_widget, const char* p_path, const char* p_dial_path, const char* filename, const char* dial_name, file_list_t* p_delete_list)
{
    char fpath[1024];
    int32_t file_size = 0;
    char* p_file_data;
    
    snprintf(fpath, sizeof(fpath), "%s/%s", p_path, filename);
    p_file_data = load_file_to_memory(fpath, &file_size);
    if (p_file_data == NULL) {
        DEBUG_INFO("read file failed. fpath=%s", fpath);
        return;
    }
    p_widget->img_info.data_size = file_size;
    p_widget->img_info.checksum = crc32_ext(p_file_data, file_size, 0xffffffff);
    snprintf(fpath, sizeof(fpath), "%s/%s/watchface/%s/ezip/%s_img_%d.bin", p_path, p_dial_path, dial_name, dial_name, p_widget->img_info.identify);

    load_memory_to_file(p_file_data, file_size, fpath);
    app_free(p_file_data);

    // 添加到删除队列
    file_node_t* p_node = (file_node_t*)app_malloc(sizeof(file_node_t));
    assert(p_node);
    memset(p_node, 0, sizeof(file_node_t));
    ido_list_head_init(&p_node->node);
    strncpy(p_node->filename, fpath, sizeof(p_node->filename));
    ido_list_insert_before(&p_node->node, &p_delete_list->head);
    p_delete_list->file_count++;
}

static void copy_video_file(ido_gui_file_widget_t* p_widget, const char* p_path, const char* p_dial_path, const char* filename, const char* dial_name, file_list_t* p_delete_list)
{
    char fpath[1024] = {0};
    int32_t file_size = 0;
    char* p_file_data;
    
    snprintf(fpath, sizeof(fpath), "%s/%s", p_path, filename);
    p_file_data = load_file_to_memory(fpath, &file_size);
    if (p_file_data == NULL) {
        DEBUG_INFO("read file failed. fpath=%s", fpath);
        return;
    }
    p_widget->img_info.data_size = file_size;
    p_widget->img_info.checksum = crc32_ext(p_file_data, file_size, 0xffffffff);
    snprintf(fpath, sizeof(fpath), "%s/%s/watchface/%s/h264/%s_video_%d.h264", p_path, p_dial_path, dial_name, dial_name, p_widget->img_info.identify);

    load_memory_to_file(p_file_data, file_size, fpath);
    app_free(p_file_data);

    // 添加到删除队列
    file_node_t* p_node = (file_node_t*)app_malloc(sizeof(file_node_t));
    assert(p_node);
    memset(p_node, 0, sizeof(file_node_t));
    ido_list_head_init(&p_node->node);
    strncpy(p_node->filename, fpath, sizeof(p_node->filename));
    ido_list_insert_before(&p_node->node, &p_delete_list->head);
    p_delete_list->file_count++;
}

// @brief 删除早期版本遗存下来的文件(%s_img_*.bin)
static void app_custom_dial_delete_invalid_files(const char* p_path, const char* p_dial_path, const char* p_dial_name)
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buff;
    char file_path[1024];
    char prefix[256];
    uint32_t prefix_len, file_path_len;
    char* p_number, *p_end;
    char byte;

    prefix_len = snprintf(prefix, sizeof(prefix), "%s_img_", p_dial_name);
    file_path_len = snprintf(file_path, sizeof(file_path), "%s/%s/watchface/%s/ezip/", p_path, p_dial_path, p_dial_name);
    dir = opendir(file_path);
    if (NULL == dir) {
        DEBUG_INFO("open dir failed. p_path=%s\r\n", file_path);
        return;
    }
    
    while (1) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }
        strncpy(&file_path[file_path_len], entry->d_name, sizeof(file_path) - file_path_len - 1);
        if (stat(file_path, &stat_buff) == -1) {
            continue;
        }
        if (S_ISDIR(stat_buff.st_mode)) {               // 文件夹
            continue;
        } else if (S_ISREG(stat_buff.st_mode)) {        // 文件
            if (memcmp(entry->d_name, prefix, prefix_len) == 0) {
                p_end = entry->d_name + strlen(entry->d_name) - 4;
                if (memcmp(p_end, ".bin", 4) == 0) {
                    p_number = entry->d_name + prefix_len;
                    while (p_number < p_end) {
                        byte = *p_number++;
                        if (byte < '0' || byte > '9') {
                            break;
                        }
                    }
                    if (p_number == p_end) {
                        DEBUG_INFO("delete file: %s", file_path);
                        remove(file_path);
                    }
                }
            }
        }
    }
    closedir(dir);
}


// @brief 创建配置文件
int32_t app_custom_dial_make_config_file(const char* p_path, int* p_pid, char* p_dial_path, char* p_dial_name, char* p_postfix, file_list_t* p_delete_list)
{
    ido_gui_file_bin_head_t head;
    ido_gui_file_widget_t* p_widget_info;
    ido_cJSON *p_widget;
    ido_cJSON* root = app_custom_dial_config_file_parser(p_path);
    if (root == NULL) {
        DEBUG_INFO("parser dial config file failed.");
        return -2;
    }
    
    memset(&head, 0, sizeof(ido_gui_file_bin_head_t));
    int project_id = ido_cJSON_GetObjectItem(root, "project")->valueint;
    head.magic = IDO_GUI_FILE_MAGIC_VAL;
    head.flag = 0xffffffff;
    head.version = ido_cJSON_GetObjectItem(root, "version")->valueint;
    head.id = ido_cJSON_GetObjectItem(root, "id")->valueint;
    DEBUG_INFO("project_id=%d, version=%d, dial_id=%d", project_id, head.version, head.id);
    if (p_pid) {
        *p_pid = project_id;
    }
    const char* dial_name;
    p_widget = ido_cJSON_GetObjectItem(root, "name");
    if (p_widget == NULL) {
        DEBUG_INFO("parser dial name failed.");
        ido_cJSON_Delete(root);
        return -3;
    }
    dial_name = p_widget->valuestring;
    if (p_dial_name) {
        strcpy(p_dial_name, dial_name);
    }
    const char* dial_path = "dyn/dynamic_app";
    p_widget = ido_cJSON_GetObjectItem(root, "dial_path");
    if (p_widget) {
        dial_path = p_widget->valuestring;
    }
    if (p_dial_path) {
        strcpy(p_dial_path, dial_path);
    }
    const char* postfix = "watch";
    p_widget = ido_cJSON_GetObjectItem(root, "postfix");
    if (p_widget) {
        postfix = p_widget->valuestring;
    }
    if (p_postfix) {
        strcpy(p_postfix, postfix);
    }

    // 删除早期遗漏的文件
    app_custom_dial_delete_invalid_files(p_path, dial_path, dial_name);
    
    char config_file[1024];
    snprintf(config_file, sizeof(config_file), "%s/%s/watchface/installer/%s.dat", p_path, dial_path, p_dial_name);
    p_widget = ido_cJSON_GetObjectItem(root, "dial_data");
    head.widget_count = ido_cJSON_GetArraySize(p_widget);
    if (head.widget_count == 0) {
        ido_cJSON_Delete(root);
        load_memory_to_file((uint8_t*)&head, sizeof(ido_gui_file_bin_head_t), config_file);
        return -1;
    }

    uint32_t user_flag = 0;
    char* p_buff;
    const char* p_filename;
    ido_cJSON *p_content;
    p_buff = (char*)app_malloc(sizeof(ido_gui_file_bin_head_t) + sizeof(ido_gui_file_widget_t) * head.widget_count);
    assert(p_buff);
    memset(p_buff, 0, sizeof(ido_gui_file_bin_head_t) + sizeof(ido_gui_file_widget_t) * head.widget_count);
    p_widget_info = (ido_gui_file_widget_t*)(p_buff + sizeof(ido_gui_file_bin_head_t));
    for (int i=0; i<head.widget_count; ++i) {
        p_content = ido_cJSON_GetArrayItem(p_widget, i);
        user_flag = ido_cJSON_GetObjectItem(p_content, "user")->valueint;
        p_widget_info[i].wtype = ido_cJSON_GetObjectItem(p_content, "wtype")->valueint;
        p_widget_info[i].xdata = ido_cJSON_GetObjectItem(p_content, "xdata")->valueint;
        p_widget_info[i].dtype = ido_cJSON_GetObjectItem(p_content, "dtype")->valueint;
        p_widget_info[i].x = ido_cJSON_GetObjectItem(p_content, "x")->valueint;
        p_widget_info[i].y = ido_cJSON_GetObjectItem(p_content, "y")->valueint;
        if (user_flag == 0) {
            p_widget_info[i].wtype = 0;
        }
        switch (p_widget_info[i].wtype) {
            case IDO_FILE_WIDGET_TYPE_PREVIEW: {        // 缩略图
                p_widget_info[i].img_info.identify = i;
                p_filename = ido_cJSON_GetObjectItem(p_content, "file")->valuestring;
                copy_preview_file(&p_widget_info[i], p_path, dial_path, p_filename, dial_name);
                break;
            }
            case IDO_FILE_WIDGET_TYPE_IMAGE:
            case IDO_FILE_WIDGET_TYPE_IMAGE_SET:
            case IDO_FILE_WIDGET_TYPE_NUM_IMAGE_SET:
            case IDO_FILE_WIDGET_TYPE_IMAGE_SET_EX:
            case IDO_FILE_WIDGET_TYPE_NUM_IMAGE_SET_EX:{
                p_widget_info[i].img_info.identify = identify_start_number + i;
                p_filename = ido_cJSON_GetObjectItem(p_content, "file")->valuestring;
                copy_image_file(&p_widget_info[i], p_path, dial_path, p_filename, dial_name, p_delete_list);
                break;
            }
            case IDO_FILE_WIDGET_TYPE_UTF8_DATA: {
                p_widget_info[i].utf8_info.size = ido_cJSON_GetObjectItem(p_content, "font_size")->valueint;
                p_widget_info[i].utf8_info.r = ido_cJSON_GetObjectItem(p_content, "r")->valueint;
                p_widget_info[i].utf8_info.g = ido_cJSON_GetObjectItem(p_content, "g")->valueint;
                p_widget_info[i].utf8_info.b = ido_cJSON_GetObjectItem(p_content, "b")->valueint;
                break;
            }
            case IDO_FILE_WIDGET_TYPE_VIDEO:
            {
                p_widget_info[i].img_info.identify = identify_start_number + i;
                p_filename = ido_cJSON_GetObjectItem(p_content, "file")->valuestring;
                copy_video_file(&p_widget_info[i], p_path, dial_path, p_filename, dial_name, p_delete_list);
                break;
            }
                
            default: {
                DEBUG_INFO("unknow widget type. %d", p_widget_info[i].wtype);
                break;
            }
        }
        if (p_widget_info[i].wtype != IDO_FILE_WIDGET_TYPE_IMAGE_SET_EX && p_widget_info[i].wtype != IDO_FILE_WIDGET_TYPE_NUM_IMAGE_SET_EX) {
            head.count++;
        }
    }
    ido_cJSON_Delete(root);
    
    head.size = sizeof(ido_gui_file_widget_t) * head.widget_count;
    head.checksum = crc32_ext((uint8_t*)p_widget_info, head.size, 0xffffffff);
    memcpy(p_buff, &head, sizeof(ido_gui_file_bin_head_t));
    load_memory_to_file(p_buff, sizeof(ido_gui_file_bin_head_t) + head.size, config_file);
    app_free(p_buff);
    return head.size == 0 ? -1 : 0;
}



//////////////////////////////////////////////////////////////////////
#if 0
// @brief 按照文件名大小排序
void file_list_insert(file_list_t* p_list, file_node_t* p_new_node)
{
    ido_list_node_t *p_pos;
    file_node_t *p_node;
    uint32_t filename_len = strlen(p_new_node->filename);
    ido_list_for_each(p_pos, &p_list->head) {
        p_node = ido_list_entry(p_pos, file_node_t, node);
        if (strncmp(p_node->filename, p_new_node->filename, filename_len) <= 0) {
            DEBUG_INFO("insert: %s->%s", p_new_node->filename, p_node->filename);
            ido_list_insert_before(&p_new_node->node, p_pos);
            return;
        }
    }
    ido_list_insert_after(&p_new_node->node, &p_list->head);
    DEBUG_INFO("append[%d]. %s", filename_len, p_new_node->filename);
}
#endif


// @brief 获取文件
void custom_dial_file_find(const char* p_path, file_list_t* p_list)
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buff;
    char file_path[1024];

    dir = opendir(p_path);
    if (NULL == dir) {
        DEBUG_INFO("open dir failed. p_path=%s", p_path);
        return;
    }
    
    while (1) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }
        snprintf(file_path, sizeof(file_path), "%s/%s", p_path, entry->d_name);
        if (stat(file_path, &stat_buff) == -1) {
            continue;
        }
        if (S_ISDIR(stat_buff.st_mode)) {               // 文件夹
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            custom_dial_file_find(file_path, p_list);
        } else if (S_ISREG(stat_buff.st_mode)) {        // 文件
            file_node_t* p_node = (file_node_t*)app_malloc(sizeof(file_node_t));
            assert(p_node);
            memset(p_node, 0, sizeof(file_node_t));
            ido_list_head_init(&p_node->node);
            strncpy(p_node->filename, file_path, sizeof(p_node->filename));
            DEBUG_INFO("file=%s", p_node->filename);
            //file_list_insert(p_list, p_node);
            ido_list_insert_before(&p_node->node, &p_list->head);
            p_list->file_count++;
        }
    }
    closedir(dir);
}

// @brief 获取所需要的文件大小
uint32_t custom_dail_file_size_get(file_list_t* p_list, uint32_t* gdata_size, uint32_t path_len)
{
    struct stat sb;
    uint32_t filedata_size = 0;
    uint32_t fsize = 32;
    uint32_t gdsize = 0;
    file_node_t* p_node;
    ido_list_node_t *p_pos, *p_temp;
    ido_list_for_each_safe(p_pos, p_temp, &p_list->head) {
        p_node = ido_list_entry(p_pos, file_node_t, node);
        gdsize += strlen(p_node->filename) + 1 - path_len;
        if (-1 == stat(p_node->filename, &sb) ) {
            DEBUG_INFO("stat file failed. file=%s", p_node->filename);
            continue;
        }
        filedata_size += sb.st_size;
    }
    if (gdata_size) {
        *gdata_size = gdsize;
    }
    fsize += gdsize + sizeof(file_head_t) * p_list->file_count;
    fsize += filedata_size;
    return fsize;
}

// @brief 打包文件
char* file_head_pack(const char* p_filepath, file_head_t* p_hdr, uint32_t name_offset, uint32_t data_offset)
{
    int32_t file_size = 0;
    char* p_filedata;

    p_filedata = load_file_to_memory(p_filepath, &file_size);
    if (NULL == p_filedata) {
        return NULL;
    }
    p_hdr->name_offset = name_offset;
    p_hdr->size = file_size;
    p_hdr->checksum = crc16_x16x15x21((uint8_t*)p_filedata, file_size, 0);
    p_hdr->reserve = 0;
    p_hdr->offset = data_offset + sizeof(head_t); 
    DEBUG_INFO("filepath=%s, filesize=%d, name_offset=%d", p_filepath, p_hdr->size, name_offset);
    return p_filedata;
}

// @brief 生成在线表盘
// @return  -1: 没有控件
//          -2: json文件加载失败
int32_t ido_app_custom_dial_make(const char* p_path)
{
    int32_t ret;
    int32_t project_id = 0;
    char dial_path[128];
    char dial_name[64];
    char postfix[32];
    char filepath[512];
    file_list_t file_list;
    file_list_t delete_file_list;
    uint32_t path_len;

    ido_cJSON_Hooks hook = {
        .malloc_fn = app_malloc,
        .free_fn = app_free,
    };
    ido_cJSON_InitHooks(&hook);

    srand(time(NULL));
    identify_start_number = rand() % 100000000;

    memset(&delete_file_list, 0, sizeof(file_list_t));
    ido_list_head_init(&delete_file_list.head);
    memset(dial_name, 0, sizeof(dial_name));
    ret = app_custom_dial_make_config_file(p_path, &project_id, dial_path, dial_name, postfix, &delete_file_list);
    if (ret != 0) {
        return ret;
    }
    
    // 删除最终生成的文件
    snprintf(filepath, sizeof(filepath), "%s/%s.%s", p_path, dial_name, postfix);
    remove(filepath);

    // 将 p_path/dyn目录下的文件全部进行打包
    memset(&file_list, 0, sizeof(file_list_t));
    ido_list_head_init(&file_list.head);
    snprintf(filepath, sizeof(filepath), "%s/%s", p_path, dial_path);
    custom_dial_file_find(filepath, &file_list);
    path_len = strlen(p_path);

    // 计算所需要的内存大小
    uint32_t data_size = 0, gdata_size = 0;
    data_size = custom_dail_file_size_get(&file_list, &gdata_size, path_len);
    char* p_data = (char*)app_malloc(data_size + 4096);
    assert(p_data);
    memset(p_data, 0, data_size);
    
    head_t* p_head = (head_t*)p_data;
    char* p_filedata;
    char* p_gdata = p_data + sizeof(head_t);
    file_head_t* p_hdr = (file_head_t*)(p_gdata + gdata_size);
    char* p_fdata = p_gdata + gdata_size + sizeof(file_head_t) * file_list.file_count;
    DEBUG_INFO("data_size=%d, gdata_size=%d, file_count=%d, path_len=%d", data_size, gdata_size, file_list.file_count, path_len);
    
    gdata_size = 0;
    uint32_t fdata_offset = 0, filename_len;
    file_node_t* p_node;
    ido_list_node_t *p_pos, *p_temp;
    p_head->file_count = file_list.file_count;
    ido_list_for_each_safe(p_pos, p_temp, &file_list.head) {
        p_node = ido_list_entry(p_pos, file_node_t, node);
        p_filedata = file_head_pack(p_node->filename, p_hdr, gdata_size, fdata_offset);
        if (p_filedata) {
            filename_len = strlen(p_node->filename) + 1 - path_len;
            memcpy(&p_gdata[gdata_size], p_node->filename + path_len, filename_len);
            gdata_size += filename_len;
            memcpy(&p_fdata[fdata_offset], p_filedata, p_hdr->size);
            fdata_offset += p_hdr->size;
        }
        DEBUG_INFO("gdata_size=%d, fdata_offset=%u, size=%u", gdata_size, fdata_offset, p_hdr->size);
        p_hdr++;
        
        // 将节点移除掉
        ido_list_del(p_pos);
        file_list.file_count--;
        app_free(p_node);
        app_free(p_filedata);
    }
    p_head->magic = 0xa1b2c8d9;
    p_head->size = sizeof(head_t);
    p_head->project_no = project_id;
    p_head->data_size = (p_fdata - p_gdata) + fdata_offset;
    p_head->checksum = crc16_x16x15x21(p_gdata, p_head->data_size, 0);
    p_head->name_size = gdata_size; 
    
    // 保存到文件中
    //filename_len = crc32_ext(p_data, p_head->data_size+p_head->size, 0xffffffff);
    snprintf(filepath, sizeof(filepath), "%s/%s.%s", p_path, dial_name, postfix);
    load_memory_to_file(p_data, p_head->data_size+p_head->size, filepath);
    app_free(p_data);

    // 清除产生的文件
    ido_list_for_each_safe(p_pos, p_temp, &delete_file_list.head) {
        p_node = ido_list_entry(p_pos, file_node_t, node);
        remove(p_node->filename);
        DEBUG_INFO("delete file: %s", p_node->filename);
        ido_list_del(p_pos);
        file_list.file_count--;
        app_free(p_node);
    }
    return 0;
}


// @param 文件夹路径
int main(int argc, char** argv)
{
//    if (argc != 2) {
//        DEBUG_INFO("usage: %s [dial_dir]", argv[0]);
//        return -1;
//    }
   
//    ido_app_custom_dial_make(argv[1]);
   ido_app_custom_dial_make("custom1");
   getchar();
   //DEBUG_INFO("malloc_count=%d", malloc_count);
   return 0;
}


/**
 * @brief 获取思澈表盘(.watch)文件占用空间大小，计算规则：
 * nor方案：对表盘所有文件以4096向上取整  -98平台对应的项目，IDW27,205G Pro,IDW28,IDS05，DR03等
 * nand方案：对表盘所有文件以2048向上取整 -99平台对应的项目，GTX12,GTX13,GTR1,TIT21
 * @param path .watch文件路径，包含文件名
 * @param platform 平台类型，目前有98(nor)，99(nand)平台
 * @return size 文件占用磁盘的实际大小，-1:失败，文件路径访问失败，-2:失败，申请内存失败，-3:失败，读取文件失败，-4:失败，输入平台类型不支持
 * */
int32_t get_sifli_watch_file_size(const char *path,int platform)
{
    uint32_t block_size_base = 0;
    if (platform != 98 && platform != 99){
        LOG_INFO("warn:input platform type invailed(%d),only support 98/99",platform);
        return -4;
    }else
        block_size_base = platform==98?4096:2048;

    FILE *fp = fopen(path,"r");
    if (fp == NULL)
    {
        LOG_INFO("err:fopen %s failed,return -1",path);
        return -1;
    }
    fseek(fp,0,SEEK_END);
    int file_size = (int32_t)ftell(fp);
    fseek(fp,0,SEEK_SET);

    uint8_t *buff = (uint8_t *)malloc(file_size);
    if (buff == NULL)
    {
        LOG_INFO("err:malloc failed,return -2");
        fclose(fp);
        return -2;
    }
    memset(buff,0,file_size);
    size_t cout = fread(buff,1,file_size,fp);
    if (cout!=file_size){
        LOG_INFO("err:fread failed,return -3");
        free(buff);
        buff=NULL;
        fclose(fp);
        return -3;
    }
    fclose(fp);

    head_t head;
    memcpy(&head,buff,sizeof(head_t));
    LOG_INFO(".watch file %s size:%d,file cout:%d,file name size:%d,block size base:%d",path,file_size,head.file_count,head.name_size,block_size_base);
    uint32_t file_head_offset = head.size + head.name_size;

    uint32_t dst_file_size=0,dst_total_file_size=0;

    for (int i = 0; i < head.file_count; i++)
    {
        file_head_t file_head;
        memcpy(&file_head,buff+file_head_offset,sizeof(file_head_t));
        if (file_head.size<block_size_base)
            dst_file_size = block_size_base;
        else
            dst_file_size = file_head.size%block_size_base!=0?((file_head.size/block_size_base)+1)*block_size_base:file_head.size/block_size_base;
        dst_total_file_size += dst_file_size;
//        DEBUG_INFO("file[%d] file size:%d,total file size:%d,src file size:%d",i,dst_file_size,dst_total_file_size,file_head.size);
        file_head_offset += sizeof(file_head_t);
    }
    LOG_INFO("get .watch file %s dst file size:%d",path,dst_total_file_size);
    free(buff);
    buff=NULL;

    return (int32_t)dst_total_file_size;
}