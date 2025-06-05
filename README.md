🛠️ Sistema Físico de Monitoramento de Enchentes (Edge Computing & IoT)
📌 Descrição do Problema
Enchentes urbanas são um grave problema em diversas regiões, especialmente em áreas de risco e cidades com infraestrutura deficiente de drenagem. A ausência de monitoramento contínuo e alertas preventivos contribui para danos materiais, riscos à vida e prejuízos ambientais.

Esse projeto propõe uma solução baseada em Edge Computing e IoT para detectar riscos de alagamento em tempo real, utilizando sensores acessíveis e simulação com Arduino.

💡 Visão Geral da Solução
O projeto utiliza um conjunto de sensores e dispositivos conectados a um Arduino UNO para:

📏 Medir o nível de água com sensor ultrassônico (HC-SR04)

🌡️ Monitorar temperatura e umidade com sensor DHT22

⏱️ Controlar data e hora com o RTC DS1307

💾 Armazenar dados de emergência na EEPROM

📺 Exibir informações em tempo real em um LCD 16x2 I2C

🔔 Acionar LEDs e buzzer conforme o nível de alerta

🚨 Níveis de alerta:
Condição	Ação
Nível da água > 30 cm	LED verde aceso (nível normal)
Nível entre 15 cm e 30 cm	LED amarelo aceso (atenção)
Nível ≤ 15 cm	LED vermelho + buzzer + registro de emergência
Umidade ≥ 85%	LED amarelo aceso (chuva detectada)

📦 O que é armazenado na EEPROM:
Timestamp (Unix time)

Temperatura (°C)

Umidade (%)

Menor distância (nível mais alto da água)

Máximo de 100 registros rotativos (sobrescreve o mais antigo ao atingir o limite)

🧪 Exibição no LCD
Linha 1: número total de emergências registradas
Linha 2: tempo desde a última emergência (em dias e horas) e a data dela

📌 Exemplo:

makefile
Copiar
Editar
Emergencias: 3
Ult: 1d4h (30/05)
▶️ Como Simular no Wokwi
Você pode simular o projeto diretamente clicando no link abaixo:

🔗 Projeto no Wokwi:
https://wokwi.com/projects/432405633921121281

✅ Passos:
Clique no link acima para abrir o projeto no Wokwi

Clique em "Start Simulation"

Acompanhe a leitura de sensores, exibição no LCD e a lógica de alerta em tempo real
