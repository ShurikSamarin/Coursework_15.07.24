#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Функция для чтения температуры с датчика DS18B20 1-Wire
float read_temperature() {
    FILE *fp;
    char path[100] = "/sys/bus/w1/devices/28-XXXXXXXXXXXX/w1_slave"; //28-XXXXXXXXXXXX - идентификатор датчика
    char buf[256];
    char *temp;
    float temperature = 0.0;

    // Открытие файла устройства датчика для чтения
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Ошибка открытия файла датчика");
        return -1;
    }

    // Чтение содержимого файла
    while (fgets(buf, sizeof(buf), fp)) {
        // Поиск строки, содержащей температуру
        temp = strstr(buf, "t=");
        if (temp) {
            // Преобразование строки с температурой в число
            temperature = strtof(temp + 2, NULL) / 1000.0;
            break;
        }
    }

    fclose(fp);
    return temperature;
}

void write_to_csv(float temperature) {
    FILE *fp;
    char filename[] = "temp.csv";
    time_t now;
    struct tm *now_tm;
    int year, month, day, hour, minute;

    now = time(NULL);
    now_tm = localtime(&now);

    year = now_tm->tm_year + 1900; // tm_year хранит количество лет с 1900
    month = now_tm->tm_mon + 1;    // tm_mon хранит месяц от 0 до 11
    day = now_tm->tm_mday;
    hour = now_tm->tm_hour;
    minute = now_tm->tm_min;

    fp = fopen(filename, "a"); // Открытие файла для добавления данных
    if (fp == NULL) {
        perror("Ошибка открытия файла");
        return;
    }

    // Запись данных в файл в формате dddd;mm;dd;hh;mm;temperature
    fprintf(fp, "%04d;%02d;%02d;%02d;%02d;%d\n", year, month, day, hour, minute, (int)temperature);

    fclose(fp);
}


int main() {
    float temperature = read_temperature();
    if (temperature != -1) {
        //printf("Температура: %.3f°C\n", temperature); // дебаг для вывода температуры на консоль
        write_to_csv(temperature);
    }
    return 0;
}
