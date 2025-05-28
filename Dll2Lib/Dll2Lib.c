/*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // isatty()

int main(int argc, char* argv[]) {
    FILE *input;

    if (argc >= 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Error opening input");
            return 1;
        }
    } else {
        if (isatty(fileno(stdin))) {
            printf("Usage: %s <input>|pipe", argv[0]);
            return 1;
        } else {
            input = stdin;
        }
    }

    char line[256];
    char dll_name[100] = {0};
    int ordinal_base = 1;
    int export_start = 0;

    // 创建 .def 文件
    FILE *def = fopen("tmp.def", "w");
    if (!def) {
        perror("Error creating .def file");
        fclose(input);
        return 1;
    }
    fprintf(def, "EXPORTS\n");

    // 解析 objdump 输出
    while (fgets(line, sizeof(line), input)) {
        // 提取 DLL 名称
        if (strstr(line, "DLL name:")) {
            sscanf(line, " DLL name: %s", dll_name);
            char* s = strrchr(dll_name, '.');
            if (s) *s = '\0';
        }
        // 检测导出表开始
        else if (strstr(line, "Ordinal") && strstr(line, "RVA") && strstr(line, "Name")) {
            export_start = 1;
            fgets(line, sizeof(line), input); // 跳过表头分隔行
        }
        // 解析导出条目
        else if (export_start) {
            int ordinal;
            unsigned rva;
            char name[100] = {0};
            if (sscanf(line, "%d %x %s", &ordinal, &rva, name) >= 2) {
                if (strlen(name) > 0) {  // 只处理有名称的导出
                    fprintf(def, "    %s @%d\n", name, ordinal);
                }
            }
        }
    }

    fclose(input);
    fclose(def);

    // 调用 dlltool 生成 .a 文件
    char command[256];
    snprintf(command, sizeof(command), 
             "dlltool -d tmp.def -l lib%s.a -D %s",
             dll_name, dll_name);
    system(command);

    snprintf(command, sizeof(command), "lib%s.def", dll_name);
    rename("tmp.def", command);

    printf("Generated lib%s.a from %s\n", dll_name, dll_name);
    return 0;
}