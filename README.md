# M2 Sistemas em Tempo Real

Integrantes: Cauã Domingos e Paulo Martino Hermans<br>
Data de entrega: 31/10/2024<br>

# Relatório de Desenvolvimento

O relatório completo do projeto está disponível em: https://docs.google.com/document/d/1xeaC7aTWnQs8WNJBr3bvLez9K6uB0nJ1/edit?usp=sharing&ouid=107006664367478057567&rtpof=true&sd=true

# Instruções de Compilação

A implementação foi feita utilizando a IDE Visual Studio Code. Também foi necessária a instalação de drivers USB-para-UART e neste caso, foi instalado o driver CP210x VCP para windows, este passo possibilita fazer o processo de flash com a placa ESP32.<br><br>
Após a instalação dos componentes necessários, abra o VSCode e instale as extenções C/C++ e ESP-IDF, clique em View, clique em Command Palette e procure por ESP-IDF: Show Examples Projects.<br><br>
Escolha um projeto exemplo, verifique se a porta de conexão da ESP, sua versão e seus drivers estão corretos, copie o código da aplicação para o arquivo main e clique em ESP-IDF: Build, Flash and Monitor.
