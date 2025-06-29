/**
 * ==================================================
 *  @file mmap_test.cpp
 *  @brief mmap测试
 *  @author ywj
 *  @date 2025-06-30 01:05
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

// 每行打印 8 字节
void hex_dump(const char *prefix, const unsigned char *data, size_t size) {
    printf("%s:\n", prefix);
    for (size_t i = 0; i < size; i += 8) {
        printf("0x%04lx: ", (unsigned long)i);
        for (int j = 0; j < 8 && i + j < size; ++j) {
            printf("%02x ", data[i + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = sb.st_size;

    // mmap 文件到内存
    unsigned char *mapped = (unsigned char *) mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // 第一次 dump
    hex_dump("Before modification", mapped, file_size);

    // 修改前两个字节
    if (file_size >= 2) {
        mapped[0] = 'a';
        mapped[1] = 'b';
        printf("Modified first two bytes to: 0x%02x 0x%02x\n\n", mapped[0], mapped[1]);

        // 可选：同步内存修改回磁盘
        if (msync(mapped, file_size, MS_SYNC) == -1) {
            perror("msync");
        }
    } else {
        printf("File too small to modify first two bytes.\n");
    }

    // 第二次 dump
    hex_dump("After modification", mapped, file_size);

    // 清理
    if (munmap(mapped, file_size) == -1) {
        perror("munmap");
    }

    close(fd);
    return 0;
}