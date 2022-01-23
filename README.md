# IRC
Internet Relay Chat 👨‍💻🌍👩‍💻

Chat for communication, between client's in home network.
## Установка и запуск
Прописать в коммадной строке
```
make
./ircserv <port> <password>
```
Сервер работает на всех доступных интерфейсах
## Использование

```
nc <host> <port>
```
Пользовательские команды:
* /PRIVMSG - sending a private message. 
  ```
  /PRIVMSG <UserNickName1,UserNickName2...> <any message>
  ```
* /NOTICE - sending a privat message, but without error response.
  ```
  /NOTICE <UserNickName1,UserNickName2...> <any message>
  ```
* /NICK - change of nickname. 
  ```
  /NICK <CookieMonster>
  ```
* /JOIN - change of channel. 
  ```
  /JOIN <any_channel>
  ```
* /LEAVE - leave channel.
  ```
  /LEAVE
  ```
* /QUIT - quit form server.
  ```
  /QUIT
  ```
* /WHO - finde User by pattern.
  ```
  /WHO <pattern>(optional)
  ```
* /LIST - displays all channels.
  ```
  /LIST
  ```
* /HELP - shows all server commands.
  ```
  /HELP
  ``` 
Операторские команды:
* /OPER - became operator.
  ```
  /OPER <password>
  ```
* /KICK - you can moderate all servers, and kick any user from channel.
  ```
  /KICK <Username>
  ```
* /KILL - you can remove the user from the network
  ```
  /KILL <Username>
  ```
* /DIE - you can shut down server
  ```
  /DIE
  ```
