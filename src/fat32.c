#include <stdint.h>
#include <stdbool.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

static struct FAT32DriverState fat32driver_state = {0};

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};



// -- Misc Helper --
static inline int32_t ceil_div(int32_t a, int32_t b) {
    return a / b + (a % b != 0);
}

uint32_t cluster_to_lba(uint32_t cluster_number) {
    return cluster_number * CLUSTER_BLOCK_COUNT;
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
    dir_table->table[0].cluster_high   = 0xFFFF & (parent_dir_cluster >> 16);
    dir_table->table[0].cluster_low    = 0xFFFF & parent_dir_cluster;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[0].attribute      = ATTR_SUBDIRECTORY;
    memcpy(dir_table->table[0].name, name, 8);

    dir_table->table[1].cluster_high   = 0xFFFF & (parent_dir_cluster >> 16);
    dir_table->table[1].cluster_low    = 0xFFFF & parent_dir_cluster;
    dir_table->table[1].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[1].attribute      = ATTR_SUBDIRECTORY;
    memcpy(dir_table->table[1].name, "..\0\0\0\0\0", 8);
}



// -- File system initializer --
bool is_empty_storage(void) {
    struct BlockBuffer boot_sector;
    read_blocks(&boot_sector, 0, 1);
    return memcmp(&boot_sector, fs_signature, BLOCK_SIZE);
}

void create_fat32(void) {
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    // Reserved values
    fat32driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    fat32driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;

    // Root
    fat32driver_state.fat_table.cluster_map[ROOT_CLUSTER_NUMBER] = FAT32_FAT_END_OF_FILE;
    
    // Write new valid File Allocation Table
    write_clusters(&fat32driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

    // Write root directory table
    struct FAT32DirectoryTable root_dir_table = {0};
    init_directory_table(&root_dir_table, "root\0\0\0\0", ROOT_CLUSTER_NUMBER);
    write_clusters(&root_dir_table, ROOT_CLUSTER_NUMBER, 1);
}

void initialize_filesystem_fat32(void) {
    if (is_empty_storage())
        create_fat32();
    else
        read_clusters(&fat32driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
}



// -- CRUD Helper --
bool is_loaded_dir_table_valid(void) {
    return fat32driver_state.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY;
}

uint32_t get_cluster_from_entry(struct FAT32DirectoryEntry entry) {
    return entry.cluster_high << 16 | entry.cluster_low;
}

int32_t driver_dir_table_linear_scan(char name[8], char ext[3], bool find_empty) {
    for (uint32_t i = 0; i < DIRECTORY_TABLE_ENTRY_COUNT; i++) {
        struct FAT32DirectoryEntry entry = fat32driver_state.dir_table_buf.table[i];
        bool is_entry_not_empty          = (entry.user_attribute & UATTR_NOT_EMPTY);
        bool search_and_found_empty      = find_empty && !is_entry_not_empty;
        bool name_match                  = is_entry_not_empty && !memcmp(entry.name, name, 8) && !memcmp(entry.ext, ext, 3);
        if (search_and_found_empty || name_match)
            return i;
    }
    return -1;
}

int8_t driver_fat_mark_empty_cluster(uint32_t empty_buf[CLUSTER_MARK_MAX], uint32_t cluster_count) {
    uint32_t marked_cluster_count = 0;
    for (int i = 0; i < CLUSTER_MAP_SIZE && marked_cluster_count < CLUSTER_MARK_MAX; i++) {
        uint32_t cluster = fat32driver_state.fat_table.cluster_map[i];
        if (cluster == FAT32_FAT_EMPTY_ENTRY)
            empty_buf[marked_cluster_count++] = i;
    }

    if (marked_cluster_count < cluster_count)
        return -1;

    return 0;
}

bool is_dirtable_empty(struct FAT32DirectoryTable *dirtable) {
    for (uint32_t i = 2; i < DIRECTORY_TABLE_ENTRY_COUNT; i++) // Skipping index 0
        if (dirtable->table[i].user_attribute & UATTR_NOT_EMPTY)
            return false;
    return true;
}



// -- File system CRUD --
// Note : Not comprehensive edge case checking, just simple and quick implementation
int8_t read(struct FAT32DriverRequest request) {
    // Read directory at parent_cluster_number
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);

    if (!is_loaded_dir_table_valid())
        return -1; // Parent cluster number is not directory

    // Linear scan for entry in dir table
    int32_t entry_index = driver_dir_table_linear_scan(request.name, request.ext, false);

    if (entry_index == -1)
        return 3; // File not found

    struct FAT32DirectoryEntry entry = fat32driver_state.dir_table_buf.table[entry_index];
    if (entry.attribute & ATTR_SUBDIRECTORY)
        return 1; // Entry is a folder
    else if (entry.filesize > request.buffer_size)
        return 2; // Buffer is too small

    // Read file
    uint32_t cluster_iterator    = get_cluster_from_entry(entry);
    uint32_t cluster_read_offset = 0;
    do {
        read_clusters(request.buf + CLUSTER_SIZE*cluster_read_offset, cluster_iterator, 1);
        cluster_iterator = fat32driver_state.fat_table.cluster_map[cluster_iterator];       // Read next linked list cluster
        cluster_read_offset++;
    } while (cluster_iterator != FAT32_FAT_END_OF_FILE);

    return 0;
}

int8_t read_directory(struct FAT32DriverRequest request) {
    // Read directory at parent_cluster_number
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);

    if (!is_loaded_dir_table_valid())
        return -1; // Parent cluster number is not directory

    int32_t entry_index = driver_dir_table_linear_scan(request.name, "\0\0\0", false);

    if (entry_index == -1)
        return 2; // Directory not found

    struct FAT32DirectoryEntry entry = fat32driver_state.dir_table_buf.table[entry_index];
    if ((entry.attribute & ATTR_SUBDIRECTORY) == 0)
        return 1; // Not a directory

    // Read directory
    read_clusters(request.buf, get_cluster_from_entry(entry), 1);

    return 0;
}

int8_t write(struct FAT32DriverRequest request) {
    // Read directory at parent_cluster_number
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);

    if (!is_loaded_dir_table_valid())
        return -1; // Parent cluster number is not directory

    int32_t same_name_index   = driver_dir_table_linear_scan(request.name, request.ext, false);
    int32_t empty_entry_index = driver_dir_table_linear_scan(request.name, request.ext, true);

    if (same_name_index != -1)
        return 1; // Entry with same name already exist
    else if (empty_entry_index == -1)
        return -1; // No empty entry available
    
    // Scan and mark empty cluster
    uint32_t empty_clusters[CLUSTER_MARK_MAX] = {0};
    uint32_t cluster_count_to_reserve         = ceil_div(request.buffer_size, CLUSTER_SIZE);
    if (request.buffer_size == 0)
        cluster_count_to_reserve = 1;
    int8_t err_code = driver_fat_mark_empty_cluster(empty_clusters, cluster_count_to_reserve);
    
    if (err_code == -1)
        return -1; // Not enough empty cluster in FAT

    // Create new entry
    struct FAT32DirectoryEntry new_entry = {
        .filesize       = request.buffer_size,
        .user_attribute = 0 | UATTR_NOT_EMPTY,
    };
    memcpy(new_entry.name, request.name, 8);
    memcpy(new_entry.ext,  request.ext,  3);

    // Write actual data
    new_entry.cluster_high = (uint16_t) (empty_clusters[0] >> 16);
    new_entry.cluster_low  = empty_clusters[0] & 0xFFFF;
    if (request.buffer_size == 0) {
        struct FAT32DirectoryTable new_table = {0};
        init_directory_table(&new_table, request.name, request.parent_cluster_number);
        new_entry.attribute = 0 | ATTR_SUBDIRECTORY;

        fat32driver_state.fat_table.cluster_map[empty_clusters[0]] = FAT32_FAT_END_OF_FILE;
        write_clusters(new_table.table, empty_clusters[0], 1);
    } else {
        for (uint32_t i = 0; i < cluster_count_to_reserve; i++) {
            uint32_t cluster_number = empty_clusters[i];
            if (i == cluster_count_to_reserve - 1)
                fat32driver_state.fat_table.cluster_map[cluster_number] = FAT32_FAT_END_OF_FILE; // EOF
            else
                fat32driver_state.fat_table.cluster_map[cluster_number] = empty_clusters[i+1];   // Point into next cluster
            write_clusters(request.buf + CLUSTER_SIZE*i, cluster_number, 1);
        }
    }

    // Update file system metadata in storage
    fat32driver_state.dir_table_buf.table[empty_entry_index] = new_entry;                      // Insert new entry into dirtable
    write_clusters(fat32driver_state.dir_table_buf.table,   request.parent_cluster_number, 1); // Dirtable
    write_clusters(fat32driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);            // FAT

    return 0;
}

int8_t delete(struct FAT32DriverRequest request) {
    // Read directory at parent_cluster_number
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);

    if (!is_loaded_dir_table_valid())
        return -1; // Parent cluster number is not directory

    // Linear scan for entry in dir table
    int32_t entry_index = driver_dir_table_linear_scan(request.name, request.ext, false);

    if (entry_index == -1)
        return 1; // File not found

    struct FAT32DirectoryEntry entry = fat32driver_state.dir_table_buf.table[entry_index];
    if (entry.attribute & ATTR_SUBDIRECTORY) {
        struct FAT32DirectoryTable dirtable;
        read_clusters(&dirtable, get_cluster_from_entry(entry), 1);
        if (!is_dirtable_empty(&dirtable))
            return 2; // Folder is not empty
    }

    // Remove entry from parent directory
    fat32driver_state.dir_table_buf.table[entry_index].user_attribute = 0;
    memset(fat32driver_state.dir_table_buf.table[entry_index].name, 0, 8);
    memset(fat32driver_state.dir_table_buf.table[entry_index].ext, 0, 3);

    // Remove FAT cluster number
    uint32_t cluster_iterator = get_cluster_from_entry(entry);
    do {
        uint32_t next_iter = fat32driver_state.fat_table.cluster_map[cluster_iterator]; // Read & save next linked list cluster
        fat32driver_state.fat_table.cluster_map[cluster_iterator] = FAT32_FAT_EMPTY_ENTRY;
        cluster_iterator   = next_iter;
    } while (cluster_iterator != FAT32_FAT_END_OF_FILE);

    // Update file system metadata in storage
    write_clusters(fat32driver_state.dir_table_buf.table,   request.parent_cluster_number, 1); // Dirtable
    write_clusters(fat32driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);            // FAT

    return 0;
}