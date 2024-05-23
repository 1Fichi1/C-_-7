// Определение WIN32_LEAN_AND_MEAN минимизирует количество загружаемых заголовочных файлов Windows
#define WIN32_LEAN_AND_MEAN

// Подключаем необходимые заголовочные файлы
#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

// Используем пространство имен std для упрощения работы с потоками ввода-вывода
using namespace std;

int main() {
    WSADATA wsaData;  // Структура для хранения информации о Windows Sockets
    ADDRINFO hints;  // Структура для указания критериев для функции getaddrinfo
    ADDRINFO* addrResult = nullptr;  // Указатель для хранения результатов функции getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET;  // Сокет для прослушивания входящих соединений
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для установления соединения с клиентом
    char recvBuffer[512];  // Буфер для хранения полученных данных

    const char* sendBuffer = "Hello from server";  // Сообщение для отправки клиенту

    // Инициализация библиотеки Winsock версии 2.2
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Очистка структуры hints и заполнение ее полей
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;  // Указываем, что будем использовать IPv4
    hints.ai_socktype = SOCK_STREAM;  // Указываем, что будем использовать потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP;  // Указываем, что будем использовать протокол TCP
    hints.ai_flags = AI_PASSIVE;  // Указываем, что сокет будет использоваться для прослушивания

    // Получаем адресную информацию для создания и привязки сокета
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Создаем сокет для прослушивания входящих соединений
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Привязываем сокет к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Начинаем прослушивание входящих соединений
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Принимаем входящее соединение
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Закрываем сокет прослушивания, так как он больше не нужен
    closesocket(ListenSocket);

    // Получаем и отправляем данные в цикле
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера перед приемом данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получаем данные от клиента
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправляем данные обратно клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);  // Закрываем сокет
                freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
                WSACleanup();  // Завершаем работу с Winsock
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;  // Соединение закрывается
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;  // Произошла ошибка при получении данных
            closesocket(ConnectSocket);  // Закрываем сокет
            freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
            WSACleanup();  // Завершаем работу с Winsock
            return 1;
        }
    } while (result > 0);  // Продолжаем, пока есть данные для получения

    // Завершаем соединение
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Закрываем сокет соединения и освобождаем ресурсы
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
    WSACleanup();  // Завершаем работу с Winsock
    return 0;  // Завершаем выполнение программы
}
