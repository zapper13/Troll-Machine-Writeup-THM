#include <stdio.h>
#include <stdlib.h>

int main() {

    char password[] = "1am0ffensive1099$$";



    // MySQL commands can be executed using system() calls
    char mysql_cmd[512];
    snprintf(mysql_cmd, sizeof(mysql_cmd), "mysql -u root -p%s -h localhost -e \"CREATE DATABASE IF NOT EXISTS test_db;\"", password);

    // Execute the command to create the database
    system(mysql_cmd);

    printf("[+] MySQL command executed to create database.\n");

    return 0;
}
