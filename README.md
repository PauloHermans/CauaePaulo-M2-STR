# Projeto 2 Sistemas em Tempo Real

Integrantes: Cauã Domingos e Paulo Martino Hermans<br>
Data de entrega: 19/09/2024<br>

# Relatório

O relatório completo do projeto está disponível em: https://docs.google.com/document/d/15oXDMCgYXUKa9fwezUOCS6JEPg2suemz/edit?usp=sharing&ouid=110783721327409540132&rtpof=true&sd=true

# Instruções de Compilação

A implementação foi feita utilizando a IDE Visual Studio Code. Também foi necessária a instalação de drivers USB-para-UART e neste caso, foi instalado o driver CP210x VCP para windows, este passo possibilita fazer o processo de flash com a placa ESP32.<br><br>
Após a instalação dos componentes necessários, abra o VSCode e instale as extenções C/C++ e ESP-IDF, clique em View, clique em Command Palette e procure por ESP-IDF: Show Examples Projects.<br><br>
Escolha um projeto exemplo, verifique a porta de conexão da ESP e seus drivers, copie o código da aplicação para o arquivo main e clique em ESP-IDF: Build, Flash and Monitor.
