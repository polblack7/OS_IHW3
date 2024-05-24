# ИДЗ по ОС №3

### Исполнитель: Анненков Никита Игоревич, БПИ226.

### Вариант 33.
Задание: Реализовать клиент–серверное приложение, шифрующее
исходный текст (в качестве ключа используется кодовая
таблица, устанавливающая однозначное соответствие
между каждой буквой и каким–нибудь числом).

### Работа по требованиям выполнена на оценку 8.


## Сценарий решаемой задачи
### Исходные сущности и их поведение:

  Клиенты (шифровальщики): Клиенты получают кусочки текста, кодируют их и отправляют закодированные фрагменты обратно серверу.


  Сервер (портфель задач): Сервер распределяет текст на кусочки для клиентов, получает закодированные фрагменты и упорядочивает их для формирования окончательного закодированного текста.

  Монитор: Получает данные, которые закодированные отправляются на сервер.
  
### Взаимодействие:

  Сервер запускается и слушает на заданном IP-адресе и порту.
  
  Клиенты подключаются к серверу, получают кусочек текста для шифрования, кодируют его и отправляют обратно.
  
  Сервер собирает закодированные фрагменты и упорядочивает их для формирования окончательного текста.

  Мониторы отслеживают, что отправляется на сервер.
  
### Параметры командной строки:

  IP-адрес и порт сервера задаются через командную строку при запуске как сервера, так и клиентов.

### Работа в сетевом режиме:

  Приложение должно корректно работать как на одном компьютере, так и в распределенном режиме.


## Небольшая инструкция по запуску:

### 4-5:

  ./server 127.0.0.1 8080 2

  ./client 127.0.0.1 8080 0 "Hello"

  ./client 127.0.0.1 8080 1 "World"

  (пример входных данных, можно указать больше фрагментов и больше их передать)


### 6-7:

  ./server 127.0.0.1 8080 2

  ./monitor 127.0.0.1 8080

  ./client 127.0.0.1 8080 0 "Hello"

  ./client 127.0.0.1 8080 1 "World"

  (пример входных данных, можно указать больше фрагментов и больше их передать)


### 8:

  ./server 127.0.0.1 8080 2

  ./monitor 127.0.0.1 8080

  ./monitor 127.0.0.1 8080

  ./client 127.0.0.1 8080 0 "Hello"

  ./client 127.0.0.1 8080 1 "World"

  (пример входных данных, можно указать больше фрагментов и больше их передать, мониторов сколько угодно)
  



(Все исполняемые файлы уже собраны, но если есть желание можно их собрать заново, я использовал gcc)




#
### Теперь к отчету по каждой разбалловке:

## 4-5 баллов:

• Разработано клиент–серверное приложение, в котором сервер и клиенты независимо друг от друга отображают только ту информацию, которая поступает им во время обмена. 

• IP Адресса и порты задаются через командную строку.

• Приложение работает как на одном компьютере так и в распределенном режиме на нескольких
компьютерах.


## 6-7 баллов:

• В дополнение к программе на предыдущую оценку разработано отдельная клиентская программа, подключаемую к серверу, которая предназначена для отображение комплексной информации о выполнении приложения в целом(т.е монитор, как называется это у меня). 

• Монитор завершает свою работу вместе с сервером, только после того, как выведет все данные.

• Монитор отображает всю информацию, которую клиент передает на сервер.

• IP Адресса и порты задаются через командную строку.

• Приложение работает как на одном компьютере так и в распределенном режиме на нескольких
компьютерах.

## 8 баллов:

• В дополнение к предыдущей программе реализована возможность, подключения множества клиентов, обеспечивающих отображение информации о работе приложения(т.е множества мониторов).

• Мониторы завершают свою работу вместе с сервером, только после того, как выведут все данные.

• Мониторы отображает всю информацию, которую клиент передает на сервер.

• IP Адресса и порты задаются через командную строку.

• Приложение работает как на одном компьютере так и в распределенном режиме на нескольких
компьютерах.






