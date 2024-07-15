#include "stdio.h"
#include "stdint.h"
#include "time.h"
#include "stdlib.h"
#include "string.h"
#include <fcntl.h>
#include <unistd.h>

#include "MQTTClient.h"

#define CLIENTID    "ExampleClientPub"
#define TOPIC       "/node-red/temp"
#define PAYLOAD     "50"
#define QOS         1
#define TIMEOUT     10000L
double DELAY = 5;

struct sensor {
    uint16_t year;
    uint8_t month;
    uint16_t day;
    uint8_t hour;
    uint8_t minute;
    int8_t t;
};

void AddRecord (struct sensor* info, int number, 
uint16_t year,uint8_t month,uint16_t day,uint8_t hour,uint8_t minute,int8_t t)
{
    info[number].year = year;
    info[number].month = month;
    info[number].day = day;
    info[number].hour = hour;
    info[number].minute = minute;
    info[number].t = t;
}

void write_to_csv(int year, int month, int day, int hour, int minute, int temperature) {
    FILE *fp;
    char filename[] = "temp.csv";

    fp = fopen(filename, "a"); // Открытие файла для добавления данных
    if (fp == NULL) {
        perror("Ошибка открытия файла");
        return;
    }

    // Запись данных в файл в формате dddd;mm;dd;hh;mm;temperature
    fprintf(fp, "%04d;%02d;%02d;%02d;%02d;%d\n", year, month, day, hour, minute, temperature);

    fclose(fp);
}

int read_ds18b20_temperature() {
    const char *device_path = "/sys/bus/w1/devices/28-XXXXXXXXXXXX/w1_slave";// путь к файлу устройства DS18B20, 28-XXXXXXXXXXXX - идентификатор датчика
    char buf[256];
    char temp_str[6];
    int fd = open(device_path, O_RDONLY);

    if(fd == -1) {
        perror("Открытие устройства не удалось");
        return -1;
    }

    ssize_t num_read = read(fd, buf, sizeof(buf) - 1);
    if(num_read <= 0) {
        perror("Ошибка чтения");
        close(fd);
        return -1;
    }

    buf[num_read] = '\0';
    char *temp_ptr = strstr(buf, "t=");
    if(temp_ptr == NULL) {
        printf("Ошибка: температура не найдена\n");
        close(fd);
        return -1;
    }

    strncpy(temp_str, temp_ptr + 2, 5);
    temp_str[5] = '\0';
    int temperature = atoi(temp_str);

    close(fd);
    return temperature / 1000; // Возвращаем температуру в градусах Цельсия
}


int main(int argc, char* argv[])
{
    int temperature = read_ds18b20_temperature(); // Температура, полученная от датчика
    // int temperature = 25; // Пример температуры, полученной от датчика
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);

    int year = now_tm->tm_year + 1900;
    int month = now_tm->tm_mon + 1;
    int day = now_tm->tm_mday;
    int hour = now_tm->tm_hour;
    int minute = now_tm->tm_min;

    write_to_csv(year, month, day, hour, minute, temperature);    

    char address[50];
    char username[50];
    char password[50];

    printf("Введите IP-адрес сервера: ");
    scanf("%49s", address);
    printf("Введите имя пользователя: ");
    scanf("%49s", username);
    printf("Введите пароль: ");
    scanf("%49s", password);

    FILE *file;
    file = fopen("temp.csv","r");
    
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, address, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username;
    conn_opts.password = password;
    

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    
    struct sensor*info = malloc (365*24*60*sizeof(struct sensor));
    

        
        int Y,M,D,H,Min,T;
        int r;
        int count=0;
        
        for (;(r=fscanf(file,"%d;%d;%d;%d;%d;%d",&Y,&M,&D,&H,&Min,&T))>0;count++)
        {
            if (r<6)
            {
                char s[20], c;
                r = fscanf (file, "%[^\n]%c",s,&c);
                printf("Wrong format in line %s\n",s);
            }
            else
            {
                printf("%d %d %d %d %d %d\n",Y,M,D,H,Min,T);
                AddRecord(info,count,Y,M,D,H,Min,T);
            }
		}
        fclose(file);
        
        int i = 0;

    while(1)
    {

        
        clock_t begin = clock();       

        char str[255];
        sprintf(str,"%d",info[i++].t);
        printf("%s,%d\n",str,i);
        
        if(i>=count)
            i=0;
        
        
        pubmsg.payload = str;
        pubmsg.payloadlen = strlen(str);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
        //printf("Waiting for up to %d seconds for publication of %s\n"
        //        "on topic %s for client with ClientID: %s\n",
        //        (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        //printf("Message with delivery token %d delivered\n", token);
        
        while ((double)(clock() - begin)/CLOCKS_PER_SEC<DELAY)
        {}
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
