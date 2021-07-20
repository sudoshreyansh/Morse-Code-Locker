# Morse-Code-Locker

A locker based on morse code for it's password, developed on ESP32 Development Board.

Many hotels provide lockers to their customers. But, in most cases, the master password is common for lockers in all rooms. In such scenario, if an untrusty employee gets to know the master password, it might be a security risk for the residents of the hotel. To solve this issue, the locker also comes with a web portal where the admin can reset the password of the locker of any room, without the use of any physical interaction or any master password. It also stores a logs of password resets.

<br>

## Circuit Diagram

![Circuit Diagram](/images/circuit.png "Circuit Diagram")
---

## Web Portal

![Web Portal](/images/mockup.png "Web Portal")
---

## Code Structure

The code is of 3 parts - the ESP32 code, the Admin Panel Web Server code and the IoT Server code.

### ESP32

The ESP32 code utilizes the WiFi features of the development board to connect to the IoT Server code using a custom protocol and log the status of the locker ( whether it is in reset state or in use ) to the IoT server. It utilizes FreeRTOS for multitasking and utilizing it's 2 cores. There are 2 push buttons for input and a status LED for output. One push button is used for entering the morse code and other as an "Enter" key.

## IoT Server

The IoT server uses a custom TCP protocol to receive the status log of the lockers and then sends it to the Admin Panel Server to display using HTTP. It also receives requests from the Admin Panel Web Server to reset a particular locker based on the ID of the locker.

## Admin Panel Web Server

The Admin Panel receives logs from and sends request for password reset to the IoT server. All password resets get logged.