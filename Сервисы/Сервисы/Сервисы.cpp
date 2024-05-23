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
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для установления соединения с сервером
    char recvBuffer[512];  // Буфер для хранения полученных данных

    const char* sendBuffer1 = "Hello from client 1";  // Первое сообщение для отправки серверу
    const char* sendBuffer2 = "Hello from client 2";  // Второе сообщение для отправки серверу

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

    // Получаем адресную информацию для соединения с сервером
    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Создаем сокет для соединения с сервером
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Подключаемся к серверу
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Unable to connect to server" << endl;
        closesocket(ConnectSocket);  // Закрываем сокет
        ConnectSocket = INVALID_SOCKET;  // Сбрасываем значение сокета
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Отправляем первое сообщение серверу
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;  // Выводим количество отправленных байт

    // Отправляем второе сообщение серверу
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;  // Выводим количество отправленных байт

    // Завершаем передачу данных
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);  // Закрываем сокет
        freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
        WSACleanup();  // Завершаем работу с Winsock
        return 1;
    }

    // Получаем данные от сервера в цикле
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера перед приемом данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получаем данные от сервера
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;
        }
        else if (result == 0) {
            cout << "Connection closed" << endl;  // Соединение закрыто сервером
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;  // Ошибка при получении данных
        }
    } while (result > 0);  // Продолжаем, пока есть данные для получения

    // Закрываем сокет и освобождаем ресурсы
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);  // Освобождаем память, выделенную для addrResult
    WSACleanup();  // Завершаем работу с Winsock
    return 0;  // Завершаем выполнение программы
}
