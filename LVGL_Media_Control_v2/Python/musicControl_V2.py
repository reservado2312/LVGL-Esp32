import pyautogui
import serial  # Para comunicação com o ESP32
import time

# Função para interpretar os comandos recebidos e acionar os controles de mídia
def controlar_musica(comando):
    if comando == "Prev":
        print("Voltando para a música anterior")
        pyautogui.press('prevtrack')  # Tecla de mídia para música anterior
    elif comando == "Play":
        print("Tocando música")
        pyautogui.press('playpause')  # Tecla de mídia para play/pause
    elif comando == "Pause":
        print("Pausando música")
        pyautogui.press('playpause')  # Mesma tecla para pausar
    elif comando == "Next":
        print("Pulando para a próxima música")
        pyautogui.press('nexttrack')  # Tecla de mídia para próxima música
    else:
        print(f"Comando desconhecido: {comando}")

# Configurando a porta serial (substitua pelo nome da sua porta)
ser = serial.Serial('COM3', 9600, timeout=1)  # Troque 'COM3' pela sua porta

# Loop principal
while True:
    try:
        # Ler o comando da porta serial
        comando = ser.readline().decode('utf-8').strip()
        
        if comando:
            controlar_musica(comando)
        
        time.sleep(0.1)  # Pequeno intervalo para evitar sobrecarga
    except Exception as e:
        print(f"Erro: {e}")
        break
